#ifndef BUSMEETBUSEVENT_H
#define BUSMEETBUSEVENT_H

#include<time.h>
#include"common.h"
#include"event.h"
#include"geometry.h"

struct BusMeetBusEvent
{ 
  char bname1[2*NAME_LENGTH];
  char bname2[2*NAME_LENGTH];
  struct Point gPoint;
  time_t timestamp;
};

int bus_meet_bus_event_handler(void* nul, struct Simulator *aSim, struct BusMeetBusEvent *aBusMeetBusEvent);
void setup_bus_meet_bus_events(struct Simulator *aSim, struct Hashtable *cntTable);

#endif
