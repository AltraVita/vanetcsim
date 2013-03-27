#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<glib.h>
#include"traj.h"
#include"files.h"

#define MAX_SPEED_MPS 25

void traj_init_func(struct Trajectory *aTraj, char *name)
{
	if(aTraj == NULL)
		return;
	strncpy(aTraj->vName, name, NAME_LENGTH);
	duallist_init(&aTraj->landmarks);
	aTraj->trustworth = TRAJ_TRUSTWORTHY;
	aTraj->nParts = 0;
}

int traj_has_name(char *name, struct Trajectory* aTraj)
{
	return !strcmp(name, aTraj->vName);
}

void traj_free_func(struct Trajectory *aTraj)
{
	if(aTraj == NULL)
		return;
	duallist_destroy(&aTraj->landmarks, free);
	free(aTraj);
}

void traj_dump_func(FILE *fp, struct Trajectory *aTraj)
{
	struct Item *aItem;
	struct Landmark *aLandmark;

	fprintf(fp, "%d\n", FILE_TRAJECTORY);
	aItem = aTraj->landmarks.head;
	while(aItem!=NULL) {
		aLandmark = (struct Landmark*)aItem->datap;
		fprintf(fp, "%s,%d,%ld,%f,%f\n", aTraj->vName, aLandmark->crossId, aLandmark->timestamp, aLandmark->gPoint.x, aLandmark->gPoint.y);
		aItem = aItem->next;
	}
}

struct Trajectory* load_traj_with_duallist(FILE *ftraj, struct Duallist *trajs)
{
	struct Trajectory *aTraj;
	struct Landmark *aLandmark;
	char buf[128], *vName, *strp;
	struct Item *aItem;

	if(trajs == NULL)
		return NULL;
	while(fgets(buf, 128, ftraj)) {
		aLandmark = (struct Landmark*)malloc(sizeof(struct Landmark));

		vName = strtok(buf, ",");
		strp = strtok(NULL, ",");
		aLandmark->crossId = atoi(strp);
		strp = strtok(NULL, ",");
		aLandmark->timestamp = atol(strp);
		strp = strtok(NULL, ",");
		aLandmark->gPoint.x = atof(strp);
		strp = strtok(NULL, ",");
		aLandmark->gPoint.y = atof(strp);

		aItem = duallist_find(trajs, vName, (int(*)(void*,void*))traj_has_name);
		if(aItem == NULL) {
			aTraj = (struct Trajectory*)malloc(sizeof(struct Trajectory));
			traj_init_func(aTraj, vName);
			
			duallist_add_to_tail(trajs, aTraj);
		} else {
			aTraj = (struct Trajectory*)aItem->datap;
		}
		duallist_add_to_tail(&aTraj->landmarks, aLandmark);
	}

	return aTraj;
}


struct Trajectory* load_traj_with_hashtable(FILE *ftraj, struct Hashtable *trajs)
{
	struct Trajectory *aTraj;
	struct Landmark *aLandmark;
	char buf[128], *vName, *strp;
	struct Item *aItem;

	if(trajs == NULL)
		return NULL;
	while(fgets(buf, 128, ftraj)) {
		aLandmark = (struct Landmark*)malloc(sizeof(struct Landmark));

		vName = strtok(buf, ",");
		strp = strtok(NULL, ",");
		aLandmark->crossId = atoi(strp);
		strp = strtok(NULL, ",");
		aLandmark->timestamp = atol(strp);
		strp = strtok(NULL, ",");
		aLandmark->gPoint.x = atof(strp);
		strp = strtok(NULL, ",");
		aLandmark->gPoint.y = atof(strp);

		aItem = hashtable_find(trajs, vName);
		if(aItem == NULL) {
			aTraj = (struct Trajectory*)malloc(sizeof(struct Trajectory));
			traj_init_func(aTraj, vName);
			
			hashtable_add(trajs, vName, aTraj);
		} else {
			aTraj = (struct Trajectory*)aItem->datap;
		}
		duallist_add_to_tail(&aTraj->landmarks, aLandmark);
	}

	return aTraj;
}


int log_trajectory(struct Trace *aTrace, struct Region *aRegion, struct Region *rsuRegion)
{
	struct Item *aItem, *nextItem, *bItem, *crossItem;
	struct Report *aRep, *nextRep;
	struct Path *aPath;
	struct Road *aRoad;
	struct Point *aPoint;
	double avgSpeed, dist;
	FILE *fdump=NULL;
	char dumpfile[256];
	int i, no=0, entry;
	time_t t, tt;

	aItem = aTrace->reports.head;
	while(aItem!=NULL) {
		entry = 1;
		while(aItem!=NULL && (((struct Report*)aItem->datap)->onRoad == NULL || !is_point_in_box(&((struct Report*)aItem->datap)->gPoint, &rsuRegion->chosen_polygon->box))) {
			aItem = aItem->next;
		}
		if(aItem!=NULL) {
			nextItem = aItem->next;
			while(nextItem != NULL) {
				while(nextItem!=NULL && is_point_in_box(&((struct Report*)nextItem->datap)->gPoint, &rsuRegion->chosen_polygon->box) && ((struct Report*)nextItem->datap)->onRoad == NULL) {
					nextItem = nextItem->next;
				}

				if(nextItem!=NULL && !is_point_in_box(&((struct Report*)nextItem->datap)->gPoint, &rsuRegion->chosen_polygon->box)) 
					break;

				if(nextItem != NULL) {
					aRep = (struct Report*)aItem->datap;
					nextRep = (struct Report*)nextItem->datap;

					/* Notice: the aRegion here may not be the same region that rsuRegion is based on, 
 						   in which case the Id of crosses are not identical */
					aPath = get_path_between_two_reports(aRegion, aRep, nextRep, -1);
					if(aPath!=NULL && aPath->roads.nItems >= 2 && is_path_within_box(aPath, &rsuRegion->chosen_polygon->box)) {
						aPath->length = distance_on_path(aPath, &aRep->gPoint, &nextRep->gPoint);
						if(nextRep->timestamp != aRep->timestamp)
							avgSpeed = aPath->length/(nextRep->timestamp - aRep->timestamp);
						else
							avgSpeed = -1;
						if(avgSpeed != -1 && avgSpeed < MAX_SPEED_MPS) {
							aRoad = (struct Road*)aPath->roads.head->datap;
							aPoint = &((struct CandRoad*)aRep->onRoad->datap)->gPoint;
							dist = distance_to_tail_cross(aRoad, aPoint);
							if(avgSpeed) {
								t = aRep->timestamp+dist/avgSpeed;
								tt = ceil(t);
							} else {
								t = aRep->timestamp + (nextRep->timestamp-aRep->timestamp)/2;
								tt = ceil(t);	
							}
							/* Because of the above reason, we use absolute location to identify the right cross,
							 then record the Id of this cross in rsuRegion */
							crossItem = duallist_find(&rsuRegion->crosses, &aRoad->tailEnd->gPoint, (int(*)(void*,void*))cross_has_location);
							if(crossItem != NULL) {
								if(entry) {
									entry = 0;
									no ++;
									if(fdump!=NULL) {
										fflush(fdump);
										fclose(fdump);
									}
									sprintf(dumpfile, "%s_%d.trj", aTrace->vName, no);
									if((fdump = fopen(dumpfile, "w"))==NULL) {
										printf("Cannot open file %s to write!\n", dumpfile);
										exit (1);
									}									;
									fprintf(fdump, "%d\n", FILE_TRAJECTORY);
								}

								fprintf(fdump, "%s_%d,%d,%ld,%f,%f\n", aTrace->vName, no, ((struct Cross*)crossItem->datap)->number, tt, aRoad->tailEnd->gPoint.x, aRoad->tailEnd->gPoint.y);
							}
							bItem = aPath->roads.head->next;
							for(i=0;i<aPath->roads.nItems-2;i++) {
								aRoad = (struct Road*)bItem->datap;
								t += aRoad->length/avgSpeed;
								tt = ceil(t);
								crossItem = duallist_find(&rsuRegion->crosses, &aRoad->tailEnd->gPoint, (int(*)(void*,void*))cross_has_location);
								if(crossItem != NULL) {
									if(entry) {
										entry = 0;
										no ++;
										if(fdump!=NULL) {
											fflush(fdump);
											fclose(fdump);
										}
										sprintf(dumpfile, "%s_%d.trj", aTrace->vName, no);
										if((fdump = fopen(dumpfile, "w"))==NULL) {
											printf("Cannot open file %s to write!\n", dumpfile);
											exit (1);
										}									;
										fprintf(fdump, "%d\n", FILE_TRAJECTORY);
									}
									fprintf(fdump, "%s_%d,%d,%ld,%f,%f\n", aTrace->vName, no, ((struct Cross*)crossItem->datap)->number, tt, aRoad->tailEnd->gPoint.x, aRoad->tailEnd->gPoint.y);
								}
								bItem = bItem->next;
							}
						}
					}
					if(aPath)	
						path_free_func(aPath);
				}
				aItem = nextItem;
				if(aItem != NULL)
					nextItem = aItem->next;
			}
		}
		if(aItem != NULL)
			aItem = aItem->next;
	}
	if(fdump!=NULL) {
		fflush(fdump);
		fclose(fdump);
	}
	return 0;
}


/* a straight forward method to generate given number k-length spoofed trajectories using stack */
void spoof_a_traj_using_stack(struct Duallist *spooftrajs, struct Trajectory *aTraj, struct Region *rsuRegion, int k, int number)
{
	int  i;
	struct Item *aItem, *aLandmarkItem, *bLandmarkItem;
	struct Duallist *aTempSpooftraj, *tempSpooftrajs, *newTempSpooftraj;

	if(spooftrajs == NULL) {
		spooftrajs = (struct Duallist*)malloc(sizeof(struct Duallist));
		duallist_init(spooftrajs);
	}

	if(rsuRegion==NULL || aTraj == NULL || (aTraj!=NULL&&aTraj->landmarks.nItems<k) ) {
		return;
	}

	tempSpooftrajs = (struct Duallist*)malloc(sizeof(struct Duallist));
	stack_init(tempSpooftrajs);
	
	aItem = aTraj->landmarks.head;
	for(i=0; (k==-1&&i<aTraj->landmarks.nItems) || (k>0 && i<aTraj->landmarks.nItems+1-k); i++) {
		newTempSpooftraj = (struct Duallist*)malloc(sizeof(struct Duallist));
		duallist_init(newTempSpooftraj);
		duallist_add_to_tail(newTempSpooftraj, aItem);
		stack_push(tempSpooftrajs, newTempSpooftraj);
		aItem = aItem->next;
	}

	while(tempSpooftrajs->head != NULL && (number == -1 || (number != -1 && number>0)) ) {
		aTempSpooftraj = (struct Duallist*)stack_pop(tempSpooftrajs);
		aItem = aTempSpooftraj->head->prev;
		aLandmarkItem = (struct Item*)aItem->datap;
		bLandmarkItem = aLandmarkItem->next;
		while(bLandmarkItem!=NULL) {
			if(are_two_landmarks_neighbors(rsuRegion, aLandmarkItem->datap, bLandmarkItem->datap) 
			   && (k == -1 || ( k>0 && distance_to_tail(bLandmarkItem) >= k-aTempSpooftraj->nItems && aTempSpooftraj->nItems < k))) {
				newTempSpooftraj = duallist_copy_by_reference(NULL, aTempSpooftraj);
				duallist_add_to_tail(newTempSpooftraj, bLandmarkItem);
				stack_push(tempSpooftrajs, newTempSpooftraj);
			}
			bLandmarkItem = bLandmarkItem->next;
		}

		if(k==-1 || (k>0 && aTempSpooftraj->nItems == k)) {
			if(number>0)
				number --;
			duallist_add_to_tail(spooftrajs, aTempSpooftraj);
		} else
			duallist_destroy(aTempSpooftraj, NULL);
		
	}
}

			
int are_two_landmarks_neighbors(struct Region *rsuRegion, struct Landmark *aLandmark, struct Landmark *bLandmark)
{
	struct Item *aItem, *bItem, *cItem;
	struct Cell *aCell;
	struct Cross *aCross;
	struct Road *aRoad;
	struct Duallist surCells;

	duallist_init(&surCells);
	surroundings_from_point(&surCells, rsuRegion, &aLandmark->gPoint);
	aItem = surCells.head;
	while(aItem != NULL) {
	      aCell = (struct Cell*)aItem->datap;
	      bItem = aCell->crosses.head;
	      while(bItem != NULL) {
			aCross = (struct Cross*)bItem->datap;
			if(aCross->number == aLandmark->crossId) {
				cItem = aCross->outRoads.head;
				while(cItem != NULL) {
					aRoad = (struct Road*)cItem->datap;
					if(aRoad->tailEnd->number == bLandmark->crossId) 
						return 1;
					cItem = cItem->next;
				}
				cItem = aCross->inRoads.head;
				while(cItem != NULL) {
					aRoad = (struct Road*)cItem->datap;
					if(aRoad->headEnd->number == bLandmark->crossId) 
						return 1;
					cItem = cItem->next;
				}
			}
		      	bItem=bItem->next;
	      }
	      aItem = aItem->next;
	}
	return 0;
}

/* return the total time wasting at the same RSUs */
time_t get_parts_in_traj(struct Duallist *parts, struct Trajectory *aTraj)
{
	struct Item *aItem, *bItem;
	struct Landmark *aLandmark, *bLandmark;
	struct Duallist *newPart;
	time_t gooseTime = 0;

	duallist_init(parts);
	aItem = aTraj->landmarks.head;
	while(aItem != NULL)  {
		aLandmark = (struct Landmark*)aItem->datap;
		newPart = (struct Duallist*)malloc(sizeof(struct Duallist));
		duallist_init(newPart);
		duallist_add_to_tail(newPart, aLandmark);

		bItem = aItem->next;
		while(bItem !=NULL) {
			bLandmark = (struct Landmark*)bItem->datap;
			if(bLandmark->crossId != aLandmark->crossId) 
				break;
			else {
				duallist_add_to_tail(newPart, bLandmark);
				gooseTime += bLandmark->timestamp - aLandmark->timestamp;
			}
			bItem = bItem->next;
		}
		duallist_add_to_tail(parts, newPart);
		aItem = bItem;
	}
	aTraj->nParts = parts->nItems;
	return gooseTime;
}


int check_two_parts(struct Duallist *aPart, struct Duallist *bPart, time_t checkwindow)
{
	struct Item *aItem, *bItem;
	struct Landmark *aLandmark, *bLandmark;
	int rst = 0;
	
	aItem = aPart->head;
	while(aItem != NULL) {
		aLandmark = (struct Landmark*)aItem->datap;
		bItem = bPart->head;
		while(bItem!=NULL) {
			bLandmark = (struct Landmark*)bItem->datap;
			if(bLandmark->timestamp > aLandmark->timestamp-checkwindow && bLandmark->timestamp < aLandmark->timestamp+checkwindow) {
				if(aLandmark->crossId != bLandmark->crossId)
					rst = -1;
				else
					rst = 1;
				return rst;
			}
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
	return rst;		
}


double similarity_between_two_trajs(struct Trajectory *aTraj, struct Trajectory *bTraj, time_t checkwindow, time_t T, int K)
{
	struct Item *aItem, *bItem;
	struct Duallist aParts, bParts, *aPart, *bPart;
	int checkrst, restriction;
	double count = 0, rst;
	time_t gooseTime1, gooseTime2;

	gooseTime1 = get_parts_in_traj(&aParts, aTraj);
	gooseTime2 = get_parts_in_traj(&bParts, bTraj);
	
	aItem = aParts.head;
	while(aItem != NULL)  {
		aPart = (struct Duallist*)aItem->datap;
		bItem = bParts.head;
		while(bItem != NULL) {
			bPart = (struct Duallist*)bItem->datap;
			checkrst = check_two_parts(aPart, bPart, checkwindow);
			if (checkrst == -1) {
				curtain_destroy(&aParts, NULL);
				curtain_destroy(&bParts, NULL);
				return -1;
			}
			count += checkrst;
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
	rst = count/MIN(aParts.nItems, bParts.nItems);
	restriction = (1-(gooseTime1+gooseTime2)*1.0/T)*K;
	if(aParts.nItems + bParts.nItems >= restriction && rst == 0)
		rst = -1;
	curtain_destroy(&aParts, NULL);
	curtain_destroy(&bParts, NULL);
	return rst;
}

int C_n_x(int n, int x)
{
	int j;
	double c;

	c = 1;
	for(j=0;j<x;j++)
		c *=((double)(n-j))/(x-j);
	return round(c);
}

int argmax_C_n_x_larger_than_m(int n, int m)
{
	int b, i;

	b = ceil(((double)n)/2);
	for(i=n; i>=b; i--) {
		if( C_n_x(n, i) >= m)
			return i;
	}
	return b;
}

