#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<time.h>
#include<math.h>
#include"files.h"
#include"common.h"
#include"contact.h"
#include"trace.h"
#include"gtk_cairo.h"
#include"files.h"

#define LOADING_LOCATION_COUNT 0x1
#define UNLOADING_LOCATION_COUNT 0x10
#define LIGHT_LOCATION_COUNT 0x100
#define HEAVY_LOCATION_COUNT 0x1000
#define CONTACT_LOCATION_COUNT 0x10000

void count_reports_and_contacts(struct Region *region, struct Hashtable *traces, struct Hashtable *pairs, int interests, int accumulative, time_t tillClock)
{
	long i;
	struct Item *aItem, *bItem;
	struct Pair *aPair;
	struct Report *aRep;
	struct Cell *aCell;
	struct Contact *aContact;
	struct Trace* aTrace;
	
	int xNumber, yNumber;

	if (!accumulative) {
		for(xNumber = 0; xNumber<region->hCells; xNumber++)
			for(yNumber = 0; yNumber<region->vCells; yNumber++) {
				aCell = region->mesh + xNumber*region->vCells + yNumber;
				aCell->n1 = 0;
				aCell->n2 = 0;
				aCell->n3 = 0;
				aCell->n4 = 0;
				aCell->n5 = 0;
				aCell->n6 = 0;
			}
	}

	if(region!=NULL && traces->count != 0) {
	      for (i = 0; i<traces->size; i++) {
		      aItem = traces->head[i];
		      while(aItem != NULL ) {
			      aTrace = (struct Trace*)aItem->datap;
			      bItem = aTrace->at;
			      while(bItem != NULL && ((struct Report*)bItem->datap)->timestamp <= tillClock) {
				      aRep = (struct Report*)bItem->datap; 
				      if(is_point_in_box(&aRep->gPoint, &region->chosen_polygon->box)) {
					      xNumber = floor((aRep->gPoint.x - region->chosen_polygon->box.xmin)/region->cellSize);
					      yNumber = floor((aRep->gPoint.y - region->chosen_polygon->box.ymin)/region->cellSize);
					      aCell = region->mesh + xNumber*region->vCells + yNumber;
					      /* Cell.n1 is for locations of loading    (0x1),
						 Cell.n2 is for locations of unloading  (0x10),
						 Cell.n3 is for locations of light taxi (0x100),
						 Cell.n4 is for locations of heavy taxi (0x1000),
						 Cell.n5 is for locations of contacts   (0x10000),
					      */
					      /* the taxi is light */
					      if (aRep->state == 0) {
						      if(interests & 0x100)
							      aCell->n3 ++;
						      if( bItem!=aTrace->reports.head 
						       && (interests & 0x10) 
						       && ((struct Report*)bItem->prev->datap)->state != 0)
							      aCell->n2 ++;
					      } else {
						      if(interests & 0x1000)
							      aCell->n4 ++;
						      if( bItem!=aTrace->reports.head 
						       && (interests & 0x1) 
						       && ((struct Report*)bItem->prev->datap)->state == 0)
							      aCell->n1 ++;
					      }
				      }
				      bItem = bItem->next;
			      }
			      aTrace->at = bItem;
			      aItem = aItem->next;
		      }
	      }
	} 

	if(region!=NULL && pairs->count != 0) {
	      for (i = 0; i<pairs->size; i++) {
		      aItem = pairs->head[i];
		      while(aItem != NULL) {
			      aPair = (struct Pair*)aItem->datap;
			      bItem = aPair->at;
			      while(bItem != NULL && ((struct Contact*)bItem->datap)->startAt <= tillClock) {
				      aContact = (struct Contact*)bItem->datap;
				      xNumber = floor((aContact->gPoint.x - region->chosen_polygon->box.xmin)/region->cellSize);
				      yNumber = floor((aContact->gPoint.y - region->chosen_polygon->box.ymin)/region->cellSize);
				      aCell = region->mesh + xNumber*region->vCells + yNumber;
				      if(interests & 0x10000)
					      aCell->n5 ++;
				      bItem = bItem->next;
			      }
			      aPair->at = bItem;
			      aItem = aItem->next;
		      }
	      }
	} 
}



void dump_reports_and_contacts(FILE *fdump, struct Region *region, int interests, time_t atClock) 
{
	int m, n;
	struct Cell *aCell;
	unsigned long value;
	char buf[128];


	if (interests == 0) 
	      return;

	for(m=0; m<region->hCells; m++) {
	      for(n=0; n<region->vCells; n++) {
		      aCell = region->mesh + m*region->vCells + n;
		      value = 0;
		      
		      if(interests & 0x1)
			      value += aCell->n1;
		      if(interests & 0x10)
			      value += aCell->n2;
		      if(interests & 0x100)
			      value += aCell->n3;
		      if(interests & 0x1000)
			      value += aCell->n4;
		      if(interests & 0x10000)
			      value += aCell->n5;
		      if(value != aCell->n6) {
			      aCell->n6 = value;
			      ttostr(atClock, buf);
			      fprintf(fdump, "%d,%d,%s,%ld\n", m, n, buf, value);
		      }
	      }
      }
}


int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	struct Region *region;
	char *dumpfile = "dump.dsp";

	struct Hashtable traces;
	struct Hashtable pairs;
	long traceTableSize = 2500;
	long pairTableSize = 10e5;
	int interests = 0, accumulative = 0;

	time_t lstart = 0, lend = 0;
	time_t atClock;
	time_t tGran = 60;

	double minvalue = 0, maxvalue = 100;
	int first;

	if(argc < 3) {
	      printf("%s is used to compute the probability distribution funciton of GPS reports or contacts.\n", argv[0]);
	      printf("Usage: %s [-m \"loading\" | \"heavy\" | \"unloading\" | \"light\" | \"contact\"] [-gt tGran] [-accumulative] [-r minvalue maxvalue] [-w dump.dsp] .map [.ogd | .cont | .lst ...]\n", argv[0]);
	      exit(1);
	}

	dumpfile = NULL;
	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'm':
		      if(!strcmp(argv[2], "loading")) {
			      interests |= LOADING_LOCATION_COUNT;
		      } else if (!strcmp(argv[2], "unloading")) {
			      interests |= UNLOADING_LOCATION_COUNT;
		      } else if (!strcmp(argv[2], "light")) {
			      interests |= LIGHT_LOCATION_COUNT;
		      } else if (!strcmp(argv[2], "heavy")) {
			      interests |= HEAVY_LOCATION_COUNT;
		      } else if (!strcmp(argv[2], "contact")) {
			      interests |= CONTACT_LOCATION_COUNT;
		      }
		      argc-=2;
		      argv+=2;
		      break;
		      
	      case 'g':
		      tGran = atoi(argv[2]);
		      argc-=2;
		      argv+=2;
		      break;

	      case 'w':
		      dumpfile = argv[2];
		      argc-=2;
		      argv+=2;
		      break;

	      case 'a':
		      accumulative = 1;
		      argc-=1;
		      argv+=1;
		      break;
	
	      case 'r':
		      minvalue = atof(argv[2]);
		      maxvalue = atof(argv[3]);
		      argc-=3;
		      argv+=3;
		      break;

	      default:
		      printf("Usage: %s [-m \"loading\" | \"heavy\" | \"unloading\" | \"light\" | \"contact\"] [-gt tGran] [-w dump.dsp] .map [.ogd | .cont | .lst ...]\n", argv[0]);
	      }
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, -1, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}

	if((fdump=fopen(dumpfile, "w"))!= NULL) {
		fprintf(fdump, "%d\n", FILE_CELL_DISPLAYS);
		fprintf(fdump, "%lf %lf\n", minvalue, maxvalue);

		first = 1;
		while(argc > 1) {
			hashtable_init(&traces, traceTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
			hashtable_init(&pairs, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
			if((fsource=fopen(argv[1], "r"))!=NULL) {
			      	printf("Loading %s file ...\n", argv[1]);
				lstart = 0;
				lend = 0;
				load_source_file(fsource, region, &traces, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, &pairs, PAIRWISE_TABLE, (void*(*)(FILE*, struct Region*, void *, int, time_t *, time_t *))load_contacts_with_hashtable, NULL, NULL, NULL, NULL, NULL, NULL, &lstart, &lend, NULL);
			      	fclose(fsource);
				if(lstart) {
					if(first) {
						atClock = lstart;
						first --;
					}
					set_trace_table_time(&traces, atClock);
					set_pair_table_time(&pairs, atClock);
					while (atClock < lend) {
						count_reports_and_contacts(region, &traces, &pairs, interests, accumulative, atClock+tGran);
						dump_reports_and_contacts(fdump, region, interests, atClock);

						atClock += tGran;
					}
					atClock = lend;
				}
		      	}
			hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
			hashtable_destroy(&pairs, (void(*)(void*))pair_free_func);

		     	argc--;
		      	argv++;
		}
 
		fclose(fdump);
	}

	region_free_func(region);
	return 0;
}

