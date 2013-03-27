#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include"common.h"
#include"files.h"
#include"geometry.h"
#include"trace.h"
#include"busroute.h"

int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	struct Region *region, *aRegion;
	int insertMode = INSERT_MODE_AVGSPEED;
	int outputMode = OUTPUT_MODE_CELL;
	int interval;
	char dumpfile[256], *directory, buf[32];

	struct Hashtable traces, routes;
	struct Item *aItem, *bItem, *tItem;
	struct Trace *aTrace, *bTrace;
	unsigned long i;
	struct Duallist roads;
	struct Busroute *aRoute;

	if(argc < 3) {
		printf("%s is used to retrieve required spots of vehicles from mgd files. New GPS reports are generated according to the given model.\n", argv[0]);
	      	printf("Usage: %s [-insert avgspeed|traffic] [-output cell|interval seconds] [-d directory] .map (.mgd|.lst ...)\n", argv[0]);
	      	exit(1);
	}

	directory = ".";
	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'i':
		      if(strcmp(argv[2], "traffic")==0)
			  insertMode = INSERT_MODE_TRAFFIC;
		      else if(strcmp(argv[2], "avgspeed")==0)
			  insertMode = INSERT_MODE_AVGSPEED;
		      argc-=2;
		      argv+=2;
		      break;
	      case 'd':
		      directory = argv[2];
		      argc-=2;
		      argv+=2;
		      break;

	      case 'o':
		      if(argv[2][0]=='c') {
			      outputMode = OUTPUT_MODE_CELL;
			      argc-=2;
			      argv+=2;
		      } else if(argv[2][0] == 'i') {
			      outputMode = OUTPUT_MODE_INTERVAL;
			      interval = atoi(argv[3]);
			      argc-=3;
			      argv+=3;
		      }
		      break;

	      default:
	              printf("Usage: %s [-mode avgspeed|traffic] [-cell] [-interval seconds] .map (.mgd|.lst ...)\n", argv[0]);
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

				if(aTrace->type == FILE_MODIFIED_GPS_BUS && routes.count) {
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
		 				bTrace = insert_reports(aRegion, aTrace, insertMode, outputMode, interval);
						region_free_func(aRegion);
					}
					duallist_destroy(&roads, NULL);

				} else { 
		 			bTrace = insert_reports(region, aTrace, insertMode, outputMode, interval);
				}

				if(bTrace) {
					mkdir(directory,0777);
					sprintf(dumpfile, "%s/%s.mgd", directory, bTrace->vName);
					if( (fdump = fopen(dumpfile, "w"))!=NULL) {
						trace_dump_func(fdump, bTrace);
						fflush(fdump);
						fclose(fdump);
					}
					trace_free_func(bTrace);
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

