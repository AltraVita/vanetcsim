#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "trace.h"
#include"common.h"
#include"files.h"
#include"shgps.h"


int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	char dumpfile[256], *directory;

	struct Hashtable traces;
	long traceTableSize = 2000;
	struct Item *aItem, *bItem;
	struct Trace *aTrace;
	struct Report *aRep;

	unsigned long i;


	if(argc < 2) {
	      printf("%s is used to convert old version ogd to latest version.\n", argv[0]);
	      printf("Usage: %s [-d directory] (.ogd|.lst ...)\n", argv[0]);
	      exit(1);
	}

	directory = ".";
	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'd':
		      directory = argv[2];
		      argc-=2;
		      argv+=2;
		      break;
	      default:
		      printf("Usage: %s [-d directory] (.ogd|.lst ...)\n", argv[0]);
	      }
	}

	hashtable_init(&traces, traceTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			printf("Loading %s file ...\n", argv[1]);
			load_source_file(fsource, NULL, &traces, (void*(*)(int, FILE*, struct Region *, struct Hashtable *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	printf("Altering the heading in old version ogd files & dumping...\n");
	for (i = 0; i<traces.size; i++) {
		aItem = traces.head[i];
		while(aItem != NULL ) {
			  aTrace = (struct Trace*)aItem->datap;
			  bItem = aTrace->reports.head;
			  while(bItem!=NULL) {
				  aRep = (struct Report*)bItem->datap;
				  aRep->angle = normal_angle_taxi(aRep->angle);
				  bItem = bItem->next;
			  }

			  sprintf(dumpfile, "%s/Taxi_%s.ogd", directory, aTrace->vName);
			  if( (fdump = fopen(dumpfile, "w"))!=NULL) {
				  trace_dump_func(fdump, aTrace);
				  fflush(fdump);
				  fclose(fdump);
			  }
			  aItem = aItem->next;
		}
	}
	 
	hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
	return 0;
}
