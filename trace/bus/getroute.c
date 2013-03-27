#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "busroute.h"
#include "common.h"
#include "files.h"
#include "trace.h"
#include "geometry.h"

int main(int argc, char **argv)
{
	FILE *fsource=NULL, *fdump=NULL;
	struct Region *region;

	struct Hashtable traces, routes;
	long traceTableSize = 2000, routeTableSize = 500;
	struct Item *aItem, *bItem;
	struct Trace *aTrace;
	struct Busroute *aRoute, *bRoute;
	struct Road *aRoad;
	int first;

	unsigned long i;
	char *routeid = NULL, *routeId;
	char outfile[64];

	struct Votetree upVotetree, downVotetree;


	if(argc < 3) {
	      printf("%s is used to determine bus routes from bus GPS reports.\n", argv[0]);
	      printf("Usage: %s [-r routeid] .map (.ogd|.mgd|.lst ...)\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'r':
			routeid = argv[2];
			argc-=2;
			argv+=2;
			break;
		default:
	      		printf("Usage: %s [-r routeid] .map (.ogd|.mgd|.lst ...)\n", argv[0]);
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
			load_source_file(fsource, region, &traces, (void*(*)(int, FILE*, struct Region *, void*, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	if(region!=NULL && traces.count != 0) {
		hashtable_init(&routes, routeTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);

		while(traces.count) {
			first = 1;
			for (i = 0; i<traces.size; i++) {
			      aItem = traces.head[i];
			      while(aItem != NULL ) {
					aTrace = (struct Trace*)aItem->datap;
					if(first) {
						first = 0;
						if(routeid == NULL || strcmp(routeid, aTrace->onRoute)==0) {
							routeId = (char*)malloc(sizeof(char)*NAME_LENGTH);
							strncpy(routeId, aTrace->onRoute, NAME_LENGTH);
						}	
						aRoute = (struct Busroute*)malloc(sizeof(struct Busroute));
						route_init_func(aRoute);
						sprintf(aRoute->name, "%s_upway", aTrace->onRoute);
						hashtable_add(&routes, aRoute->name, aRoute);

						bRoute = (struct Busroute*)malloc(sizeof(struct Busroute));
						route_init_func(bRoute);
						sprintf(bRoute->name, "%s_downway", aTrace->onRoute);
						hashtable_add(&routes, bRoute->name, bRoute);
					
						votetree_init_func(&upVotetree);	
						votetree_init_func(&downVotetree);
					}

					if (strcmp(routeId, aTrace->onRoute)==0) {
						vote_for_route(aTrace, region, &upVotetree, &downVotetree);
						aItem = aItem->next;
						trace_free_func((struct Trace*)hashtable_pick(&traces, aTrace->vName));
					} else 
						aItem = aItem->next;
				}
			}
			determine_route_path(&upVotetree); 
			if(upVotetree.bestCandPath.path) {
				aRoute->path = path_copy_func(upVotetree.bestCandPath.path);
				aRoute->path->length = distance_on_path(aRoute->path, NULL, NULL);
				aRoute->path->turns = turns_on_path(aRoute->path);
				bItem = aRoute->path->roads.head;
				while(bItem) {
					aRoad = (struct Road*)bItem->datap;
					if(bItem == aRoute->path->roads.head) {
						aRoute->box.xmin = aRoad->box.xmin;
						aRoute->box.xmax = aRoad->box.xmax;
						aRoute->box.ymin = aRoad->box.ymin;
						aRoute->box.ymax = aRoad->box.ymax;
					} else {
						if(aRoad->box.xmin < aRoute->box.xmin)
							aRoute->box.xmin = aRoad->box.xmin;
						if(aRoad->box.xmax > aRoute->box.xmax)
							aRoute->box.xmax = aRoad->box.xmax;
						if(aRoad->box.ymin < aRoute->box.ymin)
							aRoute->box.ymin = aRoad->box.ymin;
						if(aRoad->box.ymax > aRoute->box.ymax)
							aRoute->box.ymax = aRoad->box.ymax;
							
					}
					bItem = bItem->next;
				}
			} else {
				route_free_func(hashtable_pick(&routes, aRoute->name));
			}
			votetree_free_func(&upVotetree);

			determine_route_path(&downVotetree); 
			if(downVotetree.bestCandPath.path) {
				bRoute->path = path_copy_func(downVotetree.bestCandPath.path);
				bRoute->path->length = distance_on_path(bRoute->path, NULL, NULL);
				bRoute->path->turns = turns_on_path(bRoute->path);
				bItem = bRoute->path->roads.head;
				while(bItem) {
					aRoad = (struct Road*)bItem->datap;
					if(bItem == bRoute->path->roads.head) {
						bRoute->box.xmin = aRoad->box.xmin;
						bRoute->box.xmax = aRoad->box.xmax;
						bRoute->box.ymin = aRoad->box.ymin;
						bRoute->box.ymax = aRoad->box.ymax;
					} else {
						if(aRoad->box.xmin < bRoute->box.xmin)
							bRoute->box.xmin = aRoad->box.xmin;
						if(aRoad->box.xmax > bRoute->box.xmax)
							bRoute->box.xmax = aRoad->box.xmax;
						if(aRoad->box.ymin < bRoute->box.ymin)
							bRoute->box.ymin = aRoad->box.ymin;
						if(aRoad->box.ymax > bRoute->box.ymax)
							bRoute->box.ymax = aRoad->box.ymax;
							
					}
					bItem = bItem->next;
				}
			} else {
				route_free_func(hashtable_pick(&routes, bRoute->name));
			}
			votetree_free_func(&downVotetree);
		}
	}

	if(routes.count != 0) {
		for (i = 0; i<routes.size; i++) {
		     	aItem = routes.head[i];
		     	while(aItem != NULL ) {
				aRoute = (struct Busroute*)aItem->datap;
				sprintf(outfile, "%s.bus", aRoute->name);
				if( (fdump = fopen(outfile, "w"))!=NULL) {
					dump_route(fdump, aRoute);
					fflush(fdump);
					fclose(fdump);
				}
				aItem = aItem->next;
			}
		}
	}

	hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
	hashtable_destroy(&routes, (void(*)(void*))route_free_func);
	region_free_func(region);
	return 0;
}
