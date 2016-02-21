#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <persistence.h>
#include <time.h>

#define DEBUG 1


FILE * PERSIST_FILE;
unsigned long DAY_SINCE_EPOCH;


void checkDataFile(){
  time_t currentTime = time(NULL);
  unsigned long currentDay = currentTime % 86400L;

  if( currentDay != DAY_SINCE_EPOCH ){
    if( fclose(PERSIST_FILE) != 0 ){
      fprintf(stderr, "Could not close file!");
      exit(1);
    }
    char * fileName;
    sprintf(fileName, "data/%Lu", currentDay);
    PERSIST_FILE = fopen(fileName, "a");
    DAY_SINCE_EPOCH = currentDay;
  }
}

int persistenceInitialize(){
  checkDataFile();
}

void persistSenseSet(struct SenseSet *set){
  checkDataFile();

  char * dataEntry;
  sprintf(dataEntry, "%Lu|%f|%f|%d|%d|%d", set.timestamp, set.qOut, set.qDump, set.ppmOut, set.ppmIn, set.ppmRec);

  fwrite(dataEntry, sizeof(dataEntry[0]), sizeof(dataEntry)/sizeof(dataEntry[0]), PERSIST_FILE);
}

void persistenceCleanup(){
  if( fclose(PERSIST_FILE) != 0 ){
    fprintf(stderr, "Could not close file!");
    exit(1);
  }
}
