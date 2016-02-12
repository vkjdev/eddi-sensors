#include <stdio.h>
#include <time.h>

const char * flowOutFile = "/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw";
const char * flowInFile = "/sys/devices/12d10000.adc/iio:device0/in_voltage1_raw";

struct timespec tim, timrem;
tim.tv_sec = 0;
tim.tv_nsec = 1000000L;

unsigned int lastFlowOut, lastFlowIn, thisFlowOut, thisFlowIn, 
signed int flowCountOut, flowCountIn;

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


int main(int argc, char** argv)
{

  lastFlowOut = analogRead(flowOutFile);
  lastFlowIn = analogRead(flowInFile);
  gettimeofday( &lastTime, NULL );
  int iterationCount = 0;
  
  while(nanosleep(tim, timrem) == 0){
    // get flow out
    thisFlowOut = analogRead(flowOutFile);
    if( thisFlowOut != lastFlowOut ){
      flowCountIn++;
    }
    lastFlowOut = thisFlowOut;
    
    // get flow in
    thisFlowIn = analogRead(flowInFile);
    if( thisFlowIn != lastFlowIn ){
      flowCountOut++
    }
    lastFlowIn = thisFlowIn;
    
    // log flow rate if at limit
    if( iterationCount == 1000 ){
      gettimeofday( &thisTime, NULL );
      float elapsed = (float)(((long long int)thisTime.tv_sec * 1000000 + thisTime.tv_usec) - ((long long int)lastTime.tv_sec * 1000000 + lastTime.tv_usec));
      float outFrequency = flowCountOut / elapsed
      float inFrequency = flowCountIn / elapsed
      
      float outQ = outFrequency / 5.5;
      float inQ = inFrequency / 5.5;
      
      printf("Out Q: %f\n", outQ);
      printf("In  Q: $f\n", inQ);
      
      iterationCount = 0;
    }
  }
  
  
  
}
