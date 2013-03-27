#include <string.h>
#include <stdlib.h>
//#include <glib/gnode.h>
#include "busroute.h"
#include "rnorrexp.h"
#include "files.h"
#include "node.h"

int route_has_name(char *name, struct Busroute* aRoute)
{
	return !strcmp(name, aRoute->name);
}

void route_init_func(struct Busroute *aRoute)
{
	int randint;
	if(aRoute == NULL)
		return;
	memset(aRoute->name, '\0', NAME_LENGTH);
	srand(time(NULL));
	randint = rand();
	aRoute->color.integer = randint;
	aRoute->color.rgbo.opacity = 125;
	aRoute->box.xmin = 0;
	aRoute->box.xmax = 0;
	aRoute->box.ymin = 0;
	aRoute->box.ymax = 0;
	aRoute->path = (struct Path*)malloc(sizeof(struct Path));
	path_init_func(aRoute->path);
	duallist_init(&aRoute->stops);
}


void route_free_func(struct Busroute *aRoute) 
{
	if(aRoute == NULL) 
		return;
	if(aRoute->path)
		path_free_func(aRoute->path); 
	duallist_destroy(&aRoute->stops, NULL);
	free(aRoute);
}

void dump_route(FILE *fOutput, struct Busroute *aRoute) 
{
	unsigned long i;
	struct Item *aItem;
	struct Road *aRoad;
	struct Stop *aStop;

  	fprintf(fOutput, "%d\n", FILE_BUS_ROUTE);
	fprintf(fOutput, "%s\n", aRoute->name);
	fprintf(fOutput, "%f %f %f %f\n", aRoute->box.xmin, aRoute->box.xmax, aRoute->box.ymin, aRoute->box.ymax);

	if(aRoute->path) {
		fprintf(fOutput, "%lu\n", aRoute->path->roads.nItems);
		aItem = aRoute->path->roads.head;
		for(i=0;i<aRoute->path->roads.nItems;i++) {
			aRoad = (struct Road*)aItem->datap;
			fprintf(fOutput, "%d\n", aRoad->id);
			aItem = aItem->next;
		}
		fprintf(fOutput, "%.2lf\n", aRoute->path->length);
		fprintf(fOutput, "%d\n", aRoute->path->turns);
	} else 
		fprintf(fOutput, "0\n");

	fprintf(fOutput, "%lu\n", aRoute->stops.nItems);
	aItem = aRoute->stops.head;
	for(i=0;i<aRoute->stops.nItems;i++) {
		aStop = (struct Stop*)aItem->datap;
		fprintf(fOutput, "%d\n", aStop->id);
		aItem = aItem->next;
	}

}


struct Busroute *load_route(FILE *fInput, struct Region *aRegion)
{
	struct Busroute *newRoute;
	struct Item *aItem;
	struct Road *aRoad;
	int id;
	unsigned long i, nItems;

	if(aRegion == NULL)
		return NULL;
	newRoute = (struct Busroute*)malloc(sizeof(struct Busroute));
	route_init_func(newRoute);

	fscanf(fInput, "%s\n", newRoute->name);
	fscanf(fInput, "%lf %lf %lf %lf\n", &newRoute->box.xmin, &newRoute->box.xmax, &newRoute->box.ymin, &newRoute->box.ymax);

	fscanf(fInput, "%lu\n", &nItems);
	if(nItems){
		for(i=0;i<nItems;i++) {
			fscanf(fInput, "%d\n", &id);
			if ((aItem = duallist_find(&aRegion->roads, &id, (int(*)(void*,void*))road_has_id))!=NULL) {
				aRoad = (struct Road*)aItem->datap;
				duallist_add_to_tail(&newRoute->path->roads, aRoad);
			}
		}
		fscanf(fInput, "%lf\n", &newRoute->path->length);
		fscanf(fInput, "%d\n", &newRoute->path->turns);
	}

	fscanf(fInput, "%lu\n", &nItems);
	for(i=0;i<nItems;i++) {
		fscanf(fInput, "%d\n", &id);
		duallist_add_to_tail(&newRoute->stops, (int*)id);
	}

	return newRoute;
}


void load_route_with_hashtable(FILE *froute, struct Region *aRegion, struct Hashtable *routes) 
{
	struct Busroute *aRoute;

	if(aRegion == NULL || routes == NULL)
		return ;
	aRoute = load_route(froute, aRegion);
	if(aRoute != NULL) {
		hashtable_add(routes, aRoute->name, aRoute);
	}		
}


int stop_has_id(int *id, struct Stop *aStop)
{
	return *id == aStop->id;
}

void stop_init_func(struct Stop *aStop)
{
	if(aStop == NULL)
		return;
	aStop->onRoad = NULL;
	duallist_init(&aStop->routes);
	
}

void stop_free_func(struct Stop *aStop)
{
	if(aStop == NULL)
		return;
	duallist_destroy(&aStop->routes, NULL);
	free(aStop);
}


void dump_stop(FILE *fstop, struct Stop *aStop)
{
	unsigned long i;
	struct Item *aItem;
	struct Busroute *aRoute;

	fprintf(fstop, "%d\n", aStop->id);
	fprintf(fstop, "%.5lf, %.5lf\n", aStop->gPoint.x, aStop->gPoint.y);
	fprintf(fstop, "%d\n", aStop->isDest);
	fprintf(fstop, "%d\n", aStop->onRoad->id);

	fprintf(fstop, "%lu\n", aStop->routes.nItems);
	aItem = aStop->routes.head;
	for(i=0;i<aStop->routes.nItems;i++) {
		aRoute = (struct Busroute*)aItem->datap;
		fprintf(fstop, "%s\n", aRoute->name);
		aItem = aItem->next;
	}
}

void dump_stop_with_hashtable(FILE *fstop, struct Hashtable *stops)
{
	unsigned long i;
	struct Item *aItem;
	struct Stop *aStop;

	fprintf(fstop, "%lu\n", stops->count);
	for(i = 0; i<stops->size; i++) {
		aItem = stops->head[i];
		while (aItem!=NULL) {
			aStop = (struct Stop*)aItem->datap;
			dump_stop(fstop, aStop);
			aItem = aItem->next;
		}
	}
}


struct Stop* load_stop(FILE *fstop, struct Region *aRegion)
{
	int id;
	char *name;
	unsigned long i, nItems;
	struct Item *aItem;
	struct Stop *newStop;
	struct Road *aRoad;

	if(aRegion == NULL)
		return NULL;
	newStop = (struct Stop*)malloc(sizeof(struct Stop));
	stop_init_func(newStop);
	
	fscanf(fstop, "%d\n", &newStop->id);
	fscanf(fstop, "%lf, %lf\n", &newStop->gPoint.x, &newStop->gPoint.y);
	fscanf(fstop, "%d\n", &newStop->isDest);
	fscanf(fstop, "%d\n", &id);
	if ((aItem = duallist_find(&aRegion->roads, &id, (int(*)(void*,void*))road_has_id))!=NULL) {
		aRoad = (struct Road*)aItem->datap;
		newStop->onRoad = aRoad;
	}

	fscanf(fstop, "%lu\n", &nItems);
	for(i=0;i<nItems;i++) {
		name = (char*)malloc(sizeof(char)*NAME_LENGTH);
		fscanf(fstop, "%s\n", name);
		duallist_add_to_tail(&newStop->routes, name);
	}
	return newStop;
}


void load_stop_with_hashtable(FILE *fstop, struct Region *aRegion, struct Hashtable *stops)
{
	unsigned long i, count;
	struct Stop *aStop;

	if(aRegion == NULL || stops == NULL)
		return ;
	fscanf(fstop, "%lu\n", &count);
	for (i=0;i<count;i++){
		aStop = load_stop(fstop, aRegion);
		if(aStop != NULL) {
			hashtable_add(stops, &aStop->id, aStop);
		}		
	}
}


void setup_routes_and_stops(struct Hashtable *routes, struct Hashtable *stops)
{
	int id;
  	char *name;
	unsigned int i;
	struct Item *aItem, *bItem, *cItem;
	struct Busroute *aRoute;
	struct Stop *aStop;

	if(routes == NULL || stops == NULL)
		return;

	for(i = 0; i<routes->size; i++) {
		aItem = routes->head[i];
		while (aItem!=NULL)
		{
			aRoute = (struct Busroute*)aItem->datap;
			bItem = aRoute->stops.head;
			while(bItem != NULL) {
				id = (int)bItem->datap;
				if((cItem = hashtable_find(stops,&id))!=NULL) 
					bItem->datap = cItem->datap;
				bItem = bItem->next;
			}
			aItem = aItem->next;
		}
	}

	for(i = 0; i<stops->size; i++) {
		aItem = stops->head[i];
		while (aItem!=NULL)
		{
			aStop = (struct Stop*)aItem->datap;
			bItem = aStop->routes.head;
			while(bItem != NULL) {
				name = (char*)bItem->datap;
				if((cItem = hashtable_find(routes,name))!=NULL) {
					free(bItem->datap); 
					bItem->datap = cItem->datap;
				}
				bItem = bItem->next;
			}
		}
	}
}

void setup_cells_with_routes(struct Region *region, struct Hashtable *routes)
{
	struct Item *aItem, *bItem;
	struct Busroute *aRoute;
	struct Cell *aCell;
	struct Duallist *cellList;
	unsigned long i, j;

	if(region && routes->count) {
		for (i = 0; i<routes->size; i++) {
			aItem = routes->head[i];
			while(aItem) {
				aRoute = (struct Busroute*)aItem->datap;
				cellList = get_route_coverage(region, aRoute);
				if(cellList) {
					bItem = cellList->head;
					while(bItem) {
						aCell = (struct Cell*)bItem->datap;
						duallist_add_to_tail(&aCell->routes, aRoute);
						bItem = bItem->next;
					}
					duallist_destroy(cellList, NULL);
				}
				aItem = aItem->next;
			}
		}
		duallist_init(&region->busCoveredCells);
		for(i = 0; i<region->hCells; i++)
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				if(aCell->routes.nItems)
					duallist_add_to_tail(&region->busCoveredCells, aCell);
			}
	}
}


void votetree_init_func(struct Votetree *aVotetree)
{
	if(aVotetree == NULL)
		return;
	aVotetree->root = g_node_new(NULL);
	duallist_init(&aVotetree->votes);
	aVotetree->dstPoint.x = 0;
	aVotetree->dstPoint.y = 0;
	aVotetree->travelTime = 3600*2;
	aVotetree->bestCandPath.path = NULL;
	aVotetree->bestCandPath.count = 0;

}

void votetree_free_func(struct Votetree *aVotetree)
{
	g_node_destroy(aVotetree->root);
	duallist_destroy(&aVotetree->votes, free);
	if(aVotetree->bestCandPath.path)
		path_free_func(aVotetree->bestCandPath.path);
}

int vote_has_road(struct Road* aRoad, struct Vote *aVote)
{
	return aRoad == (struct Road*)aVote->votee;
}

void vote_for_route(struct Trace *aTrace, struct Region *region, struct Votetree *upVotetree, struct Votetree *downVotetree)
{
	struct Item *aItem, *cItem, *tItem, *fromItem, *toItem, *bItem, *prevItem, *postItem;
	struct Path *aPath;
	struct Road *aRoad;
	struct Report *aRep, *bRep, *fromRep, *toRep;
	struct Votetree *cVotetree;
	GNode *cNode, *pNode;
	int NotFound, isUpDest, isDownDest;
	struct Vote *aVote;
	int upDestCount, downDestCount;
	double d;

	//find destination locations
	if(upVotetree->dstPoint.x == 0 || downVotetree->dstPoint.x == 0) {
		upDestCount = 0;
		downDestCount = 0;
		aItem = aTrace->reports.head;
		while(aItem!=NULL) {
			aRep = (struct Report*)aItem->datap;
			if(aRep->state & 0x20) {
				if(!upDestCount || !downDestCount) {
					prevItem = NULL;
					bItem = aItem->prev;
					while(bItem != aTrace->reports.head) {
						bRep = (struct Report*)bItem->datap;
						if((d = distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, bRep->gPoint.x, bRep->gPoint.y))<MAX_DEST_RANGE) {
							bItem = bItem->prev;
						} else {
							prevItem = bItem;
							break;
						}
					}
					
					postItem = NULL;
					bItem = aItem->next;
					while(bItem != NULL) {
						bRep = (struct Report*)bItem->datap;
						if((d = distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, bRep->gPoint.x, bRep->gPoint.y))<MAX_DEST_RANGE) {
							bItem = bItem->next;
						} else {
							postItem = bItem;
							break;
						}
					}

					isUpDest = 0;
					if((prevItem == NULL && postItem && (((struct Report*)postItem->datap)->state&0x10))
					 ||(prevItem && postItem == NULL && (((struct Report*)prevItem->datap)->state&0x10) == 0)
					 ||(prevItem && postItem && (((struct Report*)postItem->datap)->state&0x10) && (((struct Report*)prevItem->datap)->state&0x10) == 0))
						isUpDest = 1;
					isDownDest = 0;
					if((prevItem == NULL && postItem && (((struct Report*)postItem->datap)->state&0x10) == 0)
					 ||(prevItem && postItem == NULL && (((struct Report*)prevItem->datap)->state&0x10))
					 ||(prevItem && postItem && (((struct Report*)postItem->datap)->state&0x10)==0 && (((struct Report*)prevItem->datap)->state&0x10)))
						isDownDest = 1;

					if(isUpDest && upDestCount == 0) {
						upDestCount = 1;
						upVotetree->dstPoint.x = aRep->gPoint.x;	
						upVotetree->dstPoint.y = aRep->gPoint.y;
					} else if (isDownDest && downDestCount == 0) {
						downDestCount = 1;
						downVotetree->dstPoint.x = aRep->gPoint.x;	
						downVotetree->dstPoint.y = aRep->gPoint.y;
					}
				}

				if(upDestCount && (d = distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, upVotetree->dstPoint.x/upDestCount, upVotetree->dstPoint.y/upDestCount))<MAX_DEST_RANGE) {
					upDestCount ++;
					upVotetree->dstPoint.x += aRep->gPoint.x;	
					upVotetree->dstPoint.y += aRep->gPoint.y;
				}else if(downDestCount && (d = distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, downVotetree->dstPoint.x/downDestCount, downVotetree->dstPoint.y/downDestCount))<MAX_DEST_RANGE) {
					downDestCount ++;
					downVotetree->dstPoint.x += aRep->gPoint.x;	
					downVotetree->dstPoint.y += aRep->gPoint.y;
				}
			}
			aItem = aItem->next;
		}

		if(upDestCount && downDestCount) {
			upVotetree->dstPoint.x /= upDestCount;
			upVotetree->dstPoint.y /= upDestCount;
			downVotetree->dstPoint.x /= downDestCount;
			downVotetree->dstPoint.y /= downDestCount;

			upVotetree->srcPoint.x = downVotetree->dstPoint.x;
			upVotetree->srcPoint.y = downVotetree->dstPoint.y;
			downVotetree->srcPoint.x = upVotetree->dstPoint.x;
			downVotetree->srcPoint.y = upVotetree->dstPoint.y;
		} else {
			printf("No two destinations found in trace %s, may need manually add destinations.\n", aTrace->vName);
			return;
		}

	}

	if(upVotetree->dstPoint.x && downVotetree->dstPoint.x) {
		aItem = aTrace->reports.head;
		while(aItem != NULL) {
			fromItem = NULL;
			toItem = NULL;
			while(aItem!=NULL) {
				aRep = (struct Report*)aItem->datap;
				if(aRep->state & 0x20) {
					fromItem = aItem;
					fromRep = (struct Report*)fromItem->datap;
					break;
				}
				aItem = aItem->next;
			}
			if(aItem) {
				aItem = aItem->next;
				while(aItem!=NULL) {
					aRep = (struct Report*)aItem->datap;
					if(aRep->state & 0x20) {
						toItem = aItem;
						toRep = (struct Report*)toItem->datap;
						break;
					}
					aItem = aItem->next;
				}
			}
			// check out which way
			cVotetree = NULL;
			if(fromItem && toItem
			&& distance_in_meter(fromRep->gPoint.x, fromRep->gPoint.y, downVotetree->srcPoint.x, downVotetree->srcPoint.y)<MAX_DEST_RANGE 
			&& distance_in_meter(toRep->gPoint.x, toRep->gPoint.y, downVotetree->dstPoint.x, downVotetree->dstPoint.y)<MAX_DEST_RANGE) {
				if(downVotetree->travelTime > toRep->timestamp - fromRep->timestamp) {
					downVotetree->travelTime = toRep->timestamp - fromRep->timestamp;
					cVotetree = downVotetree;
				} else if (downVotetree->travelTime * 2 > toRep->timestamp - fromRep->timestamp) {
					cVotetree = downVotetree;
				} 
			} else if( fromItem && toItem
				&& distance_in_meter(fromRep->gPoint.x, fromRep->gPoint.y, upVotetree->srcPoint.x, upVotetree->srcPoint.y)<MAX_DEST_RANGE 
				&& distance_in_meter(toRep->gPoint.x, toRep->gPoint.y, upVotetree->dstPoint.x, upVotetree->dstPoint.y)<MAX_DEST_RANGE) {
				if(upVotetree->travelTime > toRep->timestamp - fromRep->timestamp) {
					upVotetree->travelTime = toRep->timestamp - fromRep->timestamp;
					cVotetree = upVotetree;
				} else if (upVotetree->travelTime * 2 > toRep->timestamp - fromRep->timestamp) {
					cVotetree = upVotetree;
				} 
			}

			//build up votetrees pair by pair of reports
			if(cVotetree) {
				cNode = cVotetree->root;
				bItem = fromItem;
				while(bItem!=toItem) {
					aRep = (struct Report*)bItem->datap;
					bRep = (struct Report*)bItem->next->datap;
					aPath = get_path_between_two_reports(region, aRep, bRep, MAX_PERCEPTION_DISTANCE);
					if(aPath) {
						if(cNode == cVotetree->root)
							cItem = aPath->roads.head;
						else
							cItem = aPath->roads.head->next;
						while(cItem) {
							NotFound = 1;
							aRoad = (struct Road*)cItem->datap;
							pNode = cNode->children;
							while(pNode) {
								aVote = (struct Vote*)pNode->data;
								if((struct Road*)aVote->votee == aRoad) {
									NotFound = 0;
									if((cNode==cVotetree->root && cItem == aPath->roads.head)
									 ||(cNode!=cVotetree->root && cItem == aPath->roads.head->prev))
										aVote->count ++;
									cNode = pNode;
									break;
								}
								pNode = pNode->next;
							}
							if(NotFound) {
								if ((tItem = duallist_find(&cVotetree->votes, aRoad, (int(*)(void*,void*))vote_has_road))!=NULL) {
									aVote = (struct Vote*)tItem->datap;
								} else {
									aVote = (struct Vote*)malloc(sizeof(struct Vote));
									aVote->votee = aRoad;
									aVote->count = 0;
									duallist_add_to_tail(&cVotetree->votes, aVote);
								}
								pNode = g_node_insert_data(cNode, -1, aVote);
								if((cNode==cVotetree->root && cItem == aPath->roads.head)
								 ||(cNode!=cVotetree->root && cItem == aPath->roads.head->prev))
									aVote->count ++;
								cNode = pNode;
							}
							cItem = cItem->next;
						}
						path_free_func(aPath);
					}
					bItem = bItem->next;
				}
			}
		}

	}
}



void determine_route_path(struct Votetree *aVotetree)
{
	g_node_traverse(aVotetree->root, G_PRE_ORDER, G_TRAVERSE_LEAVES, -1, (GNodeTraverseFunc)get_best_path_on_votetree, aVotetree);
}

gboolean get_best_path_on_votetree(GNode *aNode, struct Votetree *aVotetree)
{
	struct Vote *aVote;
	struct Road *aRoad, *bRoad;
	struct CandidatePath aCandPath;


	aCandPath.path = (struct Path*)malloc(sizeof(struct Path));
	path_init_func(aCandPath.path);
	aCandPath.count = 0;

	while(aNode->parent) {
		aVote = (struct Vote*)aNode->data;
		aRoad = (struct Road*)aVote->votee;
		if(aNode->children==NULL && distance_point_to_polyline(&aVotetree->dstPoint, &aRoad->points, NULL, NULL)>MAX_DEST_RANGE) {
			aCandPath.count = 0;
			break;
		}
		if(aNode->parent->parent) 
			bRoad = (struct Road*)((struct Vote*)aNode->parent->data)->votee;
		else
			bRoad = NULL;
		if((bRoad==NULL && distance_point_to_polyline(&aVotetree->srcPoint, &aRoad->points, NULL, NULL)<MAX_DEST_RANGE) || (bRoad && bRoad->tailEnd == aRoad->headEnd)) {
			duallist_add_to_head(&aCandPath.path->roads, aVote->votee);
			aCandPath.count += aVote->count;
			aNode = aNode->parent;
		} else {
			aCandPath.count = 0;
			break;
		}
	}

	if(aCandPath.count && (aVotetree->bestCandPath.count == 0 || aCandPath.count*1.0/aCandPath.path->roads.nItems > aVotetree->bestCandPath.count*1.0/aVotetree->bestCandPath.path->roads.nItems)) {
		path_free_func(aVotetree->bestCandPath.path);
		aVotetree->bestCandPath.path = aCandPath.path;
		aVotetree->bestCandPath.count = aCandPath.count;
	} else {
		path_free_func(aCandPath.path);
	}

	return FALSE;
}

// return a duallist containing cells covered by a busroute
struct Duallist* get_route_coverage(struct Region *region, struct Busroute *aRoute)
{
	struct Item *aItem, *bItem;
	struct Duallist *rtList;
	struct Road *aRoad;
	struct Point *aPoint, *bPoint;

	if(region && aRoute) {
		rtList = (struct Duallist*)malloc(sizeof(struct Duallist));
		duallist_init(rtList);
		aItem = aRoute->path->roads.head;
		while(aItem) {
			aRoad = (struct Road*)aItem->datap;
			bItem = aRoad->points.head;
			while(bItem && bItem->next) {
				aPoint = (struct Point*)bItem->datap;
				bPoint = (struct Point*)bItem->next->datap;
				cells_on_line(region, aPoint, bPoint, rtList);
				bItem = bItem->next;
			}
			aItem = aItem->next;
		}
		return rtList;
	}
	return NULL;
}

