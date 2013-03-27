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
	FILE *fsource=NULL;
	struct Region *region;
	struct Hashtable routes;
	long routeTableSize = 500;
	struct Cell *aCell;
	unsigned long i, j, count;
	int cellsize = 300;


	if(argc < 3) {
	      printf("%s is used to get the coverage of bus routes.\n", argv[0]);
	      printf("Usage: %s [-cell size] .map (.lst |.bus ...)\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'c':
			cellsize = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		default:
	      		printf("Usage: %s [-cell size] .map (.lst |.bus ...)\n", argv[0]);
	      }
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, cellsize, -1);
	      fclose(fsource);
	      argc--;
	      argv++;
	}

	hashtable_init(&routes, routeTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			load_source_file(fsource, region, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &routes, (void*(*)(FILE*, struct Region*, void *))load_route_with_hashtable, NULL, NULL, NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	count = 0;
	for(i = 0; i<region->hCells; i++)
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if(is_cell_in_polygon(aCell, region->chosen_polygon))
				count ++;
		}

	setup_cells_with_routes(region, &routes);
	printf("%d %.2lf\n", cellsize, ((double)region->busCoveredCells.nItems)/count);

	hashtable_destroy(&routes, (void(*)(void*))route_free_func);
	region_free_func(region);
	return 0;
}
