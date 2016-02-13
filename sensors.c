#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

char * flowOutFile =
	"/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw";
char * flowRecircFile =
	"/sys/devices/12d10000.adc/iio:device0/in_voltage1_raw";

char * salOutFile =
	"/sys/devices/12d10000.adc/iio:device0/in_voltage2_raw";
char * salRecircFile =
	"/sys/devices/12d10000.adc/iio:device0/in_voltage3_raw";

struct timespec tim;
struct timespec timrem;

unsigned int lastFlowOut, lastflowRecirc, thisFlowOut, thisflowRecirc;
signed int flowCountOut, flowCountRecirc;

struct timeval lastTime, thisTime;


int analogRead(char * fName) {
  FILE * fd;
  char val[8];

  // open value file
  if((fd = fopen(fName, "r")) == NULL) {
     printf("Error: can't open analog voltage value\n");
     return 0;
  }
  fgets(val, 8, fd);
  fclose(fd);
  if( atoi(val) > 2500 ){
    return 1;
  } else {
    return 0;
  }
}

int reportValues(){
	gettimeofday( &thisTime, NULL );
	float elapsed = (float)(((long long int)thisTime.tv_sec * 1000000L + thisTime.tv_usec) - ((long long int)lastTime.tv_sec * 1000000L + lastTime.tv_usec))/(double)1000000;
	float outFrequency = flowCountOut / elapsed;
	float recFrequency = flowCountRec / elapsed;

	float outQ = outFrequency / 5.5;
	float recQ = recFrequency / 5.5;

	float outResistance = analogRead(salOutFile);
	float recircResistance = analogRead(salRecircFile);

	printf("Elapsed: %f\n", elapsed);
	printf("Out    Q: %f\n", outQ);
	printf("Recirc Q: %f\n", recQ);
	// printf("Out    TDS: %f\n", outResistance);
	// printf("Recirc TDS: %f\n", recircResistance);

	lastTime = thisTime;
	flowCountOut = 0;
	flowCountIn = 0;
}


int main(int argc, char** argv)
{
  lastFlowOut = analogRead(flowOutFile);
  lastflowRecirc = analogRead(flowRecircFile);
  gettimeofday( &lastTime, NULL );

  tim.tv_sec = 0;
  tim.tv_nsec = 1000000L;

 int iterationCount = 0;
  while(nanosleep(&tim, &timrem) == 0){
    // get flow out
    thisFlowOut = analogRead(flowOutFile);
    if( thisFlowOut != lastFlowOut ){
      flowCountRecirc++;
    }
    lastFlowOut = thisFlowOut;

    // get flow in
    thisflowRecirc = analogRead(flowRecircFile);
    if( thisflowRecirc != lastflowRecirc ){
      flowCountOut++;
    }
    lastflowRecirc = thisflowRecirc;

    // log flow rate if at limit
    if( iterationCount == 1000 ){
      reportValues();
      iterationCount = 0;
    } else {
      iterationCount++;
    }
  }



}
