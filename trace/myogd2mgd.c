#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include"trace.h"
#include"busroute.h"
#include"common.h"
#include"files.h"
#include"geometry.h"
#include"ogd2mgd.h"
#include<assert.h>

//#define DEBUG

#ifdef DEBUG
void dump_candchoice_queue_to_file(char * dumpfile, struct Duallist *duallist);
#endif

int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	struct Region *region, *aRegion;
	char dumpfile[256], *directory, buf[32];

	struct Hashtable traces, routes;
	struct Item *aItem, *tItem, *bItem;
	struct Trace *aTrace, *bTrace, *dTrace;
	struct Busroute *aRoute;
	struct Duallist roads;

	double wdist=WEIGHT_DISTANCE_ERROR,  wangle = WEIGHT_ANGLE_ERROR, wlength = WEIGHT_LENGTH, wturns = WEIGHT_TURNS;

	unsigned long i;
	int defact = 0;
	char *defactd;


	if(argc < 2) {
	      printf("Usage: %s [-d directory] [-o defact_dir] [-w wdist wangle wlength wturns] .map (.ogd|.lst ...)\n", argv[0]);
	      exit(1);
	}

	directory = ".";
	defactd = "./defect";
	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'o':
		      defact = 1;
		      defactd = argv[2];
		      argc-=2;
		      argv+=2;
		      break;

	      case 'd':
		      directory = argv[2];
		      argc-=2;
		      argv+=2;
		      break;
	      case 'w':
		      wdist = atof(argv[2]);
		      wangle = atof(argv[3]);
		      wlength = atof(argv[4]);
		      wturns = atof(argv[5]);
		      argc -= 5;
		      argv += 5;
		      break;
	      default:
	      	      printf("Usage: %s [-d directory] [-o defact_dir] [-w wdist wangle wlength wturns] .map (.ogd|.lst ...)\n", argv[0]);
	      }
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, -1, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}

	hashtable_init(&traces, 2000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
	hashtable_init(&routes, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);
	while(region && argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			printf("Loading %s file ...\n", argv[1]);
			load_source_file(fsource, region, &traces, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, &routes, (void*(*)(FILE*, struct Region*, void *))load_route_with_hashtable, NULL, NULL, NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}


	if(traces.count != 0) {
	      for (i = 0; i<traces.size; i++) {
		      aItem = traces.head[i];
		      while(aItem != NULL ) {
				aTrace = (struct Trace*)aItem->datap;
				aRegion = NULL;
				bTrace = NULL;
				dTrace = NULL;

				if((aTrace->type == FILE_ORIGINAL_GPS_BUS|| aTrace->type == FILE_MODIFIED_GPS_BUS) && routes.count) {
					duallist_init(&roads);
					sprintf(buf, "%s_upway", aTrace->onRoute);
  					tItem = hashtable_find(&routes, buf);
					if(tItem) {
						aRoute = (struct Busroute*)tItem->datap;
						bItem = aRoute->path->roads.head;
						while(bItem) {
							duallist_add_to_tail(&roads, bItem->datap);
							bItem = bItem->next;
						}	
					}
					sprintf(buf, "%s_downway", aTrace->onRoute);
  					tItem = hashtable_find(&routes, buf);
					if(tItem) {
						aRoute = (struct Busroute*)tItem->datap;
						bItem = aRoute->path->roads.head;
						while(bItem) {
							duallist_add_to_tail(&roads, bItem->datap);
							bItem = bItem->next;
						}	
					}

					aRegion = build_region_with_roads(&roads);
					if(aRegion) {
		 				amend_trace(aRegion, aTrace, wdist, wangle, wlength, wturns, &bTrace, &dTrace);
						region_free_func(aRegion);
					}
					duallist_destroy(&roads, NULL);

				} else { 
		 			amend_trace(region, aTrace, wdist, wangle, wlength, wturns, &bTrace, &dTrace); // entrance of the mapmatching code
				}

#ifdef DEBUG
                sprintf(dumpfile, "transVecDump_%s.txt",aTrace->vName);
				if( (fdump = fopen(dumpfile, "w"))!=NULL) {
					trace_dump_transVecHistory(fdump, aTrace);
					fflush(fdump);
					fclose(fdump);
				}
				
#endif

				if(bTrace) {
					mkdir(directory,0777);
					sprintf(dumpfile, "%s/%s.mgd", directory, aTrace->vName);
					if( (fdump = fopen(dumpfile, "w"))!=NULL) {
						trace_dump_func(fdump, bTrace);
						fflush(fdump);
						fclose(fdump);
					}
					trace_free_func(bTrace);
				}

				if(dTrace) {
					if(defact && dTrace->reports.nItems) {
						mkdir(defactd,0777);
						sprintf(dumpfile, "%s/%s.ogd", defactd, aTrace->vName);
						if( (fdump = fopen(dumpfile, "w"))!=NULL) {
							trace_dump_func(fdump, dTrace);
							fflush(fdump);
							fclose(fdump);
						}
					}
					trace_free_func(dTrace);
				}
				trace_free_func((struct Trace*)hashtable_pick(&traces, aTrace->vName));

			      	aItem = aItem->next;
		      }
	      }
	} 
	 
	hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
	hashtable_destroy(&routes, (void(*)(void*))route_free_func);
	region_free_func(region);
	return 0;
}



void amend_trace(struct Region *aRegion, struct Trace *aTrace, double wdist, double wangle, double wlength, double wturns, struct Trace **rtTrace, struct Trace **dTrace)
{
	struct Item *aItem, *nextItem, *bItem; 
	struct Report *aRep, *nextRep, *newRep;
	struct CandRoad *newCandRoad;
	struct Duallist surCells;
	struct Cell *aCell;
	struct Point* modifiedPoint;
	struct Point* pTransVec;

	if (aTrace == NULL) {
		*rtTrace = NULL;
		*dTrace = NULL;
		return;
	}

	printf("Amending Trace: %s ...\n", aTrace->vName);

	*rtTrace = (struct Trace*) malloc(sizeof(struct Trace));
	sprintf((*rtTrace)->vName, "%sa", aTrace->vName);
	strncpy((*rtTrace)->onRoute, aTrace->onRoute, strlen(aTrace->onRoute)+1);
	(*rtTrace)->color.integer = aTrace->color.integer + 20;
	if(aTrace->type == FILE_ORIGINAL_GPS_TAXI)
		(*rtTrace)->type = FILE_MODIFIED_GPS_TAXI;
	else
		(*rtTrace)->type = FILE_MODIFIED_GPS_BUS;
	duallist_init(&(*rtTrace)->reports);

	*dTrace = (struct Trace*) malloc(sizeof(struct Trace));
	sprintf((*dTrace)->vName, "%sd", aTrace->vName);
	strncpy((*dTrace)->onRoute, aTrace->onRoute, strlen(aTrace->onRoute)+1);
	(*dTrace)->color.integer = aTrace->color.integer + 20;
	(*dTrace)->type = aTrace->type;
	duallist_init(&(*dTrace)->reports);



// my algorithm
// 0. initialize trace-v to be zero.
// 1. if there still report in the trace, translate the p_ogd to p_trans by vector trace-v, which is the overall translating error stored in the trace
// 2. caluculate the translation vector v_i from the p_trans to the perpendicular point to the nearest road
// 3. update the trace->v, by adding v_i to it.
// 4. let the perpendicular point to be the p_mgd
// 5. Iterate to the next report, go to step 1. 

	aItem = aTrace->reports.head;

	// 0. initialize trace-v to be zero.
	set_trace_transVec(aTrace, (double)0, (double)0);
	while(aItem!=NULL) {
		aTrace->at = aItem;

		aRep = (struct Report*)aItem->datap;
		if(aRep->candRoads.head == NULL) {
			duallist_init(&surCells);
			surroundings_from_point(&surCells, aRegion, &aRep->gPoint);
			bItem = surCells.head;
			while(bItem != NULL) {
				aCell = (struct Cell*)bItem->datap;
				if(aCell != NULL) {
					//add_candidate_roads(aCell, aRep, aTrace->isHeadingValid);
					modifiedPoint = (struct Point*)malloc(sizeof(struct Point));
					pTransVec = get_trace_transVec(aTrace);
					assert(pTransVec != NULL);
					modifiedPoint->x = aRep->gPoint.x + pTransVec->x;
					modifiedPoint->y = aRep->gPoint.y + pTransVec->y;
					add_candidate_roads_from_modified_point(aCell, aRep, aTrace->isHeadingValid, modifiedPoint);
					free(modifiedPoint);
				}
				bItem = bItem->next;
			}
			duallist_destroy(&surCells, NULL);
		}
		aRep->onRoad = aRep->candRoads.head;

/*		nextItem = aItem->next;
		if(nextItem != NULL) {
			nextRep = (struct Report*)nextItem->datap;
			duallist_init(&surCells);
			surroundings_from_point(&surCells, aRegion, &nextRep->gPoint);
			bItem = surCells.head;
			while(bItem != NULL) {
				aCell = (struct Cell*)bItem->datap;
				if(aCell != NULL) {
					//add_candidate_roads(aCell, nextRep, aTrace->isHeadingValid);
					modifiedPoint = (struct Point*)malloc(sizeof(struct Point));
					pTransVec = get_trace_transVec(aTrace);
					assert(pTransVec!=NULL);
					modifiedPoint->x = nextRep->gPoint.x + pTransVec->x;
					modifiedPoint->y = nextRep->gPoint.y + pTransVec->y;
					add_candidate_roads_from_modified_point(aCell, nextRep, aTrace->isHeadingValid, modifiedPoint);
					free(modifiedPoint);
				}
				bItem = bItem->next;
			}
			duallist_destroy(&surCells, NULL);
			nextRep->onRoad = nextRep->candRoads.head;
		}
*/		
		amend_report(aRegion, aTrace, wdist, wangle, wlength, wturns);
		if(aRep->onRoad) {
			newRep = (struct Report*)malloc(sizeof(struct Report));
			newRep->fromTrace = *rtTrace;
			newRep->timestamp = aRep->timestamp;
			newRep->speed = aRep->speed;
			newRep->angle = aRep->angle;
			newRep->state = aRep->state;
			newRep->msgType = aRep->msgType;
			newRep->routeLeng = aRep->routeLeng;
			newRep->gasVol = aRep->gasVol;
			newRep->errorInfo = aRep->errorInfo;
			duallist_init(&newRep->candRoads);
			newCandRoad = (struct CandRoad*)malloc(sizeof(struct CandRoad));
			newCandRoad->aRoad = ((struct CandRoad*)aRep->onRoad->datap)->aRoad;
			copy_segment(&newCandRoad->onSegment, &((struct CandRoad*)aRep->onRoad->datap)->onSegment);
			duallist_add_to_head(&newRep->candRoads, newCandRoad);
			newRep->onRoad = newRep->candRoads.head;
			newRep->onRoadId = newCandRoad->aRoad->id;
			newRep->onPath = NULL;
			newRep->gPoint.x = ((struct CandRoad*)aRep->onRoad->datap)->gPoint.x;
			newRep->gPoint.y = ((struct CandRoad*)aRep->onRoad->datap)->gPoint.y;
			duallist_add_to_tail(&(*rtTrace)->reports, newRep);
		} else {
			newRep = (struct Report*)malloc(sizeof(struct Report));
			newRep->fromTrace = aTrace;
			newRep->timestamp = aRep->timestamp;
			newRep->speed = aRep->speed;
			newRep->angle = aRep->angle;
			newRep->state = aRep->state;
			newRep->msgType = aRep->msgType;
			newRep->routeLeng = aRep->routeLeng;
			newRep->gasVol = aRep->gasVol;
			newRep->errorInfo = aRep->errorInfo;
			duallist_init(&newRep->candRoads);
			newRep->onRoad = NULL;
			newRep->onRoadId = -1;
			newRep->onPath = NULL;
			newRep->gPoint.x = aRep->gPoint.x;
			newRep->gPoint.y = aRep->gPoint.y;
			duallist_add_to_tail(&(*dTrace)->reports, newRep);
		}
		
		if(aItem->prev != aTrace->reports.head) 
			duallist_destroy(&((struct Report*)aItem->prev->datap)->candRoads, free);

		aItem = aItem->next;
	}

}

/* amend the report at Trace.at */
void amend_report(struct Region *aRegion, struct Trace *aTrace, double wdist, double wangle, double wlength, double wturns)
{
	struct Report *thisRep, *prevRep, *nextRep;
	struct CandRoad *thisCandRoad, *nextCandRoad, *prevCandRoad=NULL, *aCandRoad;
	struct Path *forepart, *postpart;
	int i , j, maxSpeed;
	struct Duallist choices;
	struct CandChoice *newChoice;
	double prevRep2thisRep, prevAmd2thisAmd ;
	struct Item *aItem;

	struct Point * modifiedPoint, *crossPoint, *ptransVec;
	struct Segment* aSegment;
	

#ifdef DEBUG
	char dumpfile[256];
#endif

	if(aTrace->type == FILE_ORIGINAL_GPS_TAXI || aTrace->type == FILE_MODIFIED_GPS_TAXI)
		maxSpeed = TAXI_MAX_SPEED;
	else if(aTrace->type == FILE_ORIGINAL_GPS_BUS || aTrace->type == FILE_MODIFIED_GPS_BUS)
		maxSpeed = BUS_MAX_SPEED;

	thisRep = (struct Report*)aTrace->at->datap;

	if(aTrace->at == aTrace->reports.head)
		prevRep = NULL;
	else
		prevRep = (struct Report*)aTrace->at->prev->datap;
	if(aTrace->at->next == NULL)
		nextRep = NULL;
	else
		nextRep = (struct Report*)aTrace->at->next->datap;

	duallist_init(&choices);

// 2. caluculate the translation vector from the modifiedPoint to the perpendicular point to the nearest road
// 3. update the trace->transVec, by adding v_i to it.
// 4. let the perpendicular point to be the p_mgd
// 5. Iterate to the next report, go to step 1. 



	// --> my report amend code should be implemented there
	j = 0;
	while(j<TOP_N_CANDIDATES && thisRep->onRoad!=NULL) {
		thisCandRoad = (struct CandRoad*)thisRep->onRoad->datap;

		newChoice = (struct CandChoice*)malloc(sizeof(struct CandChoice));

		modifiedPoint = (struct Point*)malloc(sizeof(struct Point));

		ptransVec = get_trace_transVec(aTrace);
		modifiedPoint->x = thisRep->gPoint.x + ptransVec->x;
		modifiedPoint->y = thisRep->gPoint.y + ptransVec->y;
	
		crossPoint = (struct Point*)malloc(sizeof(struct Point));
		aSegment = (struct Segment*)malloc(sizeof(struct Segment));
		double dist = distance_point_to_polyline(modifiedPoint, &thisCandRoad->aRoad->points, crossPoint, aSegment); 


		free(modifiedPoint);
		free(crossPoint);
		free(aSegment);

//		newChoice->score = dist;
		newChoice->score = thisCandRoad->weight;
		newChoice->onRoad = thisRep->onRoad;

		duallist_add_in_sequence_from_head(&choices, newChoice, (int(*)(void*,void*))candchoice_has_smaller_score_than);
		//duallist_add_in_sequence_from_head(&choices, newChoice, (int(*)(void*,void*))candchoice_has_larger_score_than);

		thisRep->onRoad = thisRep->onRoad->next;
		j++;
	}
	// <--
	
	

	while(choices.head != NULL) {
		aItem = ((struct CandChoice*)choices.head->datap)->onRoad;
		aCandRoad = (struct CandRoad*)aItem->datap;
		
		/* if thisRep is not at the same spot as prevRep, then the amended one shouldn't either */
		if(prevRep != NULL && prevCandRoad != NULL) {
			prevRep2thisRep = distance_in_meter(prevRep->gPoint.x, prevRep->gPoint.y, thisRep->gPoint.x, thisRep->gPoint.y);
			prevAmd2thisAmd = distance_in_meter(prevCandRoad->gPoint.x, prevCandRoad->gPoint.y, aCandRoad->gPoint.x, aCandRoad->gPoint.y);
			if(prevAmd2thisAmd==0 && prevRep2thisRep != 0 ) {
				candchoice_free_func(duallist_pick_head(&choices));
				continue;
			} else
				break;
		} else 
			break;
	}
		
	if(choices.head != NULL){ 
		thisRep->onRoad = ((struct CandChoice*)choices.head->datap)->onRoad;
		aCandRoad = (struct CandRoad*)thisRep->onRoad->datap;
/*		crossPoint = (struct Point*)malloc(sizeof(struct Point));
		aSegment = (struct Segment*)malloc(sizeof(struct Segment));
		distance_point_to_polyline(modifiedPoint, &thisCandRoad->aRoad->points, crossPoint, aSegment); 
		double x = crossPoint->x - thisRep->gPoint.x;
		double y = crossPoint->y - thisRep->gPoint.y;
*/
		double x = aCandRoad->gPoint.x - thisRep->gPoint.x;
		double y = aCandRoad->gPoint.y - thisRep->gPoint.y;
		set_trace_transVec(aTrace, x, y);

		struct Point* testTransVec = get_trace_transVec(aTrace);
		

/*		free(crossPoint);
		free(aSegment);*/
	}	
	else { 
		thisRep->onRoad = NULL;
	}

	
	#ifdef DEBUG
    	mkdir("./dump",0777);	
        sprintf(dumpfile, "%s/%s.txt", "./dump", ctime(&(thisRep->timestamp)));
        dump_candchoice_queue_to_file(dumpfile, &choices);
	#endif
	
	duallist_destroy(&choices, (void(*)(void*))candchoice_free_func);

}

double calculate_choice_weight(double distance, double angle, double length, int turns, double wdist, double wangle, double wlength, double wturns)
{	
	double l, t;

	if(length > MAX_LENGTH)
		l = MAX_LENGTH;
	else 
		l = length;

	if(turns > MAX_TURNS)
		t = MAX_TURNS;
	else
		t = turns;

	return wdist*(1-distance/MAX_DISTANCE_ERROR) + wangle*(1-angle/MAX_ANGLE_ERROR) + wlength*(1-l/MAX_LENGTH) + wturns*(1-t/MAX_TURNS);
}


int candchoice_has_smaller_score_than(struct CandChoice *aCandChoice, struct CandChoice *bCandChoice)
{
	return aCandChoice->score < bCandChoice->score;
} 

int candchoice_has_larger_score_than(struct CandChoice *aCandChoice, struct CandChoice *bCandChoice)
{
	return aCandChoice->score > bCandChoice->score;
} 

void candchoice_free_func(struct CandChoice *aCandChoice)
{
	if(aCandChoice == NULL) return;
	free(aCandChoice);
}

#ifdef DEBUG
void candchoice_dump_func(FILE *fOutput, void* ptr){
	struct CandChoice* pCand = (struct CandChoice*) ((struct Item*)ptr)->datap;
	fprintf(fOutput, "Road:%d, Score:%lf\n",((struct CandRoad*)pCand->onRoad->datap)->aRoad->id,pCand->score);
}

void dump_candchoice_queue_to_file(char * dumpfile, struct Duallist *duallist){
	FILE * fdump;
	if( (fdump = fopen(dumpfile, "w"))!=NULL) {
		duallist_dump(fdump, duallist, candchoice_dump_func);
		fflush(fdump);
		fclose(fdump);
	}
}
#endif
