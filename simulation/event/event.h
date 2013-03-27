#ifndef EVENT_H
#define EVENT_H

#include<time.h>
#include"simulator.h"

struct Event
{
  time_t timestamp;
  void *byWho;
  void *datap;
  int(*handler_func)(struct Simulator*, void*, void*);
};

void event_init_func(struct Event *aEvent, time_t timestamp, void *byWho, void *datap, int(*handler_func)(struct Simulator*, void*, void*));
int event_has_earlier_timestamp_than(struct Event *aEvent, struct Event *bEvent);
int event_has_later_timestamp_than(struct Event *aEvent, struct Event *bEvent);
void add_event(struct Simulator *aSim, struct Event *aEvent);
int consume_an_event(struct Simulator *aSim);

#endif
