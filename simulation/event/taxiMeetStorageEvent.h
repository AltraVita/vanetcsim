#ifndef TAXIMEETSTORAGEEVENT_H
#define TAXIMEETSTORAGEEVENT_H

#include<time.h>
#include"common.h"
#include"event.h"
#include"geometry.h"

struct TaxiMeetStorageEvent
{ 
  char tname[NAME_LENGTH];
  char sname[NAME_LENGTH];
  time_t timestamp;
};

int taxi_meet_storage_event_handler(void *nul, struct Simulator *aSim, struct TaxiMeetStorageEvent *aTaxiMeetStorageEvent);
void setup_taxi_meet_storage_events(struct Simulator *aSim, struct Hashtable *traceTable);

#endif

