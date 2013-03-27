#include<stdio.h>
#include<stdlib.h>
#include"files.h"
#include"traj.h"

int main(int argc, char **argv)
{
	FILE *fsource;
	struct Region *region;
	struct Region *rsuRegion;

	struct Hashtable traces;
	long traceTableSize = 2000;
	struct Item *aItem;
	struct Trace *aTrace;

	int i;


	if(argc < 4) {
	      printf("%s is used to log trajectory of a trace.\n", argv[0]);
	      printf("Usage: %s .map .vmap (.mgd | .lst ...)\n", argv[0]);
	      exit(1);
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, -1, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}

	rsuRegion = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      rsuRegion = region_load_func(fsource, NULL, -1, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}
	hashtable_init(&traces, traceTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			printf("Loading %s file ...\n", argv[1]);
			load_source_file(fsource, region, &traces, (void*(*)(int, FILE*, struct Region *, void*, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL,NULL,NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	if(region!=NULL && rsuRegion!=NULL && traces.count != 0) {
	      for (i = 0; i<traces.size; i++) {
		      aItem = traces.head[i];
		      while(aItem != NULL ) {
				aTrace = (struct Trace*)aItem->datap;
				log_trajectory(aTrace, region, rsuRegion);
			      	aItem = aItem->next;
		      }
	      }
	} 
	 
	hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
	region_free_func(region);
	region_free_func(rsuRegion);
	return 0;
}




