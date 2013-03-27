#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "common.h" 
#include "geometry.h"


int main(int argc, char **argv) 
{
  double scrapLength = -1;
  double cellSize = -1;
  char *regnPoints = NULL;

  char *districtFile = NULL;
  char *riverFile = NULL;
  char *roadFile = NULL;
  char *roadAttrFile = NULL;

  char *smapfile = NULL;
  char *omapfile = NULL;

  char *editfile = NULL;

  FILE *fDistrict, *fRiver, *fRoad;
  FILE *fTmp = NULL;
  FILE *fOutput;

  struct Polygon *chosen_polygon = NULL;
  struct Region *region = NULL;

  struct Point point;

  if(argc < 2) {
	printf("Usage: %s [-r \"x1 y1 x2 y2 x3 y3...\"] [-j scrapLength length]  [-c cell size] [-s districts.shp rivers.shp roads.shp roads.txt | -m source.map] [-e editmap.txt] output.map\n", argv[0]);
	exit(1);
  }

  while(argc>1 && (argv[1][0])=='-' ) {
    switch ( argv[1][1]) {
      case 'r':
	regnPoints = argv[2];
        argc-=2;
        argv+=2;
        break;

      case 'j':
        scrapLength = atof(argv[2]);
        argc-=2;
        argv+=2;
        break;

      case 'c':
        cellSize = atof(argv[2]);
        argc-=2;
        argv+=2;
        break;

      case 's':
	districtFile = argv[2];
	riverFile = argv[3];
	roadFile = argv[4];
	roadAttrFile = argv[5];
	argc-=5;
	argv+=5;
        break;

      case 'm':
	smapfile = argv[2];
        argc-=2;
        argv+=2;
        break;

      case 'e':
	editfile = argv[2];
        argc-=2;
        argv+=2;
        break;

      default:
        printf("Bad option %s\n", argv[1]);
	printf("Usage: %s [-r \"x1 y1 x2 y2 x3 y3...\"] [-j scrapLength length]  [-c cell size] [-s districts.shp rivers.shp roads.shp roads.txt | -m source.map] [-e editmap.txt] output.map\n", argv[0]);
        exit(1);
      }
  }

  if(argc > 1) 
	omapfile = argv[1];
  else
	omapfile = "default.map";

  if(regnPoints != NULL) {
	double x, y;
	char *px, *py;

	px = strtok(regnPoints, " ");
	py = strtok(NULL, " ");
	x = atof(px);
	y = atof(py);
	point.x = x, point.y = y;
	build_polygon(&chosen_polygon, &point);
	while(1) {	
		px = strtok(NULL, " ");
		if(px==NULL) break;
		py = strtok(NULL, " ");
		x = atof(px);
		y = atof(py);
		point.x = x, point.y = y;
		build_polygon(&chosen_polygon, &point);
	}
	close_polygon(chosen_polygon);
  }

  if(districtFile != NULL || riverFile != NULL || roadFile != NULL) {

	if((fRoad=fopen(roadFile, "r"))!=NULL) {
	      fTmp = fRoad;
	} else if((fDistrict=fopen(districtFile, "r"))!=NULL){
	      fTmp = fDistrict;
	} else if((fRiver=fopen(riverFile, "r"))!=NULL)  {
	      fTmp = fRiver;
	}

	if(chosen_polygon==NULL && fTmp != NULL) {
	      double xmin, xmax, ymin, ymax;
	      fseek(fTmp, 36, SEEK_SET);
	      fread(&xmin, sizeof(double), 1, fTmp);
	      fread(&ymin, sizeof(double), 1, fTmp);
	      fread(&xmax, sizeof(double), 1, fTmp);
	      fread(&ymax, sizeof(double), 1, fTmp);
	      fclose(fTmp);
	      point.x = xmin, point.y = ymin;
	      build_polygon(&chosen_polygon, &point);
	      point.x = xmin, point.y = ymax;
	      build_polygon(&chosen_polygon, &point);
	      point.x = xmax, point.y = ymax;
	      build_polygon(&chosen_polygon, &point);
	      point.x = xmax, point.y = ymin;
	      build_polygon(&chosen_polygon, &point);
	      close_polygon(chosen_polygon);
	}
	region = build_geographical_region(districtFile, riverFile, roadFile, roadAttrFile, chosen_polygon, cellSize==-1?DEFAULT_CELLSIZE:cellSize, scrapLength==-1?DEFAULT_SCRAPLENGTH:scrapLength);

  } else if (smapfile != NULL) {
  	if((fTmp=fopen(smapfile, "rb"))!=NULL) {
		region = region_load_func(fTmp, chosen_polygon, cellSize, scrapLength);
		fclose(fTmp);
		if(region == NULL) {
			printf("Load .map failed.\n");
			return -2;
		}
	}
  }

  if(editfile != NULL && region != NULL) {
	if((fTmp = fopen(editfile, "r"))!=NULL) {
		edit_region(fTmp, region);
		check_max_degree(region); 
		fclose(fTmp);
	}
  } 

  if(region) {
	printf("Dumping region to .map file.\n");
	if((fOutput=fopen(omapfile, "wb"))== NULL) {
	      printf("Cannot open file %s to write!\n", omapfile);
	      return -1;
	}
	region_dump_func(fOutput, region);
	printf("Summary: #crosses: %6ld, #roads: %6ld, maxdgr: %2d\n", region->crosses.nItems, region->roads.nItems, region->maxdgr);
	region_free_func(region);
	fclose(fOutput);

  } else {
	printf("Got a NULL region. No .map file is created.\n");
  }

  return 0;
}
