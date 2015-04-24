
/*****************************************************************************\
* Laboratory Exercises COMP 3510                                              *
* Author: Saad Biaz                                                           *
* Date  : March 5, 2013                                                       *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/


#include "common.h"


/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/



/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/





/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/




/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/
Event e;
double RT[32], TT[32];

Status tempFlags;
int placeNextEvent = 0, totalEvents = 0, eventServiceCount = 0, deviceEvents[32], deviceEventsServiced[32];
Event capturedEvents[200];



/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void Control(void);
void InterruptRoutineHandlerDevice(void);
void BookKeeping();


/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment for embedded systems. The parent *
*           process is the "control" process while children process will gene-*
*           generate events on devices                                        *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {
  
   if (Initialization(argc,argv)){
     Control();
   } 
} /* end of main function */

/***********************************************************************\
 * Input : none                                                          *
 * Output: None                                                          *
 * Function: Monitor Devices and process events (written by students)    *
 \***********************************************************************/
void Control(void){ 

  while (1) {
  if (totalEvents ) {
    if (eventServiceCount >= 200) {eventServiceCount = 0;}
    Event temp = capturedEvents[eventServiceCount];
    RT[temp.DeviceID] += Now() - temp.When;
    DisplayEvent('X', &temp);
    // Server(&temp);
    deviceEventsServiced[temp.DeviceID] += 1;
    TT[temp.DeviceID] += Now() - temp.When;
    eventServiceCount++;
    totalEvents--;
  }

  } 
}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run whenever an event occurs on a device    *
*           The id of the device is encoded in the variable flag        *
\***********************************************************************/
void InterruptRoutineHandlerDevice(void){
    printf("An event occured at %f  Flags = %d \n", Now(), Flags);
  
  tempFlags = Flags;
  Flags = 0;
  int position = 0;
  double start = Now();
  while (tempFlags) {             //Loops through servicing events from each device in the order of 1-32, ends when no CurrentStatus == 0
    if (tempFlags & 1) { 
      if ( placeNextEvent >= 200) {placeNextEvent = 0;}         //Sets to 0 so we never run out of room
      e = BufferLastEvent[position];      //Holder variable for event
      capturedEvents[placeNextEvent] = e;
      deviceEvents[e.DeviceID] += 1;
      printf("EventID just stored: %i \n", capturedEvents[placeNextEvent].EventID);
      placeNextEvent++;
      totalEvents++;            
    }
    position ++;
    tempFlags >>= 1;
  }
  double end = Now();
  printf("Time elapsed: %f ", end - start);
  // Put Here the most urgent steps that cannot wait
}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This must print out the number of Events buffered not yet   *
*           not yet processed (Server() function not yet called)        *
\***********************************************************************/
void BookKeeping(void){
  // For EACH device, print out the following metrics :
  // 1) the percentage of missed events, 2) the average response time, and 
  // 3) the average turnaround time.
  // Print the overall averages of the three metrics 1-3 above
  
  printf("Bookkeeping activated");
}






