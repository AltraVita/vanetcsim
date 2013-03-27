#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include "common.h"
#include "files.h"
#include "geometry.h"
#include "trace.h"
#include "dumpruns.h"



int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	struct Region *region;
	char dumpfile[256], *directory;

	struct Hashtable traces;
	long traceTableSize = 2000;
	struct Item *aItem;
	struct Trace *aTrace;

	int i;


	if(argc < 3) {
	      printf("%s is used to calculate running distance in charge.\n", argv[0]);
	      printf("Usage: %s [-d directory] .map (.mgd | .lst ...)\n", argv[0]);
	      exit(1);
	}

	directory = "./";
	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'd':
		      directory = argv[2];
		      argc-=2;
		      argv+=2;
		      break;
	      default:
	   	      printf("Usage: %s [-d directory] .map (.mgd |.lst ...)\n", argv[0]);
	      }
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, -1, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}

	hashtable_init(&traces, traceTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			printf("Loading %s file ...\n", argv[1]);
			load_source_file(fsource, region, &traces, (void*(*)(int, FILE*, struct Region *, void*, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL,NULL,NULL,NULL,NULL,NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	if(region!=NULL && traces.count != 0) {
	      for (i = 0; i<traces.size; i++) {
		      aItem = traces.head[i];
		      while(aItem != NULL ) {
				aTrace = (struct Trace*)aItem->datap;
				sprintf(dumpfile, "%s/%s.run", directory, aTrace->vName);
				if( (fdump = fopen(dumpfile, "w"))!=NULL) {
					dump_runs_in_charge(fdump, region, aTrace);
					fflush(fdump);
					fclose(fdump);
				}
			      	aItem = aItem->next;
		      }
	      }
	} 
	 
	hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
	region_free_func(region);
	return 0;
}


void dump_runs_in_charge(FILE *fdump, struct Region *aRegion, struct Trace *aTrace)
{
	struct Item *aItem, *nextItem; 
	struct Report *aRep, *nextRep;
	char buf1[20], buf2[20], valid, begin;
	double rundist, nightdist; 
	struct tm *atm;
	int asecs, bsecs;
	struct Path *aPath;


	if (aTrace == NULL)
		return;

	printf("Checking charge runs in trace: %s ...\n", aTrace->vName);

	rundist = 0;
	nightdist = 0;
	begin = 1;
	valid = 1;
	aItem = aTrace->reports.head;
	while(aItem!=NULL) {
		aRep = (struct Report*)aItem->datap;
		if(aRep->state != TRACE_STATE_LIGHT) {
			if(begin) {
				ttostr(aRep->timestamp, buf1);
				begin = 0;
			}

			atm = localtime(&aRep->timestamp);
			asecs = atm->tm_hour*3600 + atm->tm_min*60 + atm->tm_sec;

			if(valid) {
				nextItem = aItem->next;
				if(nextItem != NULL) {
					nextRep = (struct Report*)nextItem->datap;
					if(nextRep->state != TRACE_STATE_LIGHT) {

						atm = localtime(&nextRep->timestamp);
						bsecs = atm->tm_hour*3600 + atm->tm_min*60 + atm->tm_sec;
						ttostr(nextRep->timestamp, buf2);
						aPath = get_path_between_two_reports(aRegion, aRep, nextRep, -1);
						if( aPath != NULL) {
							aPath->length = distance_on_path(aPath, &aRep->gPoint, &nextRep->gPoint);
							rundist += aPath->length;
							if(!(asecs >= 6*3600 && bsecs <= 23*3600)) {
								nightdist += aPath->length;
							}
							path_free_func(aPath);	
						} else {
							valid = 0;
							rundist = -1;
							nightdist = -1;
						}
					}
				}	
			}
		} else {
			if(rundist != 0) {
				fprintf(fdump, "%s,%s,%s,%.2lf,%.2lf\n", aTrace->vName, buf1, buf2, rundist, nightdist);
				begin = 1;
				valid = 1;
				rundist = 0;
				nightdist = 0;
			}
		}
		aItem = aItem->next;
	}
}


