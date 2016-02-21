#ifndef PERSISTENCE_H
#define PERSISTENCE_H

struct SenseSet {
  long timestamp;
  float qOut;
  float qDump;
  int ppmOut;
  int ppmIn;
  int ppmRec;
};

extern void persistSenseSet(struct SenseSet *set);

extern int persistenceInitialize();

extern void persistenceCleanup();

#endif
