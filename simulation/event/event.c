#include<stdlib.h>
#include<time.h>
#include"common.h"
#include"event.h"

void event_init_func(struct Event *aEvent, time_t timestamp, void *byWho, void *datap, int(*handler_func)(struct Simulator*, void*, void*))
{
	if(aEvent == NULL) {
		printf("A NULL event!\n");
	}
	aEvent->timestamp = timestamp;
	aEvent->byWho = byWho;
	aEvent->datap = datap;
	aEvent->handler_func = handler_func;
}

int event_has_earlier_timestamp_than(struct Event *aEvent, struct Event *bEvent)
{
	return aEvent->timestamp < bEvent->timestamp;
}


int event_has_later_timestamp_than(struct Event *aEvent, struct Event *bEvent)
{
	return aEvent->timestamp > bEvent->timestamp;
}

void add_event(struct Simulator *aSim, struct Event *aEvent)
{
	int whichslot;

	if(aSim == NULL) {
		printf("A NULL simulator!\n");
	}
	whichslot = (aEvent->timestamp-aSim->exprStartAt)/aSim->slotSize;
	if(whichslot>99)
		whichslot = 99;
	duallist_add_in_sequence_from_tail(aSim->eventSlots+whichslot, aEvent, (int(*)(void*, void*))event_has_earlier_timestamp_than);
	aSim->eventNums ++;
}

						
int consume_an_event(struct Simulator *aSim)
{
	struct Event *aEvent;
	int i;

	if(aSim == NULL) {
		printf("A NULL simulator!\n");
		return -1;
	}
	for(i=0;i<EVENT_SLOTS;i++) {
		if(aSim->eventSlots[i].head!=NULL) {
			aEvent = (struct Event*)duallist_pick_head(&aSim->eventSlots[i]);
			aSim->clock = aEvent->timestamp;
			aEvent->handler_func(aSim, aEvent->byWho, aEvent->datap);
			free(aEvent);
			return 1;
		}
	}
	return 0;
}

