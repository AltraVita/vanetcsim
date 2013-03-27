/* Pick reports from an ogd file */

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<time.h>
#include"trace.h"
#include"common.h"
#include"files.h"

int main(int argc, char *argv[])
{
  // deal with GPS data
  FILE *fsource, *fdump;
  char dumpfile[256], *directory;
  time_t tstart = -1, tend = -1;
  double xmin=-1,ymin=-1, xmax=-1, ymax=-1;
  struct Hashtable traces;
  struct Item *aItem, *aRepItem;
  struct Trace *aTrace;
  struct Report *aRep;
  unsigned long i, count;

  if(argc < 2) {
    printf("Usage: %s [-c xmin ymin xmax ymax] [-t from_timestamp until_timestamp] [-d directory] .ogd |.mgd|.lst\n", argv[0]);
    exit(1);
  } 
  directory = ".";
  while( (argv[1][0])=='-' ) {
    switch ( argv[1][1]) {
      case 'd':
	      directory = argv[2];
	      argc-=2;
	      argv+=2;
	      break;
      case 'c':
        xmin = atof(argv[2]);
        ymin = atof(argv[3]);
        xmax = atof(argv[4]);
        ymax = atof(argv[5]);
        argc-=5;
        argv+=5;
        break;
      case 't':
	tstart = strtot(argv[2]);
	tend = strtot(argv[3]);
        argc-=3;
        argv+=3;
        break;
       default:
        printf("Bad option %s\n", argv[1]);
        printf("Usage: %s [-c xmin ymin xmax ymax] [-tf from_timestamp] [-tu until_timestamp] .ogd \n", argv[0]);
        exit(1);
    }
  }

  hashtable_init(&traces, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
  while(argc > 1) {
	  if((fsource=fopen(argv[1], "r"))!=NULL) {
		  printf("Loading %s file ...\n", argv[1]);
		  load_source_file(fsource, NULL, &traces, (void*(*)(int,FILE*,struct Region*,void*,time_t*,time_t*,struct Box*))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
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
			if(aTrace->type == FILE_ORIGINAL_GPS_TAXI || aTrace->type == FILE_ORIGINAL_GPS_BUS)
				sprintf(dumpfile, "%s/%s.ogd", directory, aTrace->vName);
			else
				sprintf(dumpfile, "%s/%s.mgd", directory, aTrace->vName);
			if( (fdump = fopen(dumpfile, "w"))!=NULL) {
				fprintf(fdump, "%d\n", aTrace->type);
				count = 0;
				aRepItem = aTrace->reports.head;
				while(aRepItem != NULL) {
					aRep = (struct Report*)aRepItem->datap;
					if(xmin != -1 && (aRep->gPoint.x<xmin || aRep->gPoint.x>xmax || aRep->gPoint.y<ymin || aRep->gPoint.y>ymax)) {
						aRepItem = aRepItem->next;
						continue;
					}
					if(tstart!=-1 && aRep->timestamp < tstart ) {
						aRepItem = aRepItem->next;
						continue;
					}
					if(tend!=-1 && aRep->timestamp > tend) {
						break;
					}
					dump_report(fdump, aRep, aTrace->type);
					count = 1;
					aRepItem = aRepItem->next;
				}
				fflush(fdump);
				fclose(fdump);
				if(count==0)
					remove(dumpfile);
			}
			aItem = aItem->next;
		}
	}
  } 
   
  hashtable_destroy(&traces, (void(*)(void*))trace_free_func);

  return 0;
}
                                                                                                                             

