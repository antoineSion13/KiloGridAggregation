#include <stdlib.h>
#include <math.h>

#include "kilolib.h"
#include "utils.h"
#include "kilo_rand_lib.h"
#include "kilob_tracking.h"
#include "kilob_messaging.h"

//states definitions
#define RANDOM_WALK 0
#define STAY 1
#define LEAVE 2

//distributions definition
#define WRAPPED_CAUCHY_DISTRIBUTION 10
#define UNIFORM_DISTRIBUTION 11

//colors definitions
#define BLACK 20
#define WHITE 21
#define GREY 22

//Robot's type definitions
#define INFORMED_BLACK 30
#define INFORMED_WHITE 31
#define NON_INFORMED 32

//PI
#define PI 3.14159265359

int state = 0;
uint32_t timer_go_straight = 0;
uint32_t timer_turn = 0;
uint32_t go_straight_duration = 32*5; //Going straight for 5 seconds
double timer_turn_coefficient = 45; //coefficient use to convert from turning angle to time turning
uint32_t pleave_sampling_duration = 32*2; //Going straight for 5 seconds
int robot_type = NON_INFORMED; //DEFINE HERE THE TYPE OF ROBOT

bool entering_site = false;
int ground_color = GREY;
bool obstacle = false;

IR_message_t* msg; //msg to send

int buffer_size = 25;
uint16_t broadcasting_robots[buffer_size] = {0};

//Check if a robot is in its appropriate site where it must stay
bool is_in_site()
{
  if((robot_type == NON_INFORMED && (ground_color == WHITE || ground_color == BLACK))
  || (robot_type == INFORMED_BLACK && ground_color == BLACK)
  || (robot_type == INFORMED_WHITE && ground_color == WHITE))
  {
    return true;
  }
  else
  {
    return false;
  }
}

//Implementation of a Cauchy wrapped distribution
double cauchy_wrapped()
{
  //parameter gamma calculated from p = 0.5 (https://stats.stackexchange.com/questions/519831/mapping-a-wrapped-cauchy-distribution-to-a-uniform-distribution)
  double gamma = 0.69314718056;
  double random_uniform_variate = g_ran_uniform();
  double without_modulo = gamma * tan(PI * random_uniform_variate - PI/2);
  while(without_modulo > PI)
  {
    without_modulo -= 2*PI;
  }
  while(without_modulo < -PI)
  {
    without_modulo += 2*PI;
  }
  double theta = without_modulo;
  return theta;
}

//Turn following a random distribution
void turn(int random_distribution)
{
  //Calculate the angle to use here
  double random_angle = 0.0;
  if(random_distribution == WRAPPED_CAUCHY_DISTRIBUTION)
  {
    random_angle = cauchy_wrapped();
		random_angle = random_angle * 180 / PI;
  }
  else if(random_distribution == UNIFORM_DISTRIBUTION)
  {
    random_angle = -180.0 + g_ran_uniform() * 360.0;
  }
  double time = random_angle / timer_turn_coefficient; //divide by timer_turn_coefficient to get time in seconds
  if(time < 0)
  {
    turn_left();
    timer_turn = kilo_ticks + int(-time * 32); //timer_turn in kiloticks
  }
  else if(time > 0)
  {
		turn_right();
    timer_turn = kilo_ticks + int(time * 32); //timer_turn in kiloticks
  }
  else if(time == 0)
  {
    timer_turn = kilo_ticks; //this prevent the timer_turn to be set to 0 at the same time than timer_go_straight
  }
}

//Sample the probability to leave and return true if you can leave
bool leave_site()
{
  //compute number of broadcasting robots on the site
  int n = 0;
  for(int i = 0; i < buffer_size; ++i)
  {
    if(broadcasting_robots[i] != 0)
    {
      n++;
    }
  }
  double pleave = 0.5*exp(-2.25*n);
  if(robot_type == INFORMED_WHITE || robot_type == INFORMED_BLACK)
  {
   pleave = 0;
  }
  //clear the buffer
  for(int i = 0; i < buffer_size; ++i)
  {
    broadcasting_robots[i] = 0;
  }
  //test the proba
  double random_number = g_ran_uniform();
  if(random_number <= pleave)
   return true;
  else
   return false;
}

void broadcast()
{
  if((msg = kilob_message_send()) != NULL)
  {
    msg->type = 2;
    msg->data[0] = kilo_uid;
  }
}

void message_rx(IR_message_t *m, distance_measurement_t *d)
{
	if (msg->type == 1) //type 1 for message received from the Modules
	{
    ground_color = msg->data[0];
		obstacle = msg->data[1];
  }
  if (msg->type == 2) //type 2 for message received from the Kilobots
	{
    uint16_t id = msg->data[0];
    for(int i = 0; i < buffer_size; ++i)
    {
      if(broadcasting_robots[i] == id)
      {
        return;
      }
    }
    for(int i = 0; i < buffer_size; ++i)
    {
      if(broadcasting_robots[i] == 0)
      {
        broadcasting_robots[i] = id;
        return;
      }
    }
  }
  return;
}

void setup() {
	kilob_tracking_init(); //no need for kilob_messaging_init, it calls it with this
	init_motors();
	move_straight();
	timer_go_straight = kilo_ticks + go_straight_duration;
}

void loop() {
	switch (state) {
		case RANDOM_WALK:
		if(is_in_site() && !entering_site) //Check if a robot has entered the site where he needs to stay
      {
          move_straight(); //if yes, going straight for a little bit
          timer_turn = 0;
          timer_go_straight = kilo_ticks + go_straight_duration/2;
          entering_site = true; //bool to know that we are in the entering phase
      }
      else if (timer_go_straight > kilo_ticks) //while going straight, we check for obstacles
      {
        if(obstacle) //checking for obstacles (we only consider walls here, not the robot themselves as in argos)
        {
					//TO TEST BUT WE MAYBE NEED A MORE EFFICIENT METHOD TO AVOID WALL, FOR NOW IT JUST TURNS UNTIL NO WALL IS DETECTED ANYMORE BUT THIS COULD BE SLOW
					// MAYBE IMPLEMENT SMTHING TO MAKE A 1/2 TURN ON ITSELF THEN STRAIGHT WITHOUT REREADING THAT THERE IS AN OBSTACLE
            turn(UNIFORM_DISTRIBUTION); //turning from an angle selected uniformly
            timer_go_straight = 0;
        }
      }
      else if(timer_go_straight <= kilo_ticks && timer_go_straight != 0) //if we are at the end of a step of the robot
      {
        if(is_in_site() && entering_site) //if entering a site, just stop and stay there
        {
          stop();
          state = STAY;
          entering_site = false;
          timer_go_straight = 0;
          timerLeave = kilo_ticks + pleave_sampling_duration;
        }
        else //if not, normal random walk
        {
          turn(WRAPPED_CAUCHY_DISTRIBUTION); //turning from an angle selected using the Cauchy wrapped distribution (more chance to continue in the existing general direction)
          timer_go_straight = 0;
        }
      }
      else if(timer_turn <= kilo_ticks && timer_turn != 0) //if we are at the end of turning
      {
        move_straight(); //go straight for 5 seconds
        timer_turn = 0;
        timer_go_straight = kilo_ticks + go_straight_duration;
      }
			break;
		case STAY:
      broadcast();
      if(kilo_ticks >= timerLeave) //if it is time to sample the probability to leave
      {
        //sample probability to leave
        if(leave_site())
        {
          state = LEAVE; //going into state LEAVE
          timerLeave = 0;
          move_straight();
          timer_turn = 0;
        }
        else
        {
          timerLeave = kilo_ticks + pleave_sampling_duration;
        }
      }
			break;
		case LEAVE:
      if(!is_in_site()) //checking if we are out of the site
      {
        move_straight(); //transition to RANDOM_WALK if we are out
        timer_go_straight = timer + 50;
        timer_turn = 0;
        state = RANDOM_WALK;
      }
			break;
	}
  //if needed, add stuff with tracking data here
}

int main() {
  kilo_init();
	utils_init();

	utils_message_rx = message_rx; // register IR reception callback

  kilo_start(setup, loop);

  return 0;
}
