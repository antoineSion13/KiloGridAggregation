#include "kilolib.h"
#include "agent.h"
#include <stdlib.h>
#include <math.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <float.h>
#include <debug.h>

// #include "utils.h"
// #include "kilo_rand_lib.h"
// #include "kilob_tracking.h"
// #include "kilob_messaging.h"

//states definitions
#define RANDOM_WALK 0
#define STAY 1
#define LEAVE 2
#define OBSTACLE_AVOIDANCE 3

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
#define RAND_MAX_VALUE 255

typedef enum{
    false = 0,
    true = 1,
} bool;

int state = 0;
uint32_t timer_go_straight = 0;
uint32_t timer_turn = 0;
uint32_t timer_leave = 0;
uint32_t go_straight_duration = 32*10; //Going straight for 10 seconds
double timer_turn_coefficient = 45; //coefficient use to convert from turning angle to time turning
uint32_t pleave_sampling_duration = 32*2; //2 seconds sampling
int robot_type = NON_INFORMED; //DEFINE HERE THE TYPE OF ROBOT

bool broadcast_bool = false;
bool entering_site = false;
int ground_color = GREY;
bool obstacle = false;

message_t msg; //msg to send

int buffer_size = 25;
uint16_t broadcasting_robots[25] = {0};

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

double g_ran_uniform() {
  // debug_info_set(g_ran_debug, ((double) rand_hard() / (double) RAND_MAX_VALUE));
  return ((double) rand_hard() / (double) RAND_MAX_VALUE);
}

//Implementation of a Cauchy wrapped distribution
double cauchy_wrapped()
{
  //parameter gamma calculated from p = 0.5 (https://stats.stackexchange.com/questions/519831/mapping-a-wrapped-cauchy-distribution-to-a-uniform-distribution)
  double gamma = 0.69314718056;
  double random_uniform_variate = g_ran_uniform();
  double without_modulo = gamma * tan(PI * random_uniform_variate - PI/2);
  double theta = fmod(without_modulo,2*PI);
  if(theta > PI)
    theta -= 2*PI;
  else if(theta < -PI)
    theta += 2*PI;
  debug_info_set(theta_debug, theta);
  return theta;
}

void turn_left()
{
  spinup_motors();
  set_motors(kilo_turn_left, 0);
}

void turn_right()
{
  spinup_motors();
  set_motors(0, kilo_turn_right);
}

void move_straight()
{
  spinup_motors();
  set_motors(kilo_turn_left, kilo_turn_right);
}

void stop()
{
  set_motors(0, 0);
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
    double double_time = -time * 32;
    uint32_t int_time = double_time;
    timer_turn = kilo_ticks + int_time; //timer_turn in kiloticks
  }
  else if(time > 0)
  {
		turn_right();
    double double_time = time * 32;
    uint32_t int_time = double_time;
    timer_turn = kilo_ticks + int_time; //timer_turn in kiloticks
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
  {
    return true;
  }
  else
  {
    return false;
  }
}

void broadcast()
{
  broadcast_bool = true;
}

void stop_broadcast()
{
  broadcast_bool = false;
}

message_t *message_tx()
{
  if(broadcast_bool == true)
  {
    set_color(RGB(1, 1, 1));
    return &msg;
  }
  else
  {
    set_color(RGB(0, 0, 0));
    return NULL;
  }
}

void message_rx(message_t *message, distance_measurement_t *distance)
{
	if (message->type == 1) //type 1 for message received from the Modules
	{
    ground_color = message->data[0];
		obstacle = message->data[1];
  }
  if (message->type == 2) //type 2 for message received from the Kilobots
	{
    uint16_t id = message->data[0];
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
	// kilob_tracking_init(); //no need for kilob_messaging_init, it calls it with this
	// init_motors();
  msg.type = 2;
  msg.data[0] = kilo_uid;
  msg.crc = message_crc(&msg);
  stop_broadcast();

	move_straight();
	timer_go_straight = kilo_ticks + go_straight_duration;
}

void loop() {
  // debug_info_set(kilo_ticks_debug, kilo_ticks);
  // debug_info_set(timer_go_straight_debug, timer_go_straight);
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
            state = OBSTACLE_AVOIDANCE;
            stop();
            timer_go_straight = 0;
            timer_turn = 0;
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
          timer_leave = kilo_ticks + pleave_sampling_duration;
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
      if(kilo_ticks >= timer_leave) //if it is time to sample the probability to leave
      {
        //sample probability to leave
        if(leave_site())
        {
          state = LEAVE; //going into state LEAVE
          timer_leave = 0;
          move_straight();
          timer_turn = 0;
        }
        else
        {
          timer_leave = kilo_ticks + pleave_sampling_duration;
        }
      }
			break;
		case LEAVE:
      stop_broadcast();
      if(!is_in_site()) //checking if we are out of the site
      {
        move_straight(); //transition to RANDOM_WALK if we are out
        timer_go_straight = kilo_ticks + go_straight_duration;
        timer_turn = 0;
        state = RANDOM_WALK;
      }
			break;
    case OBSTACLE_AVOIDANCE:
      if(timer_turn == 0 && timer_go_straight == 0)
      {
        turn_left();
        double time = 180 / timer_turn_coefficient;
        double double_time = time * 32;
        uint32_t int_time = double_time;
        timer_turn = kilo_ticks + int_time;
      }
      else if(timer_turn <= kilo_ticks && timer_go_straight == 0)
      {
        timer_turn = 0;
        move_straight();
        timer_go_straight = kilo_ticks + go_straight_duration;
      }
      else if(timer_go_straight <= kilo_ticks && timer_turn == 0)
      {
        timer_go_straight = kilo_ticks + go_straight_duration;
        state = RANDOM_WALK;
      }
      break;
	}
  //if needed, add stuff with tracking data here
}

int main() {
  kilo_init();
	// utils_init();
  kilo_message_tx = message_tx;
	kilo_message_rx = message_rx; // register IR reception callback

  debug_info_create();

  kilo_start(setup, loop);

  return 0;
}
