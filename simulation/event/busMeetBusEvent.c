#include<stdlib.h>
#include<string.h>
#include"node.h"
#include"trace.h"
#include"geometry.h"
#include"contact.h"
#include"cntEvent.h"
#include"busMeetBusEvent.h"

int bus_meet_bus_event_handler(void* nul, struct Simulator *aSim, struct BusMeetBusEvent *aBusMeetBusEvent)
{
	struct Node *bNode1, *bNode2;
	struct Item *aItem, *temp;
	struct Pkg *aPkg, *newPkg;
	char *onRoute, *nextRoute;

	bNode1 = lookup_node(&aSim->vnodes, aBusMeetBusEvent->bname1);
	bNode2 = lookup_node(&aSim->vnodes, aBusMeetBusEvent->bname2);

	if(!bNode1 || !bNode2) 
		return 1;

	aItem = bNode1->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;

		if(aSim->pkgRcdRoute) {
			newPkg = pkg_copy_func(aPkg);
			node_recv(aSim, bNode2, newPkg);
		} else {
			onRoute = node_on_route(bNode2);
			if(aPkg->routingRecord.head) {
				nextRoute = (char*)aPkg->routingRecord.head->datap;
				if(are_strings_equal(nextRoute, onRoute)) {
					newPkg = pkg_copy_func(aPkg);
					if(node_recv(aSim, bNode2, newPkg)==0)
						if(aSim->fwdMethod == NO_REPLICA_FWD) {
							temp = aItem->next;
							storage_remove_pkg(bNode1->storage, aPkg->id);
							aItem = temp;
							continue;
						}
				}
			}

		}
		aItem = aItem->next;
	}

	aItem = bNode2->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;

		if(aSim->pkgRcdRoute) {
			newPkg = pkg_copy_func(aPkg);
			node_recv(aSim, bNode1, newPkg);
		} else {
			onRoute = node_on_route(bNode1);
			if(aPkg->routingRecord.head) {
				nextRoute = (char*)aPkg->routingRecord.head->datap;
				if(are_strings_equal(nextRoute, onRoute)) {
					newPkg = pkg_copy_func(aPkg);
					if(node_recv(aSim, bNode1, newPkg)==0)
						if(aSim->fwdMethod == NO_REPLICA_FWD) {
							temp = aItem->next;
							storage_remove_pkg(bNode2->storage, aPkg->id);
							aItem = temp;
							continue;
						}
				}
			}
		}
		aItem = aItem->next;
	}
	
	return 0;
}


void setup_bus_meet_bus_events(struct Simulator *aSim, struct Hashtable *cntTable)
{
	unsigned long i;
	struct Item *aItem, *aBusMeetBusItem;
	struct Pair *aPair; 
	struct Contact *aBusMeetBus;
	struct Event *aEvent;
	struct BusMeetBusEvent *aBusMeetBusEvent;

	if(cntTable == NULL || aSim == NULL)
		return;
	for(i=0;i<cntTable->size;i++) {
		aItem = cntTable->head[i];
		while(aItem!=NULL) {
			aPair = (struct Pair*)aItem->datap;
			/* setup events */
			if(aPair->vName1[0]=='b' && aPair->vName2[0]=='b' && aPair->contents.head != NULL) {
				aBusMeetBusItem = aPair->contents.head;
				while(aBusMeetBusItem) {
					aBusMeetBus = (struct Contact*)aBusMeetBusItem->datap;
					if(aBusMeetBus->startAt >= aSim->exprStartAt && aBusMeetBus->startAt <= aSim->exprEndAt) {
						aBusMeetBusEvent = (struct BusMeetBusEvent*)malloc(sizeof(struct BusMeetBusEvent));
						strncpy(aBusMeetBusEvent->bname1, aPair->vName1, 2*NAME_LENGTH);
						strncpy(aBusMeetBusEvent->bname2, aPair->vName2, 2*NAME_LENGTH);
						aBusMeetBusEvent->gPoint.x = aBusMeetBus->gPoint.x;
						aBusMeetBusEvent->gPoint.y = aBusMeetBus->gPoint.y;
						aBusMeetBusEvent->timestamp = aBusMeetBus->startAt;

						aEvent = (struct Event*)malloc(sizeof(struct Event));
						event_init_func(aEvent, aBusMeetBus->startAt, aSim, aBusMeetBusEvent, (int(*)(struct Simulator*, void*,void*))bus_meet_bus_event_handler);
						add_event(aSim, aEvent);
					}

					if(aBusMeetBus->endAt >= aSim->exprStartAt && aBusMeetBus->endAt <= aSim->exprEndAt) {
						aBusMeetBusEvent = (struct BusMeetBusEvent*)malloc(sizeof(struct BusMeetBusEvent));
						strncpy(aBusMeetBusEvent->bname1, aPair->vName1, 2*NAME_LENGTH);
						strncpy(aBusMeetBusEvent->bname2, aPair->vName2, 2*NAME_LENGTH);
						aBusMeetBusEvent->gPoint.x = aBusMeetBus->gPoint.x;
						aBusMeetBusEvent->gPoint.y = aBusMeetBus->gPoint.y;
						aBusMeetBusEvent->timestamp = aBusMeetBus->endAt;

						aEvent = (struct Event*)malloc(sizeof(struct Event));
						event_init_func(aEvent, aBusMeetBus->endAt, aSim, aBusMeetBusEvent, (int(*)(struct Simulator*, void*,void*))bus_meet_bus_event_handler);
						add_event(aSim, aEvent);

					} else if(aBusMeetBus->startAt > aSim->exprEndAt )
						break;
					aBusMeetBusItem = aBusMeetBusItem->next;
				}
			}
			aItem = aItem->next;
		}
	}
}
