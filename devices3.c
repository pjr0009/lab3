/*****************************************************************************\
 * Laboratory Exercises COMP 3510                                              *
 * Author: Saad Biaz                                                           *
 * Date  : February 16, 2010                                                   *
 *           changed January 17, 2012 (masked call to interrupt)               *
 *           added Grading Checking to help TA grade check students numbers    * 
 * File  : devices2.c  for Lab2                                                *
 \*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/

#include "common.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/

typedef double          TimePeriod;




/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/

#define  MAXEVENTID        100     /* Max number of events per device        */
#define  KEYGRADING        6861    /* If Show = Key then display metrics     */
/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/



/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/

Status         Flags;
Event          BufferLastEvent[MAX_NUMBER_DEVICES];


Status         Masks[MAX_NUMBER_DEVICES];
Timestamp      StartTime;       /* Start time of emulation                */
struct timeval SelectTimeout;   /* Used for select function               */
int            ParentID;        /* Process ID of Parent (Control)         */
int            fdPipes[MAX_NUMBER_DEVICES][2];
int            ChildrenPIDs[MAX_NUMBER_DEVICES];
Identifier     NumberProcessedEvents[MAX_NUMBER_DEVICES];
Quantity       NumberGeneratedEvents[MAX_NUMBER_DEVICES];
FILE          *fpDevice;        /* File per Device records generatd events */
FILE          *fp[MAX_NUMBER_DEVICES]; /* Files to record processed events     */
int            maxfd1;          /* For the select function                 */
fd_set         rset;            /* set of file descriptors used for select */
Identifier     NextEventID;     /* To identify events within devices       */
Quantity       Number_Devices;  /* Number of devices                       */
Average        lambda,          /* Mean interarrival time in 0.01 sec      */ 
               mu;              /* Mean service time as a percentage     
         * of lambda/number_devices                */
Flag           Show;
short          ClosedDevices;

/* SB_ 2/6/2012 Add Response time/Turnaround time for Grading */
Average        SumResponseTimes[MAX_NUMBER_DEVICES];
Average        SumTurnaroundTimes[MAX_NUMBER_DEVICES];




/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/
void        Server(Event *whichEvent);
static void DevicesHandler(int signo);
static void DoneDeviceHandler(int signo);
static void DoneHandler(int signo);
void        Device(Identifier DeviceID);
Flag        Initialization(int argc, char **argv);
Timestamp   Now();
void        BlockingWait(TimePeriod HowMuch);
TimePeriod  GetRandomNumberExponential(Average lambda);
TimePeriod  GetRandomNumberUniform(Average a, Average b);
void        DisplayEvent(char c,Event *whichEvent);

/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function: serves event with taking some expo. dist. time with mean mu *
\***********************************************************************/
void        Server(Event *whichEvent){
  TimePeriod ServiceTime;
  Event      thisEvent;
  Identifier thisDeviceID;
  Identifier thisEventID;
  int        i;

  Average    AvgResponseTime,
    AvgTurnaroundTime; /* SB_ 2/6/2012 add Resp... */ 
  Average        OverallAvgResponseTimes, OverallAvgTurnaroundTimes, AvgMissedEvents;
  Quantity       NbreSamples;/* SB_ 2/6/2012 add Resp... */

  memcpy(&thisEvent,whichEvent,sizeof(thisEvent));

  /* SB_ 2/6/2012 Add Response time/Turnaround time for Grading */
  // SumResponseTimes[thisEvent.DeviceID] += Now() - thisEvent.When;
  // ResponseTime must be moved to Interrupt Handler
  /* SB_ 2/6/2012 End */



  thisEventID   = thisEvent.EventID;
  thisDeviceID = thisEvent.DeviceID;
  
  ServiceTime = GetRandomNumberExponential(mu);
  BlockingWait(ServiceTime);
  

 /* SB_ 2/6/2012 Add Response time/Turnaround time for Grading */
  SumTurnaroundTimes[thisDeviceID] += Now() - thisEvent.When;

  /* SB_ 2/6/2012 End */

  if (Show==1)
    fprintf(fp[thisDeviceID],"%5d %10.3f %2d\n",
      thisEventID, thisEvent.When, thisEvent.priority);
  NumberProcessedEvents[thisDeviceID]++;

  if (NumberGeneratedEvents[thisDeviceID] >= MAXEVENTID){
      ClosedDevices++;
      if (ClosedDevices == Number_Devices){
  OverallAvgResponseTimes    = 0.0;
  OverallAvgTurnaroundTimes  = 0.0;
  AvgMissedEvents            = 0.0;
  NbreSamples                = 0;

  if (Show == KEYGRADING) {
    printf("\n\n********* GRADING Checking *********************************\n");
  }

  for (i=0; i < Number_Devices; i++){
    /* SB_ 2/6/2012 Add Response time/Turnaround time for Grading */
    AvgResponseTime   = -1;
    AvgTurnaroundTime = -1;
    if (NumberProcessedEvents[i] > 0){
      AvgResponseTime          = SumResponseTimes[i]   / NumberProcessedEvents[i];
      OverallAvgResponseTimes += AvgResponseTime;
      AvgTurnaroundTime = SumTurnaroundTimes[i] / NumberProcessedEvents[i];
      OverallAvgTurnaroundTimes += AvgTurnaroundTime;
      NbreSamples++;
    }
    AvgMissedEvents += NumberGeneratedEvents[i]-NumberProcessedEvents[i];
     /* SB_ 2/6/2012 End */
    if (Show == KEYGRADING) {
      printf("Device %2d generated about %d events and you missed %3d. Resp. is %6.4f, Turnaround is %6.4f\n",
       i,NumberGeneratedEvents[i],NumberGeneratedEvents[i]-NumberProcessedEvents[i],
       AvgResponseTime,AvgTurnaroundTime);
    }
    /* SB_ 2/6/2012 End */
    fclose(fp[i]);
    kill (ChildrenPIDs[i],SIGINT);
  }
  OverallAvgResponseTimes   = OverallAvgResponseTimes / NbreSamples;
  OverallAvgTurnaroundTimes = OverallAvgTurnaroundTimes / NbreSamples;
  if (Show == KEYGRADING){
    printf("  >>>> Overall Averages:  Missed Events (%6.4f), Response (%6.4f), Turnaround (%6.4f)\n",
     AvgMissedEvents/Number_Devices, OverallAvgResponseTimes, OverallAvgTurnaroundTimes);
    printf("********* End GRADING Checking ****************************\n\n\n");
  }
  BookKeeping(); /* Call Function to be written by students */
  exit(0);
      }
  }
}

/***********************************************************************\
* Input : signal number signo (we do not use signo)                     *
* Output: None                                                          *
* Function: Handle events from the devices. The device  reads the event *
*         from the right pipe, stores the event in the device buffer,   *
*         and for lab 3 will raise an interrupt for students' process   *  
\***********************************************************************/
static void DevicesHandler(int signo){
  int i,nbcar;
  Event whichEvent;
  signal(SIGUSR1, DevicesHandler);
  for (i = 0; i < Number_Devices; i++){
    FD_SET(fdPipes[i][0],&rset);
    
  }
  select(maxfd1,&rset, NULL, NULL,&SelectTimeout);
   for (i = 0; i < Number_Devices;i++){
     if (FD_ISSET(fdPipes[i][0],&rset)){
      nbcar = read(fdPipes[i][0],&whichEvent,sizeof(whichEvent));
      memcpy(&BufferLastEvent[i],&whichEvent,sizeof(whichEvent));

      NumberGeneratedEvents[i]++;
      Flags = Flags | Masks[i];
      /* SB_ 2/12 for Lab2 Response Time now starts with the interrupt */
      SumResponseTimes[i] += Now() - BufferLastEvent[i].When; 
      InterruptRoutineHandlerDevice();
    }
  }
}

/***********************************************************************\
 * Input : signal number signo (we do not use signo)                     *
 * Output: None                                                          *
 * Function: Device closes record file "Devicei.gen"                     *
 \***********************************************************************/
static void DoneDeviceHandler(int signo){

  fclose(fpDevice);
  exit(0);
}



/***********************************************************************\
* Input : signal number signo (we do not use signo)                     *
* Output: None                                                          *
* Function: Handle CTLR-C from keyboard and prints out the number of    *
*         of missed events by each device, and kill all device process. *
\***********************************************************************/
static void DoneHandler(int signo){
  int i;
  
  for (i = 0; i < Number_Devices; i++){
    printf("Device %2d generated about %d events and you missed %5d (BF)\n",i,
     NumberGeneratedEvents[i],NumberGeneratedEvents[i] - NumberProcessedEvents[i]);
    fclose(fp[i]);
  }
  BookKeeping(); /* Call Function to be written by students */
  exit(0);
}



/***********************************************************************\
* Input : Device ID from 0 to Number_Devices-1                         *
* Output: None                                                         *
* Function: Simulates a Device by generating events and an interrupt   *
*           the the Parent to store the event in a buffer zone         *
\***********************************************************************/
void Device(Identifier DeviceID){
  char buffer[128];
  struct timeval timeout;
  fd_set rset;
  Timestamp NextEventTimestamp, CurrentTime;
  TimePeriod Time2NextEvent;
  double dummy;
  Event whichEvent;
  char   filename[16];

  signal(SIGINT, DoneDeviceHandler);
  sprintf(filename,"Device_%d.gen",DeviceID);
  fpDevice = fopen(filename,"w");

  srand48(107*DeviceID);

  FD_ZERO(&rset);

   while (NextEventID < MAXEVENTID) {
     CurrentTime = Now();
     Time2NextEvent = GetRandomNumberExponential(lambda);
     NextEventTimestamp = CurrentTime + Time2NextEvent;
     /* Convert Time2NextEvent to a timeval timout */
     timeout.tv_usec = (long) floor(modf(Time2NextEvent,&dummy) * 1000000.0);
     timeout.tv_sec  = (long) floor(dummy);
     
     select(1, &rset, NULL, NULL, &timeout); 

     whichEvent.DeviceID = DeviceID;
     whichEvent.EventID  = NextEventID++;
     whichEvent.When     = NextEventTimestamp;
     whichEvent.priority = DeviceID; //(Priority) floor(drand48() *  MAX_PRIORITIES);
     sprintf(whichEvent.msg,"Event %d from Device %d at %f with priority %d\n",
       whichEvent.EventID,
       whichEvent.DeviceID,
       whichEvent.When,
       whichEvent.priority);
     if (Show == 1){
       DisplayEvent('G',&whichEvent);
       fprintf(fpDevice,"%5d %10.3f %2d\n",
         whichEvent.EventID, whichEvent.When, whichEvent.priority);
     }
     write(fdPipes[DeviceID][1],(char *) &whichEvent,sizeof(whichEvent));
     kill(ParentID,SIGUSR1);     
   } /* while (NextEventID < 100) */
   if (Show == 1)
     printf("Device %d is done\n",DeviceID);
   while(1);
}

/***********************************************************************\
* Input    : Standard command line parameters                           *
* Output   : Returns a flag TRUE if everything went ok                  *
* Function : Initialize global variables, structures. Create processes  *
*             to simulate devices                                       *
\***********************************************************************/

Flag Initialization(int argc, char **argv){
  int i,j;
  char filename[64];

  if (argc != 5) {
    printf("usage: command NumberOfDevices lambda  mu Show\n");
    printf("lambda: mean interarival time on each device\n");
    printf("mu    : percentage of lambda/number_devices\n");
    printf(" Show : Flag to dispaly/frint in file\n");
    return(FALSE);
  }

  FD_ZERO(&rset);

  StartTime             = Now();
  SelectTimeout.tv_sec  = 0;
  SelectTimeout.tv_usec = 0;
  ParentID              = getpid();
  NextEventID           = 0;
  maxfd1                = 0;
  Flags                 = 0; 
  ClosedDevices         = 0;
  Number_Devices        =  atoi(argv[1]);
  if ((Number_Devices < 1) || (Number_Devices > MAX_NUMBER_DEVICES)) {
    printf("The number of devices must be between 1 and  %d\n",MAX_NUMBER_DEVICES);
    return(FALSE);
  }

  mu                    = (Average) atoi(argv[3]);
  if (mu < 1.0) {
    printf("mu should be higher than 1\n");
    exit(0);
  }

  lambda                = (Average) atoi(argv[2]);
  /* mu is a percentage of lambda/Number_Devices                    */
  mu                    = lambda * mu/100.0/(float) Number_Devices;
printf("Mu =%f \n",mu); // SB_ 2/7/2012
  Show                  = (Flag)    atoi(argv[4]);

  if (Show == 1)
    printf("NumberDevices = %d   lambda = %f  mu = %f   Show = %d\n",Number_Devices,lambda, mu, Show);
  if (lambda <= 0) {
    printf("lambda should be strictly positive\n");
    exit(0);
  }
 

  if ((Number_Devices *mu) > lambda) {
    printf("warning: server too slow to handle events\n");
    sleep(3);
  }

  Masks[0] = 1;
  for (i = 0; i < Number_Devices;i++){
    Masks[i]                 = Masks[0] << i;
    NumberProcessedEvents[i] = 0;
    NumberGeneratedEvents[i] = 0;
     
    if (pipe(fdPipes[i]) != 0){
      printf("Cannot create a pipe\n");
      return(FALSE);
    }
    ChildrenPIDs[i] = fork();
    if (ChildrenPIDs[i] < 0){
      printf("Cannot fork a child\n");
      return(FALSE);
    }
    if (ChildrenPIDs[i] > 0) {
      /* Parent Code */
      close(fdPipes[i][1]);
      if (fdPipes[i][0] > maxfd1)
  maxfd1 = fdPipes[i][0];
    } else {
      /* Device (Child) Code */
      close(fdPipes[i][0]);
      for (j=0; j < i;j++){
  close(fdPipes[j][0]);
  close(fdPipes[j][1]);
      }
      Device(i);
      exit(0);
    } /* else if (ChildrenPIDs[i] > 0) */

    /* SB_ 2/6/2012 Add Response time/Turnaround time for Grading */
    SumResponseTimes[i]   = 0.0;
    SumTurnaroundTimes[i] = 0.0;
  } /* for (i = 0; i < Number_Devices;i++) */
  
  signal(SIGUSR1,DevicesHandler); 
  signal(SIGINT, DoneHandler);
  for (i = 0; i < Number_Devices;i++){
    sprintf(filename,"Device_%d.proc",i);
    fp[i] = fopen(filename,"w");
  }
  maxfd1++;
  return(TRUE);
}

/**********************************************************************\
* Input    : A time period to sleep for                                 *
* Output   : None                                                       *
* Function : This function will block the process for HowMuch period    *
\***********************************************************************/
void        BlockingWait(TimePeriod HowMuch){
  fd_set     rset;
  double     dummy;
  struct     timeval timeout;
  Timestamp  TimeDone = Now() + HowMuch;
  FD_ZERO(&rset);

  while ((Now() < TimeDone) && (HowMuch > 0)){
    timeout.tv_usec = (long) floor(modf(HowMuch,&dummy) * 1000000.0);
    timeout.tv_sec  = (long) floor(dummy);
    if (!select(1, &rset, NULL, NULL, &timeout))
      return;
    HowMuch = TimeDone - Now();
  }
  return;
}

/***********************************************************************\
* Input    : None                                                       *
* Output   : Returns the current system time                            *
\***********************************************************************/
Timestamp Now(){
  struct timeval tv_CurrentTime;
  gettimeofday(&tv_CurrentTime,NULL);
  return( (Timestamp) tv_CurrentTime.tv_sec + (Timestamp) tv_CurrentTime.tv_usec / 1000000.0-StartTime);
}

/***********************************************************************\
* Input : lambda (Mean)                                                 *
* Output: A random number following an exponential distribution with    *
*         an average of lambda                                          *
\***********************************************************************/
TimePeriod GetRandomNumberExponential(Average lambda) {
    return(-lambda * log(drand48())); 
}

/***********************************************************************\
* Inputs : a (lower bound), b (upper bound)                              *
* Output: A random number following a uniform  distribution with        *
*         parameters a and b                                            *
\***********************************************************************/
TimePeriod GetRandomNumberUniform(Average a, Average b) {
  return(a + (b - a) * drand48());
}

long       DecimalYoBinary(long number){
}

/***********************************************************************\
* Inputs   : a character c and Pointer WhichEvent to an Event           *
* Output   : None                                                       *
* Function : Displays an event preceded by the character input c        *
\***********************************************************************/
void DisplayEvent(char c, Event *whichEvent){ 
  printf("%c %10.3f : DID(%2d)  EID(%3d)  When(%10.3f)    Prio(%2d)  %s\n", 
   c,
   Now(),
   whichEvent->DeviceID,
         whichEvent->EventID,
   whichEvent->When, 
   whichEvent->priority,
   whichEvent->msg);
}















