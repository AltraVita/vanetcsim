#ifndef BUSMEETSTORAGEEVENT_H
#define BUSMEETSTORAGEEVENT_H

#include<time.h>
#include"common.h"
#include"event.h"
#include"geometry.h"

struct BusMeetStorageEvent
{ 
  char bname[2*NAME_LENGTH];
  char sname[NAME_LENGTH];
  time_t timestamp;
};

int bus_meet_storage_event_handler(void* nul, struct Simulator *aSim, struct BusMeetStorageEvent *aBusMeetStorageEvent);
void setup_bus_meet_storage_events(struct Simulator *aSim, struct Hashtable *traceTable);

#endif

