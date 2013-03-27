#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"contact.h"
#include"trace.h"
#include"files.h"

double vector_entropy(unsigned long *vector, int nItems)
{
	int i;
	struct Item *aSingleItem;
	struct Single *aSingle;
	struct Hashtable singleTable;
	char key[128];
	double pi, rst = 0;

	hashtable_init(&singleTable, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))single_has_name);
	for(i=0;i<nItems;i++) {
	      sprintf(key, "%ld", vector[i]);
	      aSingleItem = hashtable_find(&singleTable, key);
	      if(aSingleItem == NULL) {
		      aSingle = (struct Single*)malloc(sizeof(struct Single));
		      aSingle->t1 = vector[i];
		      aSingle->count = 1;
		      hashtable_add(&singleTable, key, aSingle);
	      } else {
		      aSingle = (struct Single*)aSingleItem->datap;
		      aSingle->count ++;
	      }
        }
	for (i=0;i<singleTable.size;i++) {
	      aSingleItem = singleTable.head[i];
	      while (aSingleItem != NULL) {
			aSingle = (struct Single*)aSingleItem->datap;
			pi = aSingle->count*1.0/nItems;
			rst -= pi*log(pi)/log(2);	
			aSingleItem = aSingleItem->next;
	      }
	}
	hashtable_destroy(&singleTable, free);
	return rst;

}



double vectors_joint_entropy(unsigned long *vector1, unsigned long *vector2, int nItems)
{
	int i;
	struct Item *aCoupleItem;
	struct Couple *aCouple;
	struct Hashtable coupleTable;
	char key[128];
	double pi, rst = 0;

	hashtable_init(&coupleTable, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))couple_has_names);
	for(i=0;i<nItems;i++) {
	      sprintf(key, "%ld,%ld", vector1[i], vector2[i]);
	      aCoupleItem = hashtable_find(&coupleTable, key);
	      if(aCoupleItem == NULL) {
		      aCouple = (struct Couple*)malloc(sizeof(struct Couple));
		      aCouple->t1 = vector1[i];
		      aCouple->t2 = vector2[i];
		      aCouple->count = 1;
		      hashtable_add(&coupleTable, key, aCouple);
	      } else {
		      aCouple = (struct Couple*)aCoupleItem->datap;
		      aCouple->count ++;
	      }
        }
	for (i=0;i<coupleTable.size;i++) {
	      aCoupleItem = coupleTable.head[i];
	      while (aCoupleItem != NULL) {
			aCouple = (struct Couple*)aCoupleItem->datap;
			pi = aCouple->count*1.0/nItems;
			rst -= pi*log(pi)/log(2);	
			aCoupleItem = aCoupleItem->next;
	      }
	}
	hashtable_destroy(&coupleTable, free);
	return rst;
}

int single_has_name(char *names, struct Single *aSingle)
{
	char buf[128];
	sprintf(buf, "%d", aSingle->t1);
	return !strcmp(names, buf);
}

int couple_has_names(char *names, struct Couple *aCouple)
{
	char buf[128];
	sprintf(buf, "%d,%d", aCouple->t1, aCouple->t2);
	return !strcmp(names, buf);
}

int triple_has_names(char *names, struct Triple *aTriple)
{
	char buf[128];
	sprintf(buf, "%d,%d,%d", aTriple->t1, aTriple->t2, aTriple->t3);
	return !strcmp(names, buf);
}
int contact_sample_has_earlier_timestamp_than(struct ContactSample *aContactSample, struct ContactSample *bContactSample)
{
	return aContactSample->timestamp < bContactSample->timestamp;
}


int are_contact_samples_duplicated(struct ContactSample *aContactSample, struct ContactSample *bContactSample)
{
	if(aContactSample!=NULL 
	&& bContactSample!=NULL 
	&& aContactSample->timestamp == bContactSample->timestamp
	&& aContactSample->gPoint1.x == bContactSample->gPoint1.x
	&& aContactSample->gPoint1.y == bContactSample->gPoint1.y
	&& aContactSample->gPoint2.x == bContactSample->gPoint2.x
	&& aContactSample->gPoint2.y == bContactSample->gPoint2.y
	&& aContactSample->distance == bContactSample->distance
	&& aContactSample->rAngle == bContactSample->rAngle
	&& aContactSample->rSpeed == bContactSample->rSpeed)
		return 1;
	else 
		return 0;
}

int are_contact_samples_continuous(struct ContactSample *aContactSample, struct ContactSample *bContactSample, int tGran, int sGran) 
{
	double alpha1, alpha2, dist, xdist;

	alpha1 =angle_between(aContactSample->gPoint1.x, aContactSample->gPoint1.y, aContactSample->gPoint2.x, aContactSample->gPoint2.y); 
	alpha2 = MAX(aContactSample->rAngle, alpha1) - MIN(aContactSample->rAngle, alpha1);
	dist = distance_in_meter(aContactSample->gPoint1.x, aContactSample->gPoint1.y, aContactSample->gPoint2.x, aContactSample->gPoint2.y);
	if(alpha2 == 0) 
		xdist = sGran - dist;
	else if (alpha2 == 180)
		xdist = sGran + dist;
	else if (alpha2 <= 90 || alpha2 >= 270) 
		xdist = sqrt(sGran*sGran - dist*dist*sin(alpha2*M_PI/180)*sin(alpha2*M_PI/180)) + dist*cos(alpha2*M_PI/180);
	else  
		xdist = sqrt(sGran*sGran - dist*dist*sin(alpha2*M_PI/180)*sin(alpha2*M_PI/180)) - dist*cos(alpha2*M_PI/180);

	if ( xdist >= (MAX(bContactSample->timestamp,aContactSample->timestamp) - MIN(bContactSample->timestamp,aContactSample->timestamp)) * aContactSample->rSpeed / 3.6 ) 
		return 1;
	else
		return 0;
}


int is_earlier_than_contact(time_t *timestamp, struct Contact *aContact)
{
	return *timestamp < aContact->startAt;
}


int pair_has_names(char *names, struct Pair *aPair)
{
	char buf[128];
	sprintf(buf, "%s,%s", aPair->vName1, aPair->vName2);
	return !strcmp(names, buf);
}

void pair_init_func(struct Pair *aPair)
{
	if(aPair == NULL) 
		return;
	memset(aPair->vName1, '\0', 2*NAME_LENGTH);
	memset(aPair->vName2, '\0', 2*NAME_LENGTH);
	duallist_init(&aPair->contents);
	aPair->at = NULL;
}


void pair_free_func(struct Pair *aPair)
{
	duallist_destroy(&aPair->contents, free);
	free(aPair);
}

void dump_contact_sample_pair(struct Pair *aPair, FILE *fdump)
{
	struct Item *aItem;
	struct ContactSample *aContactSample;
	char buf[128];

	aItem = aPair->contents.head;
	while(aItem!=NULL) {
		aContactSample = (struct ContactSample*)aItem->datap;
		ttostr(aContactSample->timestamp, buf);
		fprintf(fdump, "%s,%s,%s,%10.6lf,%10.6lf,%10.6lf,%10.6lf,%.2lf,%d,%d\n",
				aPair->vName1,
				aPair->vName2,
				buf,
				aContactSample->gPoint1.x,
				aContactSample->gPoint1.y,
				aContactSample->gPoint2.x,
				aContactSample->gPoint2.y,
				aContactSample->distance,
				aContactSample->rAngle,
				aContactSample->rSpeed);
		aItem = aItem->next;
	}
}

void get_cells_ready_for_trace(struct Region *region, int slotNumber)
{
	int i, j, k;
	struct Cell *aCell;

	if (region == NULL) 
		return;

	for(i = 0; i<region->hCells; i++)
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			aCell->slots = (struct Duallist*)malloc(sizeof(struct Duallist)*slotNumber);
			for ( k = 0; k < slotNumber; k++) 
				duallist_init(aCell->slots+k);
		}
}

void clear_cells_from_trace(struct Region *region, int slotNumber)
{
	int i, j, k;
	struct Cell *aCell;

	if (region == NULL) 
		return;

	for(i = 0; i<region->hCells; i++)
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->slots != NULL) {
				for (k = 0; k < slotNumber; k++) {
					duallist_destroy(aCell->slots+k, NULL);
				}
				free(aCell->slots);	
			}
		}
}

void set_traces_at_clock(struct Hashtable *traces, time_t atClock)
{
	struct Item *aItem, *bItem;
	struct Trace *aTrace;
	unsigned long at;

	for(at = 0; at<traces->size; at++) {
		aItem = traces->head[at];
		while(aItem != NULL) {
			aTrace = (struct Trace*)aItem->datap;
			bItem = aTrace->reports.head;
			while(bItem!=NULL && ((struct Report*)bItem->datap)->timestamp < atClock) 
				bItem = bItem->next;
			aTrace->at = bItem;
			aItem = aItem->next;
		}
	}
}

void mount_traces(struct Region *region, struct Hashtable *traces, struct ContactContext *context)
{
	int i;

	i = 1;
	while(context->clock <= context->endAt && i < context->numSlots) {
		mount_traces_at_slot(region, traces, context, (context->newestPos+1)%context->numSlots);
		context->newestPos = (context->newestPos+1)%context->numSlots;
		i ++;
	}
}

void mount_traces_at_slot(struct Region *region, struct Hashtable *traces, struct ContactContext *context, int position)
{
	int i, j;
	struct Cell *aCell;
	struct Item *aItem, *bItem;
	struct Trace *aTrace;

	for(i = 0; i<region->hCells; i++)
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			duallist_destroy(aCell->slots+position, NULL);
			duallist_init(aCell->slots+position);
		}

	for(i = 0; i<traces->size; i++) {
		aItem = traces->head[i];	
		while(aItem!=NULL) {
			aTrace = (struct Trace*)aItem->datap;
			bItem = aTrace->at;
			while(bItem!=NULL && ((struct Report*)bItem->datap)->timestamp < context->clock + context->tGran) {
				if( is_point_in_box(&((struct Report*)bItem->datap)->gPoint, &region->chosen_polygon->box)) { 
					i = (((struct Report*)bItem->datap)->gPoint.x - region->chosen_polygon->box.xmin)/region->cellSize;
					j = (((struct Report*)bItem->datap)->gPoint.y - region->chosen_polygon->box.ymin)/region->cellSize;
					aCell = region->mesh + i*region->vCells + j;
					duallist_add_to_tail(aCell->slots+position, bItem->datap);
				}
				bItem = bItem->next;
			}
			aTrace->at = bItem;
			aItem = aItem->next;
		}
	}
	context->clock += context->tGran;
}

void free_used_reports(struct Hashtable *traces)
{
	unsigned long i;
	struct Item *aItem, *bItem;
	struct Trace *aTrace;

	for(i = 0; i<traces->size; i++) {
		aItem = traces->head[i];	
		while(aItem!=NULL) {
			aTrace = (struct Trace*)aItem->datap;
			bItem = aTrace->at;
			while(aTrace->reports.head != bItem) {
				report_free_func(duallist_pick_head(&aTrace->reports));
			}
			aItem = aItem->next;
		}
	}
}

void mount_traces_with_seperate_slots(struct Hashtable *traces, struct ContactContext *context, struct Duallist *slots)
{
	int i;

	i = 1;
	while(context->clock <= context->endAt && i < context->numSlots) {
		mount_traces_at_seperate_slot(traces, context, slots, (context->newestPos+1)%context->numSlots);
		context->newestPos = (context->newestPos+1)%context->numSlots;
		i ++;
	}
}

void mount_traces_at_seperate_slot(struct Hashtable *traces, struct ContactContext *context, struct Duallist *slots, int position)
{
	unsigned long i;
	struct Item *aItem, *bItem;
	struct Trace *aTrace;

	duallist_destroy(slots+position, NULL);
	duallist_init(slots+position);

	for(i = 0; i<traces->size; i++) {
		aItem = traces->head[i];	
		while(aItem!=NULL) {
			aTrace = (struct Trace*)aItem->datap;
			bItem = aTrace->at;
			while(bItem!=NULL && ((struct Report*)bItem->datap)->timestamp < context->clock + context->tGran) {
				duallist_add_to_tail(slots+position, bItem->datap);
				bItem = bItem->next;
			}
			aTrace->at = bItem;
			aItem = aItem->next;
		}
	}
	context->clock += context->tGran;
}

void find_contact_samples_with_seperate_slots(struct Hashtable *pairSmpTable, struct ContactContext *context, struct Duallist *slots)
{
	int newPos;
	struct Duallist *thisSlot, *thatSlot;
	time_t atTime;

	newPos = context->currentPos;
	while(newPos != context->newestPos) {
		if(context->newestPos > newPos)
			atTime = context->clock-(context->newestPos+1-newPos)*context->tGran;
		else
			atTime = context->clock-(context->newestPos+context->numSlots+1-newPos)*context->tGran;
		thisSlot = slots+newPos;
		thatSlot = slots+newPos;
		find_contact_samples_between_two_slots(thisSlot, thatSlot, pairSmpTable, atTime, context);
		thatSlot = slots+(newPos+1)%context->numSlots;
		find_contact_samples_between_two_slots(thisSlot, thatSlot, pairSmpTable, atTime, context);
		newPos = (newPos+1)%context->numSlots;
	}
	if(newPos == context->newestPos && context->clock >= context->endAt) {
		atTime = context->clock-context->tGran;
		thisSlot = slots+newPos;
		thatSlot = slots+newPos;
		find_contact_samples_between_two_slots(thisSlot, thatSlot, pairSmpTable, atTime, context);
	}

	context->currentPos = context->newestPos;
}


void find_contact_samples(struct Region *region, struct Hashtable *pairSmpTable, struct ContactContext *context)
{
	int i, j, newPos;
	struct Point aPoint;
	struct Cell *thisCell, *thatCell;
	struct Duallist surCells;
	struct Item *aCellItem;
	struct Duallist *thisSlot, *thatSlot;
	time_t atTime;

	for(i=0;i<region->vCells*region->hCells;i++)
		*(region->map+i) = 0;

	for(i = 0; i<region->hCells; i++)
		for(j = 0; j<region->vCells;j++) {
			thisCell = region->mesh + i*region->vCells + j;
			aPoint.x = (thisCell->box.xmin+thisCell->box.xmax)/2;
			aPoint.y = (thisCell->box.ymin+thisCell->box.ymax)/2;
			duallist_init(&surCells);
			surroundings_from_point(&surCells, region, &aPoint);
			aCellItem = surCells.head;
			while (aCellItem != NULL) {
				thatCell = (struct Cell*)aCellItem->datap;
				if(*(region->map + thatCell->xNumber*region->vCells +thatCell->yNumber) != 1) {
					newPos = context->currentPos;
					while(newPos != context->newestPos) {
						thisSlot = thisCell->slots+newPos;
						thatSlot = thatCell->slots+newPos;
						if(context->newestPos > newPos)
							atTime = context->clock-(context->newestPos+1-newPos)*context->tGran;
						else
							atTime = context->clock-(context->newestPos+context->numSlots+1-newPos)*context->tGran;
						find_contact_samples_between_two_slots(thisSlot, thatSlot, pairSmpTable, atTime, context);
						thatSlot = thatCell->slots+(newPos+1)%context->numSlots;
						find_contact_samples_between_two_slots(thisSlot, thatSlot, pairSmpTable, atTime, context);
						newPos = (newPos+1)%context->numSlots;
					}
					if(newPos == context->newestPos && context->clock >= context->endAt) {
						thisSlot = thisCell->slots+newPos;
						thatSlot = thatCell->slots+newPos;
						atTime = context->clock-context->tGran;
						find_contact_samples_between_two_slots(thisSlot, thatSlot, pairSmpTable, atTime, context);
					}
				}
				aCellItem = aCellItem->next;
			}
			duallist_destroy(&surCells, NULL);
			*(region->map + i*region->vCells + j) = 1;
		}

	context->currentPos = context->newestPos;

}

void find_contact_samples_between_two_slots(struct Duallist *thisSlot, struct Duallist *thatSlot, struct Hashtable *pairSmpTable, time_t clock, struct ContactContext *context)
{
	struct Item *thisItem, *thatItem, *aItem;
	struct Report *thisRep, *thatRep, *smallIdRep, *bigIdRep;
	struct ContactSample *newContactSample;
	struct Pair *aContactSamplePair;
	char key[128], name1[64], name2[64], *smallId, *bigId;
	double alpha1, alpha2, alpha, sinSpeed, cosSpeed;
	int type1, type2;
	int needed;

	
	thisItem = thisSlot->head;
	while(thisItem != NULL) {
		thisRep = (struct Report*)thisItem->datap;
		/* check within a same slot*/
		if(thisSlot == thatSlot)
			thatItem = thisItem->next;
		else
			thatItem = thatSlot->head;

		while(thatItem != NULL) {
			thatRep = (struct Report*)thatItem->datap;
			type1 = thisRep->fromTrace->type;
			if(type1 == FILE_ORIGINAL_GPS_TAXI || type1 == FILE_MODIFIED_GPS_TAXI) {
				type1 = VEHICLE_TYPE_TAXI;
				strncpy(name1, thisRep->fromTrace->vName, NAME_LENGTH);
			} else if (type1 == FILE_ORIGINAL_GPS_BUS || type1 == FILE_MODIFIED_GPS_BUS) {
				type1 = VEHICLE_TYPE_BUS;
				if(is_report_in_upway(thisRep))
					sprintf(name1, "%s@%s_upway", thisRep->fromTrace->vName, thisRep->fromTrace->onRoute);
				else
					sprintf(name1, "%s@%s_downway", thisRep->fromTrace->vName, thisRep->fromTrace->onRoute);
			}

			type2 = thatRep->fromTrace->type;
			if(type2 == FILE_ORIGINAL_GPS_TAXI || type2 == FILE_MODIFIED_GPS_TAXI) {
				type2 = VEHICLE_TYPE_TAXI;
				strncpy(name2, thatRep->fromTrace->vName, NAME_LENGTH);
			} else if (type2 == FILE_ORIGINAL_GPS_BUS || type2 == FILE_MODIFIED_GPS_BUS) {
				type2 = VEHICLE_TYPE_BUS;
				if(is_report_in_upway(thatRep))
					sprintf(name2, "%s@%s_upway", thatRep->fromTrace->vName, thatRep->fromTrace->onRoute);
				else
					sprintf(name2, "%s@%s_downway", thatRep->fromTrace->vName, thatRep->fromTrace->onRoute);
			}

			if(context->type1 == VEHICLE_TYPE_NULL && context->type2 == VEHICLE_TYPE_NULL) {
				needed = 1;
			} else if(context->type1 == VEHICLE_TYPE_NULL && context->type2 != VEHICLE_TYPE_NULL) {
				if(type1 == context->type2 || type2 == context->type2)
					needed = 1; else needed = 0;
			} else if(context->type1 != VEHICLE_TYPE_NULL && context->type2 == VEHICLE_TYPE_NULL) {
				if(type1 == context->type1 || type2 == context->type1)
					needed = 1; else needed = 0;
			} else if(context->type1 != VEHICLE_TYPE_NULL && context->type2 != VEHICLE_TYPE_NULL) {
				if( (type1 == context->type1&&type2 == context->type2) || (type1 == context->type2 && type2 == context->type1))
					needed = 1; else needed = 0;
			}

			if(needed && are_two_reps_meeting(thisRep, thatRep, context->tGran, context->sGran)) {
				if (0 > strcmp(name1, name2)) {
					smallIdRep = thisRep;
					smallId = name1;
					bigIdRep = thatRep;
					bigId = name2;
				} else {
					smallIdRep = thatRep;
					smallId = name2;
					bigIdRep = thisRep;
					bigId = name1;
				}
				sprintf(key, "%s,%s", smallId, bigId);
				aItem = hashtable_find(pairSmpTable, key);
				if(aItem == NULL) {
					aContactSamplePair = (struct Pair*)malloc(sizeof(struct Pair));
					pair_init_func(aContactSamplePair);
					strncpy(aContactSamplePair->vName1, smallId, strlen(smallId)+1);
					strncpy(aContactSamplePair->vName2, bigId, strlen(bigId)+1);
					hashtable_add(pairSmpTable, key, aContactSamplePair);
				} else
					aContactSamplePair = (struct Pair*)aItem->datap;
				
				newContactSample = (struct ContactSample*)malloc(sizeof(struct ContactSample));
				newContactSample->timestamp = clock;
				newContactSample->gPoint1.x = smallIdRep->gPoint.x;
				newContactSample->gPoint1.y = smallIdRep->gPoint.y;
				newContactSample->gPoint2.x = bigIdRep->gPoint.x;
				newContactSample->gPoint2.y = bigIdRep->gPoint.y;
				newContactSample->distance = distance_in_meter(smallIdRep->gPoint.x,smallIdRep->gPoint.y, bigIdRep->gPoint.x, bigIdRep->gPoint.y);
				alpha1 = smallIdRep->angle;
				alpha2 = bigIdRep->angle;
				alpha = inter_angle(alpha1, alpha2);
				if(alpha < 90) {
					sinSpeed = bigIdRep->speed*sin(alpha*M_PI/180);
					cosSpeed = bigIdRep->speed*cos(alpha*M_PI/180)-smallIdRep->speed;
				} else {
					sinSpeed = bigIdRep->speed*sin(alpha*M_PI/180);
					cosSpeed = -bigIdRep->speed*cos(alpha*M_PI/180)-smallIdRep->speed;
				} 

				newContactSample->rSpeed = sqrt( sinSpeed * sinSpeed + cosSpeed * cosSpeed);
				if (newContactSample->rSpeed != 0)
					newContactSample->rAngle = acos(cosSpeed/newContactSample->rSpeed)*180/M_PI + alpha1;
				else
					newContactSample->rAngle = 0;
				newContactSample->rAngle = newContactSample->rAngle%360;

				newContactSample->fromPair = aContactSamplePair;
				duallist_add_in_sequence_from_tail(&aContactSamplePair->contents, newContactSample, (int(*)(void*,void*))contact_sample_has_earlier_timestamp_than);
			}
			thatItem = thatItem->next;
		}	
		thisItem = thisItem->next;
	}

}



int are_two_reps_meeting(struct Report *thisRep, struct Report *thatRep, int tGran, int sGran)
{
	
	if (strcmp(thisRep->fromTrace->vName, thatRep->fromTrace->vName) != 0) {
		if (distance_in_meter(thisRep->gPoint.x,thisRep->gPoint.y, thatRep->gPoint.x, thatRep->gPoint.y) <= sGran 
		 && ABS(thisRep->timestamp - thatRep->timestamp) <= tGran) {
			return 1;
		}
	}
	return 0;
}



void dump_contact_samples(struct Hashtable *pairSmpTable, FILE *fdump)
{
	struct Item *aItem;
	struct Pair *aContactSamplePair;
	unsigned long i;

	if(pairSmpTable == NULL) {
		return;
	}

	fprintf(fdump, "%d\n", FILE_CONTACT_SAMPLE);
	for(i = 0; i < pairSmpTable->size; i++) {
		aItem = pairSmpTable->head[i];
		while (aItem != NULL){
			aContactSamplePair = (struct Pair*)aItem->datap;
			dump_contact_sample_pair(aContactSamplePair, fdump);
			aItem = aItem->next;
		}
	}
}

void load_contact_samples_with_hashtable(FILE *fsource, struct Region *aRegion, struct Hashtable *table, time_t *startAt, time_t *endAt)
{
	char buf[128], *vName1, *vName2, *tm, *strp, key[128];
	struct ContactSample *newContactSample;
	struct Item *aPairItem;
	struct Pair *aPair = NULL;
	time_t timestamp;
	time_t tableStartAt, tableEndAt;
	struct Point aPoint, bPoint;
	double distance;
	int rAngle, rSpeed;
	int needed;
	int first;
	
	if(table == NULL)
		return;

	first = 1;
	while(fgets(buf, 128, fsource)) {
		vName1 = strtok(buf, ",");
		vName2 = strtok(NULL, ",");
		tm = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		aPoint.x = atof(strp);
		strp = strtok(NULL, ",");
		aPoint.y = atof(strp);
		strp = strtok(NULL, ",");
		bPoint.x = atof(strp);
		strp = strtok(NULL, ",");
		bPoint.y = atof(strp);
		strp = strtok(NULL, ",");
		distance = atof(strp);
		strp = strtok(NULL, ",");
		rAngle = atoi(strp);
		strp = strtok(NULL, ",");
		rSpeed = atoi(strp);
		timestamp = strtot(tm);

		if(aRegion == NULL && *startAt == 0 && *endAt == 0) {
			needed = 1;
		} else if (aRegion == NULL && *startAt == 0 && *endAt) {
			if (timestamp <= *endAt) needed = 1; else needed = 0;
		} else if (aRegion == NULL && *startAt && *endAt == 0) {
			if (timestamp >= *startAt) needed = 1; else needed = 0;
		} else if (aRegion == NULL && *startAt && *endAt) {
			if (timestamp <= *endAt && timestamp >= *startAt) needed = 1; else needed = 0;
		} else if (aRegion && *startAt == 0 && *endAt == 0) {
			if (is_point_in_polygon(&aPoint, aRegion->chosen_polygon) && is_point_in_polygon(&bPoint, aRegion->chosen_polygon) )
				needed = 1; else needed = 0;
		} else if (aRegion && *startAt == 0 && *endAt) {
			if (timestamp <= *endAt && is_point_in_polygon(&aPoint, aRegion->chosen_polygon) && is_point_in_polygon(&bPoint, aRegion->chosen_polygon))
				needed = 1; else needed = 0;
		} else if (aRegion && *startAt && *endAt == 0) {
			if (is_point_in_polygon(&aPoint, aRegion->chosen_polygon) && is_point_in_polygon(&bPoint, aRegion->chosen_polygon) && timestamp >= *startAt)
				needed = 1; else needed = 0;
		} else if (aRegion && *startAt && *endAt) {
			if (is_point_in_polygon(&aPoint, aRegion->chosen_polygon) && is_point_in_polygon(&bPoint, aRegion->chosen_polygon) && timestamp >= *startAt && timestamp <= *endAt)
				needed = 1; else needed = 0;
		} 
		if( needed ) {	
			newContactSample = (struct ContactSample*)malloc(sizeof(struct ContactSample));
			newContactSample->timestamp = timestamp;
			newContactSample->gPoint1.x = aPoint.x;
			newContactSample->gPoint1.y = aPoint.y;
			newContactSample->gPoint2.x = bPoint.x;
			newContactSample->gPoint2.y = bPoint.y;
			newContactSample->distance = distance;
			newContactSample->rSpeed = rSpeed;
			newContactSample->rAngle = rAngle;

			sprintf(key, "%s,%s", vName1, vName2);
			aPairItem = hashtable_find(table, key);
			if(aPairItem == NULL) {
				aPair = (struct Pair*)malloc(sizeof(struct Pair));
				pair_init_func(aPair);
				strncpy(aPair->vName1, vName1, strlen(vName1)+1);
				strncpy(aPair->vName2, vName2, strlen(vName2)+1);
				hashtable_add(table, key, aPair);
				aPair->startAt = timestamp;
				aPair->endAt = timestamp;
				if(first) {
					tableStartAt = aPair->startAt;
					tableEndAt = aPair->endAt;
					first = 0;
				}	
			} else {
				aPair = (struct Pair*)aPairItem->datap;
				if(timestamp < aPair->startAt) {
					aPair->startAt = timestamp;
					if(aPair->startAt < tableStartAt)
						tableStartAt = aPair->startAt;
				}
				if(timestamp > aPair->endAt) {
					aPair->endAt = timestamp;
					if(aPair->endAt > tableEndAt)
						tableEndAt = aPair->endAt;
				}
			}
			newContactSample->fromPair = aPair;
			duallist_add_to_tail(&aPair->contents, newContactSample);
		}
	}

	if(table->count) {
		*startAt = tableStartAt;
		*endAt = tableEndAt;
	}
}


void dump_contacts(struct Hashtable *pairContTable, FILE *fdump)
{
	struct Item *aItem;
	struct Pair *aContactPair;
	unsigned long i;

	if(pairContTable == NULL) {
		return;
	}

	fprintf(fdump, "%d\n", FILE_CONTACT);
	for(i = 0; i < pairContTable->size; i++) {
		aItem = pairContTable->head[i];
		while (aItem != NULL){
			aContactPair = (struct Pair*)aItem->datap;
			dump_contact_pair(aContactPair, fdump);
			aItem = aItem->next;
		}
	}
}


void dump_contact_pair(struct Pair *aPair, FILE *fdump)
{
	struct Item *aItem;
	struct Contact *aContact;
	char buf1[128], buf2[128];

	aItem = aPair->contents.head;
	while(aItem!=NULL) {
		aContact = (struct Contact*)aItem->datap;
		ttostr(aContact->startAt, buf1);
		ttostr(aContact->endAt, buf2);
		fprintf(fdump, "%s,%s,%s,%s,%10.6lf,%10.6lf,%d,%d\n",
				aPair->vName1,
				aPair->vName2,
				buf1,
				buf2,
				aContact->gPoint.x,
				aContact->gPoint.y,
				aContact->xNumber,
				aContact->yNumber);
		aItem = aItem->next;
	}
}


void load_contacts_with_hashtable(FILE *fsource, struct Region *aRegion, struct Hashtable *table, int mode, time_t *startAt, time_t *endAt)
{
	char buf[128], *vName1, *vName2, *tm1,*tm2, *strp, key[128];
	struct Contact *newContact;
	struct Item *aPairItem, *anEgoItem, *aLinkmanItem;
	struct Pair *aPair = NULL;
	struct Ego *anEgo = NULL;
	struct Linkman *aLinkman = NULL;

	time_t sAt, eAt;
	time_t tableStartAt, tableEndAt;
	struct Point aPoint;
	int xNumber, yNumber;
	int needed;
	int first;

	if(table == NULL)
		return;
	first = 1;
	while(fgets(buf, 128, fsource)) {

		vName1 = strtok(buf, ",");
		vName2 = strtok(NULL, ",");
		tm1 = strtok(NULL, ",");
		tm2 = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		aPoint.x = atof(strp);
		strp = strtok(NULL, ",");
		aPoint.y = atof(strp);
		strp = strtok(NULL, ",");
		xNumber = atoi(strp);
		strp = strtok(NULL, ",");
		yNumber = atoi(strp);
		sAt = strtot(tm1);
		eAt = strtot(tm2);

		if(aRegion == NULL && *startAt == 0 && *endAt == 0) {
			needed = 1;
		} else if (aRegion == NULL && *startAt == 0 && *endAt) {
			if (eAt <= *endAt) needed = 1; else needed = 0;
		} else if (aRegion == NULL && *startAt && *endAt == 0) {
			if (sAt >= *startAt) needed = 1; else needed = 0;
		} else if (aRegion == NULL && *startAt && *endAt) {
			if (eAt <= *endAt && sAt >= *startAt) needed = 1; else needed = 0;
		} else if (aRegion && *startAt == 0 && *endAt == 0) {
			if (is_point_in_polygon(&aPoint, aRegion->chosen_polygon))
				needed = 1; else needed = 0;
		} else if (aRegion && *startAt == 0 && *endAt) {
			if (eAt <= *endAt && is_point_in_polygon(&aPoint, aRegion->chosen_polygon))
				needed = 1; else needed = 0;
		} else if (aRegion && *startAt && *endAt == 0) {
			if (is_point_in_polygon(&aPoint, aRegion->chosen_polygon) && sAt >= *startAt)
				needed = 1; else needed = 0;
		} else if (aRegion && *startAt && *endAt) {
			if (is_point_in_polygon(&aPoint, aRegion->chosen_polygon) && sAt >= *startAt && eAt <= *endAt)
				needed = 1; else needed = 0;
		} 
		if( needed ) {	
			newContact = (struct Contact*)malloc(sizeof(struct Contact));
			newContact->gPoint.x = aPoint.x;
			newContact->gPoint.y = aPoint.y;
			newContact->xNumber = xNumber;
			newContact->yNumber = yNumber;
			newContact->startAt = sAt;
			newContact->endAt = eAt;

			if(mode == PAIRWISE_TABLE) {
				sprintf(key, "%s,%s", vName1, vName2);
				aPairItem = hashtable_find(table, key);
				if(aPairItem == NULL) {
					aPair = (struct Pair*)malloc(sizeof(struct Pair));
					pair_init_func(aPair);
					strncpy(aPair->vName1, vName1, strlen(vName1)+1);
					strncpy(aPair->vName2, vName2, strlen(vName2)+1);
					hashtable_add(table, key, aPair);
					aPair->color.integer = rand();
					aPair->startAt = sAt;
					aPair->endAt = eAt;
					if(first) {
						tableStartAt = aPair->startAt;
						tableEndAt = aPair->endAt;
						first = 0;
					}	
				} else {
					aPair = (struct Pair*)aPairItem->datap;
					if(sAt < aPair->startAt) {
						aPair->startAt = sAt;
						if(aPair->startAt < tableStartAt)
							tableStartAt = aPair->startAt;
					}
					if(eAt > aPair->endAt) {
						aPair->endAt = eAt;
						if(aPair->endAt > tableEndAt)
							tableEndAt = aPair->endAt;
					}
				}

				newContact->fromPair = aPair;
				duallist_add_to_tail(&aPair->contents, newContact);
			} else {
				/* Ego vName1 */
				anEgoItem = hashtable_find(table, vName1);
				if(anEgoItem == NULL) {
					anEgo = (struct Ego*)malloc(sizeof(struct Ego));
					ego_init_func(anEgo);
					strncpy(anEgo->vName, vName1, strlen(vName1)+1);
					hashtable_add(table, vName1, anEgo);
					anEgo->startAt = sAt;
					anEgo->endAt = eAt;
					if(first) {
						tableStartAt = anEgo->startAt;
						tableEndAt = anEgo->endAt;
						first = 0;
					}	
				} else {
					anEgo = (struct Ego*)anEgoItem->datap;
					if(sAt < anEgo->startAt) {
						anEgo->startAt = sAt;
						if(anEgo->startAt < tableStartAt)
							tableStartAt = anEgo->startAt;
					}
					if(eAt > anEgo->endAt) {
						anEgo->endAt = eAt;
						if(anEgo->endAt > tableEndAt)
							tableEndAt = anEgo->endAt;
					}
				}

				aLinkmanItem = duallist_find(&anEgo->linkmen, vName2, (int(*)(void*,void*))linkman_has_name);
				if(aLinkmanItem == NULL) {
					aLinkman = (struct Linkman*)malloc(sizeof(struct Linkman));
					linkman_init_func(aLinkman);
					aLinkman->color.integer=rand();
					strncpy(aLinkman->vName, vName2, strlen(vName2)+1);
					duallist_add_to_tail(&anEgo->linkmen, aLinkman);
				} else 
					aLinkman = (struct Linkman*)aLinkmanItem->datap;
				newContact->fromPair = NULL;
				duallist_add_to_tail(&aLinkman->contacts, newContact);

				/* Ego vName2 */
				anEgoItem = hashtable_find(table, vName2);
				if(anEgoItem == NULL) {
					anEgo = (struct Ego*)malloc(sizeof(struct Ego));
					ego_init_func(anEgo);
					strncpy(anEgo->vName, vName2, strlen(vName2)+1);
					hashtable_add(table, vName2, anEgo);
					anEgo->startAt = sAt;
					anEgo->endAt = eAt;
				} else {
					anEgo = (struct Ego*)anEgoItem->datap;
					if(sAt < anEgo->startAt) {
						anEgo->startAt = sAt;
					}
					if(eAt > anEgo->endAt) {
						anEgo->endAt = eAt;
					}
				}

				aLinkmanItem = duallist_find(&anEgo->linkmen, vName1, (int(*)(void*,void*))linkman_has_name);
				if(aLinkmanItem == NULL) {
					aLinkman = (struct Linkman*)malloc(sizeof(struct Linkman));
					linkman_init_func(aLinkman);
					aLinkman->color.integer=rand();
					strncpy(aLinkman->vName, vName1, strlen(vName1)+1);
					duallist_add_to_tail(&anEgo->linkmen, aLinkman);
				} else 
					aLinkman = (struct Linkman*)aLinkmanItem->datap;
				duallist_add_to_tail(&aLinkman->contacts, contact_copy_func(newContact));
			}
		}

	}
	if(table->count) {
		*startAt = tableStartAt;
		*endAt = tableEndAt;
	}
}

void retrieve_ict(struct Hashtable *cntTable, struct Hashtable *ictTable)
{
	unsigned long i;
	struct Item *aItem, *aContactItem, *aIctPairItem;
	struct Contact *aContact, *bContact;
	struct Pair *aPair, *aIctPair;
	struct ICT *aIct;
	char key[64]; 

	if(cntTable == NULL || ictTable == NULL)
		return;
	for(i=0;i<cntTable->size;i++) {
		aItem = cntTable->head[i];
		while(aItem!=NULL) {
			aPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the ict pair table */
			sprintf(key, "%s,%s", aPair->vName1, aPair->vName2);
			aIctPairItem = hashtable_find(ictTable, key);
			if(aIctPairItem == NULL) {
				aIctPair = (struct Pair*)malloc(sizeof(struct Pair));
				pair_init_func(aIctPair);
				strncpy(aIctPair->vName1, aPair->vName1, strlen(aPair->vName1)+1);
				strncpy(aIctPair->vName2, aPair->vName2, strlen(aPair->vName2)+1);
				hashtable_add(ictTable, key, aIctPair);
			} else { 
				aIctPair = (struct Pair*)aIctPairItem->datap;
			}

			aContactItem = aPair->contents.head;
			while(aContactItem!=NULL && aContactItem->next!=NULL) {
				aContact = (struct Contact*)aContactItem->datap;
				bContact = (struct Contact*)aContactItem->next->datap;
				aIct = (struct ICT*)malloc(sizeof(struct ICT));
				aIct->timestamp = bContact->startAt;
				aIct->ict = bContact->startAt-aContact->endAt;
				duallist_add_to_tail(&aIctPair->contents, aIct);
				aContactItem = aContactItem->next;
			}
			aItem = aItem->next;
		}
	}

}

void dump_icts(struct Hashtable *pairIctTable, FILE *fdump)
{
	struct Item *aItem;
	struct Pair *aIctPair;
	unsigned long i;

	if(pairIctTable == NULL) {
		return;
	}

	fprintf(fdump, "%d\n", FILE_ICT);
	for(i = 0; i < pairIctTable->size; i++) {
		aItem = pairIctTable->head[i];
		while (aItem != NULL){
			aIctPair = (struct Pair*)aItem->datap;
			dump_ict_pair(aIctPair, fdump);
			aItem = aItem->next;
		}
	}
}


void dump_ict_pair(struct Pair *aPair, FILE *fdump)
{
	struct Item *aItem;
	struct ICT *aIct;
	char buf1[128];

	aItem = aPair->contents.head;
	while(aItem!=NULL) {
		aIct = (struct ICT*)aItem->datap;
		ttostr(aIct->timestamp, buf1);
		fprintf(fdump, "%s,%s,%s,%ld\n",
				aPair->vName1,
				aPair->vName2,
				buf1,
				aIct->ict);
		aItem = aItem->next;
	}
}

void load_icts_with_hashtable(FILE *fsource, struct Hashtable *table, time_t *startAt, time_t *endAt)
{
	char buf[128], *vName1, *vName2, *tm1, *strp, key[128];
	struct ICT *newIct;
	struct Item *aPairItem;
	struct Pair *aPair;
	time_t timestamp;
	time_t tableStartAt, tableEndAt;
	int first;
	int needed;

	if(table == NULL)
		return;
	while(fgets(buf, 128, fsource)) {

		vName1 = strtok(buf, ",");
		vName2 = strtok(NULL, ",");
		tm1 = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		timestamp = strtot(tm1);

		if(*startAt == 0 && *endAt == 0) {
			needed = 1;
		} else if (*startAt == 0 && *endAt) {
			if (timestamp <= *endAt) needed = 1; else needed = 0;
		} else if ( *startAt && *endAt == 0) {
			if (timestamp >= *startAt) needed = 1; else needed = 0;
		} else if (*startAt && *endAt) {
			if (timestamp <= *endAt && timestamp >= *startAt) needed = 1; else needed = 0;
		}

		if( needed ) {	
			newIct = (struct ICT*)malloc(sizeof(struct ICT));
			newIct->ict = atol(strp);
			newIct->timestamp = strtot(tm1);

			sprintf(key, "%s,%s", vName1, vName2);
			aPairItem = hashtable_find(table, key);
			if(aPairItem == NULL) {
				aPair = (struct Pair*)malloc(sizeof(struct Pair));
				pair_init_func(aPair);
				strncpy(aPair->vName1, vName1, strlen(vName1)+1);
				strncpy(aPair->vName2, vName2, strlen(vName2)+1);
				hashtable_add(table, key, aPair);
				aPair->startAt = timestamp;
				aPair->endAt = timestamp;
				if(first) {
					tableStartAt = aPair->startAt;
					tableEndAt = aPair->endAt;
					first = 0;
				}	
			} else {
				aPair = (struct Pair*)aPairItem->datap;
				if(timestamp < aPair->startAt) {
					aPair->startAt = timestamp;
					if(aPair->startAt < tableStartAt)
						tableStartAt = aPair->startAt;
				}
				if(timestamp > aPair->endAt) {
					aPair->endAt = timestamp;
					if(aPair->endAt > tableEndAt)
						tableEndAt = aPair->endAt;
				}
			}
			duallist_add_to_tail(&aPair->contents, newIct);
		}
	}

	if(table->count) {
		*startAt = tableStartAt;
		*endAt = tableEndAt;
	}
}

void ego_init_func(struct Ego *anEgo)
{
	if (anEgo) {
		memset(anEgo->vName, '\0', 2*NAME_LENGTH);
		duallist_init(&anEgo->linkmen);
	}
}

void ego_free_func(struct Ego *anEgo)
{
	if(anEgo) {
		duallist_destroy(&anEgo->linkmen, (void(*)(void*))linkman_free_func);
		free(anEgo);
	}
}

int ego_has_name(char* name, struct Ego *anEgo)
{
	return !strcmp(name, anEgo->vName);
}


void linkman_init_func(struct Linkman *aLinkman)
{
	if (aLinkman) {
		memset(aLinkman->vName, '\0', 2*NAME_LENGTH);
		duallist_init(&aLinkman->contacts);
	}
}

void linkman_free_func(struct Linkman *aLinkman)
{
	if(aLinkman) {
		duallist_destroy(&aLinkman->contacts, free);
		free(aLinkman);
	}
}

int linkman_has_name(char* name, struct Linkman *aLinkman)
{
	return !strcmp(name, aLinkman->vName);
}

void set_pair_table_time(struct Hashtable *pairs, time_t atClock)
{
	int first;
	struct Item *aItem;
	struct Item *bItem;
	struct Contact *aContact;
	struct Pair *aPair;
	unsigned long i;

	for(i = 0; i<pairs->size; i++) {
		aItem = pairs->head[i];
		while (aItem!=NULL)
		{
			first = 1;
			aPair = (struct Pair*)aItem->datap;
			aPair->countdown = -1;
			bItem = aPair->contents.head;
			while(bItem != NULL) {
				aContact = (struct Contact*)bItem->datap;
				aContact->shown = 0;
				if(first && aContact->startAt >= atClock) {
					aPair->at = bItem;
					aPair->countdown = difftime(aContact->startAt, atClock);
					first --;
				}
				bItem = bItem->next;
			}
			aItem = aItem->next;
		}
	}
}

void set_selected_contacts_time(struct Duallist *selectedPairs, int mode, time_t atClock)
{
	struct Item *aItem;
	struct Item *bItem, *cItem;
	struct Pair *aSelectedPair;
	struct Ego *aSelectedEgo;
	struct Linkman *aLinkman;
	struct Contact *aContact;

	aItem = selectedPairs->head;
	while(aItem!=NULL) {
		if(mode == PAIRWISE_TABLE) {
			aSelectedPair = (struct Pair*)aItem->datap;
			aSelectedPair->countdown = -1;
			bItem = aSelectedPair->contents.head;
			while(bItem) {
				aContact = (struct Contact*)bItem->datap;
				aContact->shown = 1;
				if(aContact->startAt >= atClock) {
					aSelectedPair->at = bItem;
					aSelectedPair->countdown = difftime(aContact->startAt, atClock);
					break;
				}
				bItem = bItem->next;
			}

			while(bItem) {
				aContact = (struct Contact*)bItem->datap;
				aContact->shown = 0;
				bItem = bItem->next;
			}
		} else {
			aSelectedEgo = (struct Ego*)aItem->datap;
			cItem = aSelectedEgo->linkmen.head;
			while(cItem) {
				aLinkman = (struct Linkman*)cItem->datap;
				aLinkman->countdown = -1;
				bItem = aLinkman->contacts.head;
				while(bItem) {
					aContact = (struct Contact*)bItem->datap;
					aContact->shown = 1;
					if(aContact->startAt >= atClock) {
						aLinkman->at = bItem;
						aLinkman->countdown = difftime(aContact->startAt, atClock);
						break;
					}
					bItem = bItem->next;
				}
				while(bItem) {
					aContact = (struct Contact*)bItem->datap;
					aContact->shown = 0;
					bItem = bItem->next;
				}
				cItem = cItem->next;
			}

		}
		aItem = aItem->next;
	}
}

struct Contact *contact_copy_func(struct Contact *aContact)
{
	struct Contact *rtContact;

	if(aContact) {
		rtContact = (struct Contact*)malloc(sizeof(struct Contact));
		rtContact->startAt = aContact->startAt;
		rtContact->endAt = aContact->endAt;
		rtContact->gPoint.x = aContact->gPoint.x;
		rtContact->gPoint.y = aContact->gPoint.y;

		rtContact->xNumber = aContact->xNumber;
		rtContact->yNumber = aContact->yNumber;
	}
	return rtContact;
}
