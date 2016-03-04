#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/inotify.h>

#include <persistence.h>

/*
Flow Sensing
This will be measured in frequency. A high frequency means that the flow is fast. Slow means flow is low.
A0(Analog) - output flow sensor.
A1(Analog) - recirculation flow sensor. The recirculation flow sensor can be used to measure how much water is dumped during dump mode.

Salinity Sensing
A high value for this means that the water is very clean. Low values indicate salty water.
A2(Analog) - intake salinity.
A3(Analog) - output salinity.
A4(Analog) - recirculation salinity.
*/

#define IN_EVENT_SIZE  ( sizeof (struct inotify_event) )
#define IN_BUF_LEN     ( 1024 * ( IN_EVENT_SIZE + 16 ) )

// Files for Sensor Pins
#define FLOW_OUT_FILE 	"/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw"
#define FLOW_DUMP_FILE 	"/sys/devices/12d10000.adc/iio:device0/in_voltage1_raw"
#define SAL_IN_FILE 		"/sys/devices/12d10000.adc/iio:device0/in_voltage2_raw"
#define SAL_OUT_FILE 		"/sys/devices/12d10000.adc/iio:device0/in_voltage3_raw"
#define SAL_RECIRC_FILE "/sys/devices/12d10000.adc/iio:device0/in_voltage4_raw"

// Constants
const long int salResistorOhms = 1000000L;
const float salSensorSpacingCM = 0.3;
const float salSensorAreaCMSq  = 1.0;

// Timing
struct timespec tim;
struct timespec timrem;

#define PERSISTENCE_TIMER 6000
// every 60 seconds, log values


// tracking sensor values
unsigned int lastFlowOut, lastFlowDump, thisFlowOut, thisFlowDump;
long flowCountOut, flowCountDump;
struct timeval lastTime, thisTime;

pthread_t flowOutThread, flowDumpThread;
pthread_mutex_t flowOutMutex, flowDumpMutex;




void error(const char *msg){
  perror(msg);
  exit(1);
}


static int analogRead(const char* fName) {
  FILE * fd;
  char val[8];

  // open value file
  if((fd = fopen(fName, "r")) == NULL) {
     printf("Error: can't open analog voltage value\n");
     return 0;
  }
  fgets(val, 8, fd);
  fclose(fd);
  return atoi(val);
}


static int ppmFromVoltage(int millivolts){
  float ohms = (millivolts / (5000.0 - millivolts)) * salResistorOhms;

	// TODO: Need to test in order to establish constants for the function of resistance to tds

	return ohms / 100;
}


int reportSensorValues(){
	gettimeofday( &thisTime, NULL );
	float elapsed = (float)(((long long int)thisTime.tv_sec * 1000000L + thisTime.tv_usec) - ((long long int)lastTime.tv_sec * 1000000L + lastTime.tv_usec))/1000000.0;

  pthread_mutex_lock(&flowOutMutex);
	float outFrequency = flowCountOut / elapsed;
  flowCountOut = 0;
  pthread_mutex_unlock(&flowOutMutex);

  pthread_mutex_lock(&flowDumpMutex);
	float dumpFrequency = flowCountDump / elapsed;
  flowCountDump = 0;
  pthread_mutex_unlock(&flowDumpMutex);

	struct SenseSet sense;

	sense.timestamp 	= time(NULL);
	sense.qOut 			= outFrequency / 5.5;
	sense.qDump 		= dumpFrequency / 5.5;
	sense.ppmOut 		= ppmFromVoltage(analogRead(SAL_OUT_FILE));
	sense.ppmIn 		= ppmFromVoltage(analogRead(SAL_IN_FILE));
	sense.ppmRec    = ppmFromVoltage(analogRead(SAL_RECIRC_FILE));

	ssize_t rc = persistSenseSet(&sense);
  if( rc == EPIPE ){
    persistenceCleanup();
    error("ERROR Broken Pipe");
  } else if( rc < 0 ){
    persistenceCleanup();
    error("ERROR writing to socket");
  }

	lastTime = thisTime;

	return 0;
}












void * monitorFlowOut( void * ptr ){
  int inotify_fd = inotify_init();
  if( inotify_fd < 0 ){
    error("Inotify could not initialize");
  }

  int inotify_wd = inotify_add_watch( inotify_fd, FLOW_OUT_FILE, IN_MODIFY);
  if( inotify_wd == -1 ){
    error("Inotify Watch didn't work");
  }

  ssize_t numRead;
  char buffer[IN_BUF_LEN];

  for(;;){
    numRead = read(inotify_fd, buffer, IN_BUF_LEN );
    thisFlowOut = (analogRead(FLOW_OUT_FILE) > 2500) ? 1 : 0;
    if( thisFlowOut != lastFlowOut ){
      pthread_mutex_lock(&flowOutMutex);
  		flowCountOut++;
      pthread_mutex_unlock(&flowOutMutex);
  	}
  	lastFlowOut = thisFlowOut;
  }
}

void * monitorFlowDump( void * ptr ){
  int inotify_fd = inotify_init();
  if( inotify_fd < 0 ){
    error("Inotify could not initialize");
  }

  int inotify_wd = inotify_add_watch( inotify_fd, FLOW_DUMP_FILE, IN_MODIFY);
  if( inotify_wd == -1 ){
    error("Inotify Watch didn't work");
  }

  ssize_t numRead;
  char buffer[IN_BUF_LEN];

  for(;;){
    numRead = read(inotify_fd, buffer, IN_BUF_LEN );
    thisFlowOut = (analogRead(FLOW_DUMP_FILE) > 2500) ? 1 : 0;
    if( thisFlowDump != lastFlowDump ){
      pthread_mutex_lock(&flowDumpMutex);
      flowCountDump++;
      pthread_mutex_unlock(&flowDumpMutex);
    }
    lastFlowDump = thisFlowDump;
  }
}



void sig_handler(int signo){
  if( signo == SIGINT ){
    printf("Handling SIGINT\n");
    persistenceCleanup();
    exit(0);
  }
}


void initialize(){
  if( signal(SIGINT, sig_handler) == SIG_ERR ){
    error("can't catch SIGINT\n");
  }
  if( signal(SIGPIPE, SIG_IGN) == SIG_ERR ){
    error("can't catch SIGPIPE\n");
  }

  tim.tv_sec = 1;
  tim.tv_nsec = 1000000L; // every millisecond, read sensor
  // NOTE: This should really be replaced with a timer circuit!!!
	persistenceInitialize();
  gettimeofday( &lastTime, NULL );

  printf("Starting Flow Sense threads\n");
  if( pthread_create( &flowOutThread, NULL, &monitorFlowOut, NULL ) != 0 ){
    error("Could not create out flow thread");
  }
  if( pthread_create( &flowDumpThread, NULL, &monitorFlowDump, NULL ) != 0 ){
    error("Could not create dump flow thread");
  }
}



int main(int argc, char** argv){
  printf("Initializing...\n");
  initialize();

  printf("Starting Sense and Report Loop\n");
  while( sleep(5) == 0){
    reportSensorValues();
  }

  return 0;
}
