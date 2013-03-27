#include<stdlib.h>
#include<string.h>
#include"busMeetStorageEvent.h"
#include"trace.h"
#include"geometry.h"
#include"node.h"
#include"busroute.h"
#include"cntEvent.h"

int bus_meet_storage_event_handler(void* nul, struct Simulator *aSim, struct BusMeetStorageEvent *aBusMeetStorageEvent)
{
	struct Node *bNode, *sNode;
	struct Item *aItem, *bItem, *temp;
	char *strp, *onRoute;
	struct Pkg *aPkg, *newPkg;
	struct Cell *aCell;
	int found;

	bNode = lookup_node(&aSim->vnodes, aBusMeetStorageEvent->bname);
	sNode = lookup_node(&aSim->snodes, aBusMeetStorageEvent->sname);

	aCell = point_in_cell(aSim->region, &sNode->gPoint);
	aItem = bNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;

		if(aSim->pkgRcdRoute) {
			newPkg = pkg_copy_func(aPkg);
			node_recv(aSim, sNode, newPkg);
		} else {
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
				if(node_recv(aSim, sNode, newPkg)==0)
					if(aSim->fwdMethod == NO_REPLICA_FWD) {
						temp = aItem->next;
						storage_remove_pkg(bNode->storage, aPkg->id);
						aItem = temp;
						continue;
					}
			}
		}
		aItem = aItem->next;
	}

	aItem = sNode->storage->pkgs.head;
	while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;

		if(aSim->pkgRcdRoute) {
			newPkg = pkg_copy_func(aPkg);
			node_recv(aSim, sNode, newPkg);
		} else {
			onRoute = node_on_route(bNode);
			if(duallist_find(&aPkg->routingRecord, onRoute, (int(*)(void*,void*))are_strings_equal)) {
				newPkg = pkg_copy_func(aPkg);
				if(node_recv(aSim, bNode, newPkg)==0)
					if(aSim->fwdMethod == NO_REPLICA_FWD) {
						temp = aItem->next;
						storage_remove_pkg(sNode->storage, aPkg->id);
						aItem = temp;
						continue;
					}
			}
		}
		aItem = aItem->next;
	}

	return 0;
}


void setup_bus_meet_storage_events(struct Simulator *aSim, struct Hashtable *traceTable)
{
	unsigned long i;
	struct Item *aItem, *aRepItem;
	struct Trace *aTrace; 
	struct Report *aRep;
	struct Cell *aCell;
	struct Event *aEvent;
	char bname[64];
	struct BusMeetStorageEvent *aBusMeetStorageEvent;

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
					aCell = point_in_cell(aSim->region, &aRep->gPoint); 
					if(aRep->timestamp >= aSim->exprStartAt && aRep->timestamp <= aSim->exprEndAt && aCell->storage) {
						if(is_report_in_upway(aRep))
							sprintf(bname, "%s@%s_upway",aTrace->vName, aTrace->onRoute);
						else
							sprintf(bname, "%s@%s_downway", aTrace->vName, aTrace->onRoute);
						aBusMeetStorageEvent = (struct BusMeetStorageEvent*)malloc(sizeof(struct BusMeetStorageEvent));
						strncpy(aBusMeetStorageEvent->bname, bname, NAME_LENGTH);
						strncpy(aBusMeetStorageEvent->sname, aCell->storage->name, NAME_LENGTH);
						aBusMeetStorageEvent->timestamp = aRep->timestamp;

						aEvent = (struct Event*)malloc(sizeof(struct Event));
						event_init_func(aEvent, aRep->timestamp, aSim, aBusMeetStorageEvent, (int(*)(struct Simulator*, void*,void*))bus_meet_storage_event_handler);
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
