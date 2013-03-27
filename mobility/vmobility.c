#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"trace.h"
#include"geometry.h"
#include"common.h"
#include"files.h"

struct Trace_mobility
{
  char vName[NAME_LENGTH];
  struct Hashtable cells;
  unsigned long count;
};


void trace_mobility_free_func(struct Trace_mobility *aTraceMobility)
{
	if(aTraceMobility == NULL)
		return;
	hashtable_destroy(&aTraceMobility->cells, NULL);
	free(aTraceMobility);
}


int trace_mobility_has_name(char *name, struct Trace_mobility* aTraceMobility)
{
	if(aTraceMobility == NULL)
		return 0;
	return !strcmp(name, aTraceMobility->vName);
}

int trace_has_less_mobility_than(struct Trace_mobility *aTraceMobility, struct Trace_mobility *bTraceMobility)
{
	return aTraceMobility->count < bTraceMobility->count;
}

int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	struct Region *region;
	char *dumpfile = NULL, key[128];

	struct Hashtable traces;
	struct Hashtable traceMobilityTable;
	long traceTableSize = 2000;
	struct Item *aItem, *bItem, *cItem;
	struct Trace *aTrace;
	struct Report *aRep;
	struct Cell *aCell;
	struct Trace_mobility *aTraceMobility;

	unsigned long i;
	double fromRange = 0;
        double toRange = 0.5;

	unsigned long fromItem;
	unsigned long toItem;

	int days;
	struct Duallist sortedTraces;

	if(argc < 3) {
	      printf("Usage: %s [-r from to] [-w file] .map (.ogd|.lst ...)\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'w':
		      dumpfile = argv[2];
		      argc-=2;
		      argv+=2;
		      break;
	      case 'r':
		      fromRange = atof(argv[2]);
		      toRange = atof(argv[3]);
		      argc-=3;
		      argv+=3;
		      break;
	      default:
		      printf("Usage: %s [-r from to] .map (.ogd|.lst ...)\n", argv[0]);
	      }
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, -1, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}

	days = argc-1;

	hashtable_init(&traceMobilityTable, traceTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_mobility_has_name);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			hashtable_init(&traces, traceTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
			printf("Loading %s... \n", argv[1]);
			load_source_file(fsource, region, &traces, (void*(*)(int, FILE*, struct Region *, void*, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL,NULL,NULL,NULL,NULL,NULL);

			for (i = 0; i<traces.size; i++) {
				aItem = traces.head[i];
				while(aItem != NULL ) {
					aTrace = (struct Trace*)aItem->datap;
					bItem = hashtable_find(&traceMobilityTable, aTrace->vName);
					if(bItem == NULL) {
						aTraceMobility = (struct Trace_mobility*)malloc(sizeof(struct Trace_mobility));
						strncpy(aTraceMobility->vName, aTrace->vName, strlen(aTrace->vName)+1);
						hashtable_init(&aTraceMobility->cells, 10000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))cell_has_nos);

						hashtable_add(&traceMobilityTable, aTrace->vName, aTraceMobility);
					} else {
						aTraceMobility = (struct Trace_mobility*)bItem->datap;
					}
					bItem = aTrace->reports.head;
					while(bItem != NULL) {
						aRep = (struct Report*)bItem->datap;
						aCell = point_in_cell(region, &aRep->gPoint);
						if(aCell != NULL) {
							sprintf(key, "%d,%d", aCell->xNumber, aCell->yNumber);
							cItem = hashtable_find(&aTraceMobility->cells, key);
							if(cItem == NULL) {
								hashtable_add(&aTraceMobility->cells, key, aCell);
								aTraceMobility->count ++;
							}

						}
						bItem = bItem->next;
					}

					hashtable_destroy(&aTraceMobility->cells, NULL);
					hashtable_init(&aTraceMobility->cells, 10000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))cell_has_nos);
					aItem = aItem->next;
				}
			}

			fclose(fsource);
			hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
		}

		argc--;
		argv++;
	}

	duallist_init(&sortedTraces);
	for (i = 0; i<traces.size; i++) {
		aItem = traceMobilityTable.head[i];
		while(aItem != NULL ) {
			aTraceMobility = (struct Trace_mobility*)aItem->datap;
			duallist_add_in_sequence_from_head(&sortedTraces, aTraceMobility, (int(*)(void*,void*))trace_has_less_mobility_than);
			aItem = aItem->next;
		}
	}
	if(dumpfile!=NULL)
		fdump = fopen(dumpfile, "w");	
	fromItem = ceil(sortedTraces.nItems*fromRange);
	toItem = ceil(sortedTraces.nItems*toRange);
	aItem = sortedTraces.head;
	for(i=0;i<sortedTraces.nItems;i++) {
		aTraceMobility = (struct Trace_mobility*)aItem->datap;
		if(i>=fromItem && i<= toItem && fdump !=NULL) 
			fprintf(fdump, "%s\n", aTraceMobility->vName);
		printf("%.0lf ", aTraceMobility->count*1.0/days);
		aItem = aItem->next;
	}
	printf("\n");
	
	duallist_destroy(&sortedTraces, NULL);
	hashtable_destroy(&traceMobilityTable, (void(*)(void*))trace_mobility_free_func);
	region_free_func(region);
	if(dumpfile!=NULL)
		fclose(fdump);	
	return 0;
}

