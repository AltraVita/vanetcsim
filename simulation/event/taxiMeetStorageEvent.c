#include<stdlib.h>
#include<string.h>
#include"trace.h"
#include"geometry.h"
#include"node.h"
#include"busroute.h"
#include"cntEvent.h"
#include"taxiMeetStorageEvent.h"

int taxi_meet_storage_event_handler(void *nul, struct Simulator *aSim, struct TaxiMeetStorageEvent *aTaxiMeetStorageEvent)
{
	struct Node *tNode, *sNode;
	struct Item *aItem, *bItem, *temp;
	char *strp;
	struct Pkg *aPkg, *newPkg;
	struct Cell *aCell;
	int found;

	tNode = lookup_node(&aSim->vnodes, aTaxiMeetStorageEvent->tname);
	sNode = lookup_node(&aSim->snodes, aTaxiMeetStorageEvent->sname);
	aCell = point_in_cell(aSim->region, &sNode->gPoint);

	aItem = tNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;

		if(!aSim->pkgRcdRoute) {
			found = 0;
			bItem = aPkg->routingRecord.head;
			while(bItem) {
				strp = (char*)bItem->datap;
				if(duallist_find(&aCell->routes, strp, (int(*)(void*,void*))route_has_name)) {
					found = 1;
					break;
				}
				bItem = bItem->next;
			}
			if(found) {
				newPkg = pkg_copy_func(aPkg);
				node_recv(aSim, sNode, newPkg);
			} else {
				aSim->trafficCount ++;
			}
			if(aPkg->ttl > 0)
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

	// taxi pick up oldest packets from a met roadside storage 
	// Because of packets are sorted according to the start time,
	// pakets are selected to send from the head.
	aItem = sNode->storage->pkgs.head;
	while(aItem && (tNode->storage->size == -1 || (tNode->storage->size!=-1&&tNode->storage->usage+((struct Pkg*)aItem->datap)->size<tNode->storage->size)) ) {
		aPkg = (struct Pkg*)aItem->datap;
		newPkg = pkg_copy_func(aPkg);
		node_recv(aSim, tNode, newPkg);

		aItem = aItem->next;
	}

	return 0;
}



void setup_taxi_meet_storage_events(struct Simulator *aSim, struct Hashtable *traceTable)
{
	unsigned long i;
	struct Item *aItem, *aRepItem;
	struct Trace *aTrace; 
	struct Report *aRep;
	struct Cell *aCell;
	struct Event *aEvent;
	struct TaxiMeetStorageEvent *aTaxiMeetStorageEvent;

	if(traceTable == NULL || aSim->region == NULL || aSim == NULL)
		return;
	for(i=0;i<traceTable->size;i++) {
		aItem = traceTable->head[i];
		while(aItem!=NULL) {
			aTrace = (struct Trace*)aItem->datap;
			/* setup events */
			if(aTrace->vName[0]=='t' && aTrace->reports.head != NULL) {
				aRepItem = aTrace->reports.head;
				while(aRepItem != NULL ) {
					aRep = (struct Report*)aRepItem->datap;
					aCell = point_in_cell(aSim->region, &aRep->gPoint); 
					if(aRep->timestamp >= aSim->exprStartAt && aRep->timestamp <= aSim->exprEndAt && aCell->storage) {
						aTaxiMeetStorageEvent = (struct TaxiMeetStorageEvent*)malloc(sizeof(struct TaxiMeetStorageEvent));
						strncpy(aTaxiMeetStorageEvent->tname, aTrace->vName, NAME_LENGTH);
						strncpy(aTaxiMeetStorageEvent->sname, aCell->storage->name, NAME_LENGTH);
						aTaxiMeetStorageEvent->timestamp = aRep->timestamp;

						aEvent = (struct Event*)malloc(sizeof(struct Event));
						event_init_func(aEvent, aRep->timestamp, aSim, aTaxiMeetStorageEvent, (int(*)(struct Simulator*, void*,void*))taxi_meet_storage_event_handler);
						add_event(aSim, aEvent);

					} else if(aRep->timestamp > aSim->exprEndAt )
						break;
					aRepItem = aRepItem->next;
				}
			}
			aItem = aItem->next;
		}
	}


}
