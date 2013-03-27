#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<time.h>
#include<math.h>
#include"contact.h"
#include"trace.h"
#include"files.h"
#include"common.h"

#define MAP_METHOD 0
#define PAIR_METHOD 1

int main(int argc, char **argv)
{
  FILE *fsource, *fdumpCont, *fdumpSmp;
  struct Region *region = NULL;
  char first;
  char key[128];
  struct ContactContext context;
  struct Hashtable traces;
  struct Hashtable pairSmpTable;
  struct Hashtable pairContTable;
  struct Duallist *spSlots = NULL;
  struct ContactSample  *aContactSample, *bContactSample, *cContactSample;
  struct Contact *newContact;
  struct Item *aPairItem, *aContPairItem, *aItem, *bItem;
  struct Pair *aPair, *aContPair;
  time_t startAt = 0, sAt, endAt = 0, eAt;

  unsigned long i;
  unsigned long traceTableSize = 2500;
  unsigned long pairTableSize = 10e5;
  char *dumpContFile = NULL;
  char *dumpSmpFile = NULL;

  int j;
  int method;
  double avgX, avgY;

  context.tGran = DEFAULT_MEETING_TEMPORAL_GRAN;
  context.sGran = DEFAULT_MEETING_SPATIAL_GRAN;
  context.numSlots = DEFAULT_NUMBER_OF_SLIDES;
  context.crossRange = DEFAULT_CROSS_RANGE;
  context.startAt = 0;
  context.endAt = 0;
  context.type1 = VEHICLE_TYPE_NULL;
  context.type2 = VEHICLE_TYPE_NULL;


  if(argc < 3) {
	printf("Usage: %s [-time startAt endAt] [-gt meeting_temporal_gran] [-gs meeting_spatial_gran] [-n number_slots] [-r cross_range] [-st number_of_traces] [-sp number_of_pairs] [-wc dump.cont] [-ws dump.csmp] [-method map .map | pair ] [-kind type1 type2 {taxi, bus}] [.csmp | .lst | .ogd | .mgd ...]\n", argv[0]);
	exit(1);
  }

  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		startAt = strtot(argv[2]);
		endAt = strtot(argv[3]);
		argc-=3;
		argv+=3;
		break;

	case 'g':
		if(argv[1][2] == 't')
			context.tGran = atoi(argv[2]);
		if(argv[1][2] == 's')
			context.sGran = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'n':
		context.numSlots = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'k':
		if(strcmp(argv[2], "taxi")==0)
			context.type1 = VEHICLE_TYPE_TAXI;
		else if(strcmp(argv[2], "bus")==0)
			context.type1 = VEHICLE_TYPE_BUS;
		else 
			context.type1 = VEHICLE_TYPE_NULL;
		if(strcmp(argv[3], "taxi")==0)
			context.type2 = VEHICLE_TYPE_TAXI;
		else if(strcmp(argv[3], "bus")==0)
			context.type2 = VEHICLE_TYPE_BUS;
		else 
			context.type2 = VEHICLE_TYPE_NULL;
		argc-=3;
		argv+=3;
		break;
		
	case 'r':
		context.crossRange = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;
		
	case 's':
		if(argv[1][2] == 't')
			traceTableSize = atol(argv[2]);
		if(argv[1][2] == 'p')
			pairTableSize = atol(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'w':
		if(argv[1][2] == 'c')
			dumpContFile = argv[2];
		if(argv[1][2] == 's')
			dumpSmpFile = argv[2];
		argc-=2;
		argv+=2;
		break;

	case 'm':
		if (!strcmp("map", argv[2])) {
			method = MAP_METHOD;
			if((fsource=fopen(argv[3], "rb"))!=NULL) {
				region = region_load_func(fsource, NULL, -1, -1);
				get_cells_ready_for_trace(region, context.numSlots);
				fclose(fsource);
			}
			argc-=3;
			argv+=3;
			break;
		}
		if (!strcmp("pair", argv[2])) {
			method = PAIR_METHOD;
			spSlots = (struct Duallist*)malloc(sizeof(struct Duallist)*context.numSlots);
			for ( j = 0; j < context.numSlots; j++) 
				duallist_init(spSlots+j);
			argc-=2;
			argv+=2;
			break;
		}
		
	default:
		printf("Usage: %s [-time startAt endAt] [-gt meeting_temporal_gran] [-gs meeting_spatial_gran] [-n number_slots] [-r cross_range] [-st number_of_traces] [-sp number_of_pairs] [-wc dump.cont] [-ws dump.csmp] [-method map .map | pair ] [-kind type1 type2 {taxi, bus}] [.csmp | .lst | .ogd | .mgd ...]\n", argv[0]);
	}
  }

  first = 1;
  hashtable_init(&traces, traceTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
  hashtable_init(&pairSmpTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  while(argc > 1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[1]);
		sAt = startAt;
		eAt = endAt;
		load_source_file(fsource, region, &traces, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, &pairSmpTable, (void*(*)(FILE*, struct Region*, void *, time_t *, time_t *))load_contact_samples_with_hashtable, NULL, 0, NULL, NULL, NULL,NULL,NULL,NULL,NULL, &sAt, &eAt, NULL);
		fclose(fsource);
		if(sAt) {
			if(first) {
				context.startAt = sAt;
				context.endAt = eAt;
				first --;
			} else {
				if (sAt < context.startAt)
					context.startAt = sAt;
				if (eAt > context.endAt)
					context.endAt = eAt;
			}
		}
	}
	argc--;
	argv++;
  }


  /* find contact samples from GPS reports */
  if(traces.count != 0) {
        printf("Find contact samples from GPS reports ...\n");
	context.clock = context.startAt;
	set_traces_at_clock(&traces, context.clock);
	context.currentPos = context.newestPos = 0;
	int counter = 0;
	if (region != NULL && method == MAP_METHOD) {
		mount_traces_at_slot(region, &traces, &context, 0);
		mount_traces(region, &traces, &context);
		while(context.currentPos != context.newestPos) {
			find_contact_samples(region, &pairSmpTable, &context);
			mount_traces(region, &traces, &context);
		}
	} else if (method == PAIR_METHOD) {
		mount_traces_at_seperate_slot(&traces, &context, spSlots, 0);
		mount_traces_with_seperate_slots(&traces, &context, spSlots);
		while(context.currentPos != context.newestPos) {
		  counter ++;
		  printf("counter = %15d\n" , counter);
		  find_contact_samples_with_seperate_slots(&pairSmpTable, &context, spSlots);
		  free_used_reports(&traces);
		  mount_traces_with_seperate_slots(&traces, &context, spSlots);
		}
		
	}

  } 
  hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
  if((fdumpSmp=fopen(dumpSmpFile, "w"))!=NULL) {
  	dump_contact_samples(&pairSmpTable, fdumpSmp);
	fclose(fdumpSmp);
  }

  /* find contacts from contact samples */
  fdumpCont = fopen(dumpContFile, "w");
  if(fdumpCont != NULL) {
        printf("Find contacts from contact samples ...\n");
	hashtable_init(&pairContTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
	for (i=0;i<pairSmpTable.size;i++) {
		aPairItem = pairSmpTable.head[i];
		if(i%100==0)
		  printf("%15d/%-15d\n",i,pairSmpTable.size);
		while (aPairItem != NULL) {
			aPair = (struct Pair*)aPairItem->datap;
			aItem = aPair->contents.head;
			while(aItem!=NULL) {
				aContactSample = (struct ContactSample*)aItem->datap;
				j = 2;
				avgX = aContactSample->gPoint1.x + aContactSample->gPoint2.x;
				avgY = aContactSample->gPoint1.y + aContactSample->gPoint2.y;
				bItem = aItem->next;
				while(bItem!=NULL) {
					bContactSample = (struct ContactSample*)bItem->datap;
					cContactSample = (struct ContactSample*)bItem->prev->datap;
					if( are_contact_samples_duplicated(bContactSample, cContactSample)
					 || are_contact_samples_continuous(cContactSample, bContactSample, context.tGran, context.sGran)) {
						j+=2;
						avgX = avgX + bContactSample->gPoint1.x + bContactSample->gPoint2.x;
						avgY = avgY + bContactSample->gPoint1.y + bContactSample->gPoint2.y;
						bItem = bItem->next;
					} else
						break;
				}
				newContact = (struct Contact*)malloc(sizeof(struct Contact));
				newContact->startAt = aContactSample->timestamp;
				if(bItem != NULL && bItem->prev != aItem)
					newContact->endAt = cContactSample->timestamp + context.tGran;
				else
					newContact->endAt = aContactSample->timestamp + context.tGran;
				newContact->gPoint.x = avgX/j;
				newContact->gPoint.y = avgY/j;
				if(region) {
					newContact->xNumber = floor((newContact->gPoint.x - region->chosen_polygon->box.xmin)/region->cellSize);
					newContact->yNumber = floor((newContact->gPoint.y - region->chosen_polygon->box.ymin)/region->cellSize);
				}

				sprintf(key, "%s,%s", aPair->vName1, aPair->vName2);
				aContPairItem = hashtable_find(&pairContTable, key);
				if(aContPairItem == NULL) {
					aContPair = (struct Pair*)malloc(sizeof(struct Pair));
					strncpy(aContPair->vName1, aPair->vName1, strlen(aPair->vName1)+1);
					strncpy(aContPair->vName2, aPair->vName2, strlen(aPair->vName2)+1);
					duallist_init(&aContPair->contents);
					hashtable_add(&pairContTable, key, aContPair);
				} else
					aContPair = (struct Pair*)aContPairItem->datap;

				newContact->fromPair = aContPair;
				duallist_add_to_tail(&aContPair->contents, newContact);

				aItem = bItem;
			}
			aPairItem = aPairItem->next;
		}
	}
  	dump_contacts(&pairContTable, fdumpCont);
	fclose(fdumpCont);
  	hashtable_destroy(&pairContTable, (void(*)(void*))pair_free_func);
  }
  hashtable_destroy(&pairSmpTable, (void(*)(void*))pair_free_func);
  if(region) {
	clear_cells_from_trace(region, context.numSlots); 
	region_free_func(region);
  } 
  if(spSlots) {
	for (j = 0; j < context.numSlots; j++) {
		duallist_destroy(spSlots+j, NULL);
	}
	free(spSlots);	
  }
  return 0;
}

