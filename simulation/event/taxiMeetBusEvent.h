#ifndef TAXIMEETBUSEVENT_H
#define TAXIMEETBUSEVENT_H

#include<time.h>
#include"common.h"
#include"event.h"
#include"geometry.h"

struct TaxiMeetBusEvent
{ 
  char tname[NAME_LENGTH];
  char bname[2*NAME_LENGTH];
  struct Point gPoint;
  time_t timestamp;
};

int taxi_meet_bus_event_handler(void *nul, struct Simulator *aSim, struct TaxiMeetBusEvent *aTaxiMeetBusEvent);
void setup_taxi_meet_bus_events(struct Simulator *aSim, struct Hashtable *cntTable);

#endif
