#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <persistence.h>
#include <time.h>

#define DEBUG 1


CURL *curleasy;
CURLM *curlmulti;
int multihandles;

char * EDDI_ID;
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
    PERSIST_FILE = fopen(sprintf("data/%Lu",currentDay), "a");
    DAY_SINCE_EPOCH = currentDay;
  }
}

int persistenceInitialize(){
  EDDI_ID = getenv("EDDI_ID");
  if( EDDI_ID === NULL ){
    fprintf(stderr, "EDDI_ID Environment Variable must be set");
    exit(1);
  }

  checkDataFile();
}

void persistSenseSet(struct SenseSet *set){
  checkDataFile();

  char * dataEntry;
  sprintf(dataEntry, "%Lu|%f|%f|%d|%d|%d", set.timestamp, set.qOut, set.qDump, set.ppmOut, set.ppmIn, set.ppmRec);

  frwite(dataEntry, sizeOf(dataEntry[0]), sizeOf(dataEntry)/sizeOf(dataEntry[0]), PERSIST_FILE);
}

void persistenceCleanup(){
  if( fclose(PERSIST_FILE) != 0 ){
    fprintf(stderr, "Could not close file!");
    exit(1);
  }
}
