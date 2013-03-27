#include<stdlib.h>
#include<string.h>
#include"trace.h"
#include"geometry.h"
#include"contact.h"
#include"node.h"
#include"cntEvent.h"
#include"taxiMeetBusEvent.h"

int taxi_meet_bus_event_handler(void *nul, struct Simulator *aSim, struct TaxiMeetBusEvent *aTaxiMeetBusEvent)
{
	struct Node *tNode, *bNode;
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;
	char *onRoute;

	tNode = lookup_node(&aSim->vnodes, aTaxiMeetBusEvent->tname);
	bNode = lookup_node(&aSim->vnodes, aTaxiMeetBusEvent->bname);
	if(!tNode || !bNode)
		return 1;

	// taxi drop relay packets to a met bus
	aItem = tNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;

		if(!aSim->pkgRcdRoute) {
			onRoute = node_on_route(bNode);
			if(duallist_find(&aPkg->routingRecord, onRoute, (int(*)(void*,void*))are_strings_equal)) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, bNode, newPkg);
			} else {
				aSim->trafficCount ++;
			}
			if(aPkg->ttl>0)
				aPkg->ttl --;
			if(aPkg->ttl == 0) {
				temp = aItem->next;
				storage_remove_pkg(tNode->storage, aPkg->id);
				aItem = temp;
				continue;
			}
		}
		aItem = aItem->next;
	}

	// taxi pick up oldest packets from a met bus
	// Because of packets are sorted according to the start time,
	// pakets are selected to send from the head.
	aItem = bNode->storage->pkgs.head;
	while(aItem && (tNode->storage->size == -1 || (tNode->storage->size!=-1&&tNode->storage->usage+((struct Pkg*)aItem->datap)->size<tNode->storage->size)) ) {
		aPkg = (struct Pkg*)aItem->datap;
		newPkg = pkg_copy_func(aPkg);
		newPkg->ttl = aSim->pkgTTL;
		node_recv(aSim, tNode, newPkg);

		aItem = aItem->next;
	}

	return 0;
}


void setup_taxi_meet_bus_events(struct Simulator *aSim, struct Hashtable *cntTable)
{
	unsigned long i;
	struct Item *aItem, *aTaxiMeetBusItem;
	struct Pair *aPair; 
	struct Contact *aTaxiMeetBus;
	struct Event *aEvent;
	struct TaxiMeetBusEvent *aTaxiMeetBusEvent;

	if(cntTable == NULL || aSim == NULL)
		return;
	for(i=0;i<cntTable->size;i++) {
		aItem = cntTable->head[i];
		while(aItem!=NULL) {
			aPair = (struct Pair*)aItem->datap;
			/* setup events */
			if(aPair->vName1[0]=='b' && aPair->vName2[0]=='t' && aPair->contents.head != NULL) {
				aTaxiMeetBusItem = aPair->contents.head;
				while(aTaxiMeetBusItem) {
					aTaxiMeetBus = (struct Contact*)aTaxiMeetBusItem->datap;
					if(aTaxiMeetBus->startAt >= aSim->exprStartAt && aTaxiMeetBus->startAt <= aSim->exprEndAt) {
						aTaxiMeetBusEvent = (struct TaxiMeetBusEvent*)malloc(sizeof(struct TaxiMeetBusEvent));
						strncpy(aTaxiMeetBusEvent->bname, aPair->vName1, NAME_LENGTH);
						strncpy(aTaxiMeetBusEvent->tname, aPair->vName2, NAME_LENGTH);
						aTaxiMeetBusEvent->gPoint.x = aTaxiMeetBus->gPoint.x;
						aTaxiMeetBusEvent->gPoint.y = aTaxiMeetBus->gPoint.y;
						aTaxiMeetBusEvent->timestamp = aTaxiMeetBus->startAt;

						aEvent = (struct Event*)malloc(sizeof(struct Event));
						event_init_func(aEvent, aTaxiMeetBus->startAt, aSim, aTaxiMeetBusEvent, (int(*)(struct Simulator*, void*,void*))taxi_meet_bus_event_handler);
						add_event(aSim, aEvent);
					}

					if(aTaxiMeetBus->endAt >= aSim->exprStartAt && aTaxiMeetBus->endAt <= aSim->exprEndAt) {
						aTaxiMeetBusEvent = (struct TaxiMeetBusEvent*)malloc(sizeof(struct TaxiMeetBusEvent));
						strncpy(aTaxiMeetBusEvent->bname, aPair->vName1, NAME_LENGTH);
						strncpy(aTaxiMeetBusEvent->tname, aPair->vName2, NAME_LENGTH);
						aTaxiMeetBusEvent->gPoint.x = aTaxiMeetBus->gPoint.x;
						aTaxiMeetBusEvent->gPoint.y = aTaxiMeetBus->gPoint.y;
						aTaxiMeetBusEvent->timestamp = aTaxiMeetBus->endAt;

						aEvent = (struct Event*)malloc(sizeof(struct Event));
						event_init_func(aEvent, aTaxiMeetBus->endAt, aSim, aTaxiMeetBusEvent, (int(*)(struct Simulator*, void*,void*))taxi_meet_bus_event_handler);
						add_event(aSim, aEvent);

					} else if(aTaxiMeetBus->startAt > aSim->exprEndAt )
						break;
					aTaxiMeetBusItem = aTaxiMeetBusItem->next;
				}
			}
			aItem = aItem->next;
		}
	}
}
