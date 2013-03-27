#include<stdlib.h>
#include<string.h>
#include"trace.h"
#include"geometry.h"
#include"busArriveCellEvent.h"
#include"node.h"


int bus_arrive_cell_event_handler(void* nul, struct Simulator *aSim, struct BusArriveCellEvent *aBusArriveCellEvent)
{
	struct Node *aNode;
	struct Item *aItem, *bItem, *temp;
	char buf[32];
	struct Pkg *aPkg, *bPkg;

	sprintf(buf, "%d,%d", aBusArriveCellEvent->xNumber, aBusArriveCellEvent->yNumber);
	aNode = lookup_node(&aSim->vnodes, aBusArriveCellEvent->bname);
	aItem = aNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		if(strcmp(aPkg->dst, buf)==0) {
			aPkg->endAt = aBusArriveCellEvent->timestamp;
			bItem = duallist_find(&aSim->pkgs, &aPkg->id, (int(*)(void*,void*))pkg_has_id);
			bPkg = (struct Pkg*)bItem->datap;
			if( bPkg->endAt == 0 
			|| (bPkg->endAt && aPkg->endAt<bPkg->endAt) 
			|| (bPkg->endAt && aPkg->endAt==bPkg->endAt && aPkg->routingRecord.nItems < bPkg->routingRecord.nItems)) {
				bPkg->endAt = aSim->clock;
				duallist_destroy(&bPkg->routingRecord, free);
				duallist_copy(&bPkg->routingRecord, &aPkg->routingRecord,(void*(*)(void*))address_copy_func);
			} 
			temp = aItem->next;
			storage_remove_pkg(aNode->storage, aPkg->id);
			aItem = temp;
			continue;
		}  
		aItem = aItem->next;
	}
	return 0;
}

void setup_bus_arrive_cell_events(struct Simulator *aSim, struct Hashtable *traceTable)
{
	unsigned long i;
	struct Item *aItem, *aRepItem;
	struct Trace *aTrace; 
	struct Report *aRep;
	struct Cell *aCell;
	struct Event *aEvent;
	char buf[64];
	struct BusArriveCellEvent *aBusArriveCellEvent;

	if(traceTable == NULL || aSim->region == NULL || aSim == NULL)
		return;
	for(i=0;i<traceTable->size;i++) {
		aItem = traceTable->head[i];
		while(aItem!=NULL) {
			aTrace = (struct Trace*)aItem->datap;
			/* setup events */
			if(aTrace->vName[0]=='b' && aTrace->reports.head != NULL) {
				aRepItem = aTrace->reports.head;
				while(aRepItem != NULL ) {
					aRep = (struct Report*)aRepItem->datap;
					if(aRep->timestamp >= aSim->exprStartAt && aRep->timestamp <= aSim->exprEndAt) {
						aCell = point_in_cell(aSim->region, &aRep->gPoint); 
						sprintf(buf, "%d,%d", aCell->xNumber, aCell->yNumber);
						if(hashtable_find(&aSim->destinations, buf)) {
							if(is_report_in_upway(aRep))
								sprintf(buf, "%s@%s_upway",aTrace->vName, aTrace->onRoute);
							else
								sprintf(buf, "%s@%s_downway", aTrace->vName, aTrace->onRoute);
							aBusArriveCellEvent = (struct BusArriveCellEvent*)malloc(sizeof(struct BusArriveCellEvent));
							strncpy(aBusArriveCellEvent->bname, buf, NAME_LENGTH);
							aBusArriveCellEvent->xNumber = aCell->xNumber;
							aBusArriveCellEvent->yNumber = aCell->yNumber;
							aBusArriveCellEvent->timestamp = aRep->timestamp;

							aEvent = (struct Event*)malloc(sizeof(struct Event));
							event_init_func(aEvent, aRep->timestamp, aSim, aBusArriveCellEvent, (int(*)(struct Simulator*, void*,void*))bus_arrive_cell_event_handler);
							add_event(aSim, aEvent);
						}
					} else if(aRep->timestamp > aSim->exprEndAt )
						break;
					aRepItem = aRepItem->next;
				}
			}
			aItem = aItem->next;
		}
	}
}
