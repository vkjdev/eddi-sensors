#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
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

// Files for Sensor Pins
#define FLOW_OUT_FILE 	"/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw";
#define FLOW_DUMP_FILE 	"/sys/devices/12d10000.adc/iio:device0/in_voltage1_raw";
#define SAL_IN_FILE 		"/sys/devices/12d10000.adc/iio:device0/in_voltage2_raw";
#define SAL_OUT_FILE 		"/sys/devices/12d10000.adc/iio:device0/in_voltage3_raw";
#define SAL_RECIRC_FILE "/sys/devices/12d10000.adc/iio:device0/in_voltage4_raw";

// Constants
const long int salResistorOhms = 1000000L;
const float salSensorSpacingCM = 0.3;
const float salSensorAreaCMSq  = 1.0;

// Timing
struct timespec tim;
struct timespec timrem;

// tracking sensor values
unsigned int lastFlowOut, lastFlowDump, thisFlowOut, thisFlowDump;
long flowCountOut, flowCountDump;
struct timeval lastTime, thisTime;


static int analogRead(#define fName) {
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


static int ppmFromVoltage(int millivolts){
	// 1) Voltage Dividers!
	// Req = R1 + R2
	// I = 5 / Req
	// V1 = I R1
	// v1 = (5/Req) * R1
	// r1 = (v1 * Req) / 5
	// r1 = (v1R1 / 5) + (v1R2 / 5)
	// 5r1 = v1r1 + v1R2
	// r1(5 - v1) = v1R2
	// r1 = (v1*r2) / (5 - v1)
	// r1 = (v1/v2) * r2
  float ohms = (millivolts / (5000.0 - millivolts)) * salResistorOhms;

	// TODO: Need to test in order to establish constants for the function of resistance to tds

	return ohms / 100;
}


int reportSensorValues(){
	gettimeofday( &thisTime, NULL );
	float elapsed = (float)(((long long int)thisTime.tv_sec * 1000000L + thisTime.tv_usec) - ((long long int)lastTime.tv_sec * 1000000L + lastTime.tv_usec))/1000000.0;
	float outFrequency = flowCountOut / elapsed;
	float dumpFrequency = flowCountRec / elapsed;

	struct SenseSet sense;

	sense.qOut 			= outFrequency / 5.5;
	sense.qDump 		= dumpFrequency / 5.5;
	sense.ppmOut 		= ppmFromVoltage(analogRead(SAL_OUT_FILE));
	sense.ppmIn 		= ppmFromVoltage(analogRead(SAL_IN_FILE));
	sense.ppmRecirc = ppmFromVoltage(analogRead(SAL_RECIRC_FILE));

	persistSenseSet(&sense);

	lastTime = thisTime;
	flowCountOut = 0;
	flowCountIn = 0;

	return 0;
}


void checkFlowSensors(){
	// get flow out
	thisFlowOut = analogRead(FLOW_OUT_FILE);
	if( thisFlowOut != lastFlowOut ){
		flowCountOut++;
	}
	lastFlowOut = thisFlowOut;

	// get flow in
	thisFlowDump = analogRead(FLOW_DUMP_FILE);
	if( thisFlowDump != lastFlowDump ){
		flowCountDump++;
	}
	lastFlowDump = thisFlowDump;
}






void initializeMain(){
  tim.tv_sec = 0;
  tim.tv_nsec = 1000000L; // every millisecond, read sensor
  // NOTE: This should really be replaced with a timer circuit!!!
}

void initializeSensors(){
	persistenceInitialize();
  lastFlowOut = analogRead(FLOW_OUT_FILE);
  lastFlowDump = analogRead(FLOW_DUMP_FILE);
  gettimeofday( &lastTime, NULL );
}





int main(int argc, char** argv){
  initializeMain();
	initializeSensors();

  int iterationCount = 0; // marks the approximate millisecond
  while(nanosleep(&tim, &timrem) == 0){
  	checkFlowSensors();

    // determine control state every 5 seconds
    if( iterationCount % 5000 == 0 ){
			determineControlState();
    }

    // log flow rate every 60 seconds
    if( iterationCount == 59999 ){
      reportSensorValues();
      iterationCount = 0;
    } else {
      iterationCount++;
    }
  }
}
