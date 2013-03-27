#include<stdlib.h>
#include<stdio.h>
#include"common.h"
#include"geometry.h"


int main(int argc, char **argv)
{
	FILE *fsource=NULL;
	struct Region *region;
	double area;
	struct Cell *aCell;

	unsigned long i, j;

	if(argc < 2) {
	      printf("%s is used to measure the area of the given map.\n", argv[0]);
	      printf("Usage: %s .map\n", argv[0]);
	      exit(1);
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, 10, -1);
	      fclose(fsource);
	}

	area = 0;
	for(i = 0; i<region->hCells; i++)
		for(j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if(is_cell_in_polygon(aCell, region->chosen_polygon))
				area += 100;
		}
	printf("Area is %.0lf m^2.\n", area);	
	return 0;
}

