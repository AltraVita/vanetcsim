#ifndef BUSARRIVECELLEVENT_H
#define BUSARRIVECELLEVENT_H

#include<time.h>
#include"common.h"
#include"event.h"
#include"geometry.h"

struct BusArriveCellEvent
{ 
  char bname[NAME_LENGTH];
  int xNumber;
  int yNumber;
  time_t timestamp;
};

int bus_arrive_cell_event_handler(void* nul, struct Simulator *aSim, struct BusArriveCellEvent *aBusArriveCellEvent);
void setup_bus_arrive_cell_events(struct Simulator *aSim, struct Hashtable *traceTable);

#endif

