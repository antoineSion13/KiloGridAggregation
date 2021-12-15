#include <stdlib.h>

#include "kilolib.h"
#include "utils.h"
#include <math.h>
#include "kilob_tracking.h"

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

int state = 0;
uint32_t timerGoStraight = 0;
uint32_t timerTurn = 0;
uint32_t goStraightDuration = 32*5 //Going straight for 5 seconds

bool enteringSite = false;

int robotType = NON_INFORMED; //DEFINE HERE THE TYPE OF ROBOT

int groundColor = GREY;

void message_rx(IR_message_t *m, distance_measurement_t *d)
{
	//implement smthing to change the groundColor when receiving a message from the KiloGrid
	if (msg->type == 1)
	{
    groundColor = msg->data[0];
  }
  return;
}

void setup() {
	kilob_tracking_init();
	init_motors();
	move_straight();
	timerGoStraight = kilo_ticks + goStraightDuration;

}

void loop() {
	switch (state) {
		case RANDOM_WALK:
		if(IsInSite() && !enteringSite) //Check if a robot has entered the site where he needs to stay
      {
          move_straight(); //if yes, going straight for 10 seconds
          timerTurn = 0;
          timerGoStraight = timer + goStraightDuration;
          enteringSite = true; //bool to know that we are in the entering phase
      }
      else if (timerGoStraight > kilo_ticks) //while going straight, we check for obstacles
      {
        if(CheckObstacles()) //checking for obstacles
        {
          if(enteringSite && IsInSite()) //if entering a site, just stop and stay there
          {
            Stop();
            state = STAY;
            enteringSite = false;
            timerGoStraight = 0;
            timerLeave = timer + 20;
          }
          else // not entering a site, normal random walk
          {
            Turn(UNIFORM_DISTRIBUTION); //turning from an angle selected uniformly
            timerGoStraight = 0;
          }
        }
        else
        {
          //continue going straight
        }
      }
      else if(timer == timerGoStraight) //if we are at the end of a step of the robot
      {
        if(enteringSite && IsInSite()) //if entering a site, just stop and stay there
        {
          Stop();
          state = STAY;
          enteringSite = false;
          timerGoStraight = 0;
          timerLeave = timer + 20;
        }
        else //if not, normal random walk
        {
          Turn(WRAPPED_CAUCHY_DISTRIBUTION); //turning from an angle selected using the Cauchy wrapped distribution (more chance to continue in the existing general direction)
          timerGoStraight = 0;
        }
      }
      else if(timerTurn == timer) //if we are at the end of turning
      {
        GoStraight(); //go straight for 5 seconds
        timerTurn = 0;
        timerGoStraight = timer + 50;
      }
			break;
		case STAY:

			break;
		case LEAVE:

			break;
	}
}

int main() {
  kilo_init();
	utils_init();

	utils_message_rx = message_rx; // register IR reception callback

  kilo_start(setup, loop);

  return 0;
}

//Check if a robot is in its appropriate site where it must stay
bool IsInSite()
{
  if((robotType == NON_INFORMED && (groundColor == WHITE || groundColor == BLACK))
  || (robotType == INFORMED_BLACK && groundColor == BLACK)
  || (robotType == INFORMED_WHITE && groundColor == WHITE))
  {
    return true;
  }
  else
  {
    return false;
  }
}
