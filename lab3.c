
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



#define  MAX_QUEUE_SIZE 32


/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/




/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/
Event e;
double RespTime[32], TurnaroundTime[32];
double RespTimeHigh[32], TurnaroundTimeHigh[32];
Status tempFlags;
int capturedLength = 0, capturedIndex = 0, totalEvents = 0, eventServiceCount = 0, deviceEvents[32], deviceEventsServiced[32];

// array of events captured and waiting for processing
Event capturedEvents[MAX_QUEUE_SIZE];


int processedEvents[100];
int processedLength;

int in_use = 0;

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
  int f;
  for(f = 0; f < 100; f++){
    processedEvents[f] = -1;
  }
  int processedIndex = 0;
  int k;
  //busy waiting loop
  while (1) {

    // if we have events to process
    int i;
    if(totalEvents){
      for(i = 0; i < capturedLength; i++){
          Event temp;
          memcpy(&temp, &capturedEvents[i], sizeof(Event));
          if(temp.priority == 0){
            RespTimeHigh[temp.DeviceID] = Now() - temp.When;
            int value = (temp.DeviceID * 100) + temp.EventID;
            //see if the the event has already been processed
            int processed = 0;
            for(k = 0; k < processedLength; k++){
              if(processedEvents[k] == value){
                processed = 1;
              }
            }


            if(!processed){
              deviceEvents[temp.DeviceID]++; //increment events for device
              deviceEventsServiced[temp.DeviceID] += 1;
              memcpy(&processedEvents[processedIndex], &value, sizeof(int));
              if(processedLength < 100){
                processedLength++;
              }
              if(processedIndex < 99){
                processedIndex++;
              } else {
                processedIndex = 0;
              }
              totalEvents--;
              Server(&temp);
          
            }
            double time =  Now() - temp.When;
            TurnaroundTimeHigh[temp.DeviceID] = time;
            // printf("\n at index %d: device ID: %d, event ID: %d, checked to see if already processed at %d\n\n\n",
              // i,
              // temp.DeviceID,
              // temp.EventID,
              // value);
          }
      }
      for(i = 0; i < capturedLength; i++){
          Event temp;
          memcpy(&temp, &capturedEvents[i], sizeof(Event));
          if(temp.priority > 0){
            RespTime[temp.DeviceID] = Now() - temp.When;
            int value = (temp.DeviceID * 100) + temp.EventID;
            //see if the the event has already been processed
            int processed = 0;
            for(k = 0; k < processedLength; k++){
              if(processedEvents[k] == value){
                processed = 1;
              }
            }


            if(!processed){
              deviceEvents[temp.DeviceID]++; //increment events for device
              deviceEventsServiced[temp.DeviceID] += 1;
              memcpy(&processedEvents[processedIndex], &value, sizeof(int));
              if(processedLength < 100){
                processedLength++;
              }
              if(processedIndex < 99){
                processedIndex++;
              } else {
                processedIndex = 0;
              }
              totalEvents--;
              Server(&temp);
          
            }
            double time =  Now() - temp.When;
            TurnaroundTime[temp.DeviceID] = time;
            // printf("\n at index %d: device ID: %d, event ID: %d, checked to see if already processed at %d\n\n\n",
              // i,
              // temp.DeviceID,
              // temp.EventID,
              // value);
          }
      }

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
  int position = 0;
  double start = Now();
  tempFlags = Flags; // store this so we can get through 

  
  // within the length of one interrupt handle raised flags
  while (tempFlags > 0) {
    if ((tempFlags >= 1) & 1) {  // roll off captured flag
      memcpy(&e, &BufferLastEvent[position], sizeof(Event));      // grab event from buffer
      // DisplayEvent('A', &e);
      memcpy(&capturedEvents[capturedIndex], &e, sizeof(Event)); // deep copy to captured events
      if(capturedIndex >= MAX_QUEUE_SIZE){capturedIndex = 0;}else{capturedIndex++;}
      if(capturedLength >= MAX_QUEUE_SIZE){capturedLength = MAX_QUEUE_SIZE;}else{capturedLength++;}
      totalEvents++;            
      int ithBitHandled = (1 >> (position));
      Flags &= ~ithBitHandled;
    }
    tempFlags >>= 1;
    position++;
  }
  double end = Now();
  // printf("Time elapsed: %f ", end - start);
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
  int i;
  int numDevices = 0;
  int missedEventsTotal = 0;
  double responseTimeTotal = 0;
  double responseTimeTotalHigh = 0;
  double turnaroundTimeTotal = 0;
  double turnaroundTimeTotalHigh = 0;
  for(i = 0; i < MAX_NUMBER_DEVICES; i++){
    
    if(deviceEvents[i] != 0){
      numDevices+= 1;
      missedEventsTotal += (deviceEvents[i] - deviceEventsServiced[i]);
      responseTimeTotal += RespTime[i];
      responseTimeTotalHigh += RespTimeHigh[i];
      turnaroundTimeTotal += TurnaroundTime[i];
      turnaroundTimeTotalHigh += TurnaroundTimeHigh[i];

    }
  }
  printf("\n %d %2f %2f %2f %2f \n", (missedEventsTotal/numDevices), (responseTimeTotalHigh/numDevices), (turnaroundTimeTotalHigh/numDevices), (responseTimeTotal/numDevices), (turnaroundTimeTotal/numDevices));
  
}






