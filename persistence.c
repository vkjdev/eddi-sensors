#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <persistence.h>
#include <time.h>



FILE * PERSIST_FILE;
unsigned long DAY_SINCE_EPOCH;


void checkDataFile(){
  time_t currentTime = time(NULL);
  unsigned long currentDay = currentTime / 86400L;

  if( currentDay != DAY_SINCE_EPOCH ){
    if( PERSIST_FILE != NULL && fclose(PERSIST_FILE) != 0 ){
      printf("Could not close file!\n");
      exit(1);
    }
    char fileName[20];
    sprintf(fileName, "./data/%lu.dat", currentDay);
    printf("Opening Persist File: %s\n", fileName);
    PERSIST_FILE = fopen(fileName, "a+");
    if( PERSIST_FILE == NULL ){
      printf("Could not open Persist File: %s\n", fileName);
      exit(1);
    }
    DAY_SINCE_EPOCH = currentDay;
  }
}

void persistenceInitialize(){
  checkDataFile();
}

void persistSenseSet(struct SenseSet *setPtr){
  checkDataFile();

  struct SenseSet set = *setPtr;

  printf("Persisting Sense Set for Timestamp %lu\n", set.timestamp);

  fprintf(PERSIST_FILE, "%lu|%f|%f|%d|%d|%d\n", set.timestamp, set.qOut, set.qDump, set.ppmOut, set.ppmIn, set.ppmRec);
  fflush(PERSIST_FILE);
}

void persistenceCleanup(){
  if( fclose(PERSIST_FILE) != 0 ){
    fprintf(stderr, "Could not close file!");
    exit(1);
  }
}
