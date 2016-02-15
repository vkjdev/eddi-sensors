#ifndef PERSISTENCE_H
#define PERSISTENCE_H

struct senseSet {
  long timestamp;
  float qOut;
  float qDump;
  int ppmOut;
  int ppmIn;
  int ppmRec;
};

extern void persistSenseSet(struct senseSet *set);

extern int persistenceInitialize();

extern void persistenceCleanup();

#endif
