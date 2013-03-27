#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "contact.h"
#include "geometry.h"
#include "errors.h"
#include "storage.h"

/***********************************************************
 * Region structure related
 * ********************************************************/


struct Region* build_geographical_region(char *districtFile, char *riverFile, char *roadFile, char *roadAttrFile, struct Polygon *chosen_polygon, double cellSize, double scrapLength)
{

	FILE *froad, *froadAttr = NULL, *fdist, *friver;
	struct Region *region;

	region=(struct Region*)malloc(sizeof(struct Region));

	region->chosen_polygon = chosen_polygon;
	region->scrapLength = scrapLength;
	region->cellSize = distance_in_latitude(cellSize);
	setup_cells_in_region(region);

	duallist_init(&region->roads);
	duallist_init(&region->crosses);
	duallist_init(&region->districts);
	duallist_init(&region->rivers);
	region->maxdgr = 0;
	region->roadNums = 1;
	region->crossNums = 1;

	if((fdist=fopen(districtFile, "r"))!=NULL) {
		printf("Loading districts information...\n");
		read_all_district_records(fdist, region);
		fclose(fdist);
	}
	if((friver=fopen(riverFile, "r"))!=NULL) {
		printf("Loading rivers information...\n");
		read_all_river_records(friver, region);
		fclose(friver);
	}
	if((froad=fopen(roadFile, "r"))!=NULL) {
		printf("Loading roads information...\n");
		if(strcmp(roadAttrFile, "NULL")!=0 )
			froadAttr=fopen(roadAttrFile, "r");
		read_all_road_records(froad, froadAttr, region);
		fclose(froad);
	}
	return region;
}

void read_all_road_records(FILE *froad, FILE *froadAttr, struct Region *region)
{
	struct Polyline_record rcd;
	char buf[128], *strp;
	struct Box box;
	int aint, i, inRegion;

	fseek(froad, 100, SEEK_SET);
	if (froadAttr != NULL)
		fgets(buf, 128, froadAttr);
	while (fgetc(froad)!=EOF) {
		fseek(froad, -1, SEEK_CUR);
		fread(&aint, sizeof(int), 1, froad);
		rcd.header.number = big2little(aint);
		fread(&aint, sizeof(int), 1, froad);
		rcd.header.length = big2little(aint);
		fread(&rcd.type, sizeof(int), 1, froad);
		fread(rcd.apolyline.box, sizeof(double), 4, froad);
		fread(&rcd.apolyline.nParts, sizeof(int), 1, froad);
		fread(&rcd.apolyline.nPoints, sizeof(int), 1, froad);
		rcd.apolyline.parts = (int*)malloc(sizeof(int) * rcd.apolyline.nParts);
		fread(rcd.apolyline.parts, sizeof(int), rcd.apolyline.nParts, froad);
		rcd.apolyline.points = (struct Point*)malloc(sizeof(struct Point) * rcd.apolyline.nPoints);
		fread(rcd.apolyline.points, sizeof(struct Point), rcd.apolyline.nPoints, froad);

		rcd.lable[0] = '\0';
		rcd.width = ROAD_WIDTH;
		rcd.direction = DIRECTION_DUAL;
		if (froadAttr != NULL) {
			fgets(buf, 128, froadAttr);
			if(buf[0] != '\r') {
			//	strp = strtok(buf, "\t");
			//	strncpy(rcd.lable, strp, strlen(strp)+1);
				strp = strtok(buf, "\t");
			//	rcd.width = atoi(strp);
				strp = strtok(NULL, "\t");
				rcd.direction = atoi(strp);
			}
		}
  
		/* select a record whether within a given chosen_polygon */
		box.xmin = rcd.apolyline.box[0], box.ymin = rcd.apolyline.box[1];
		box.xmax = rcd.apolyline.box[2], box.ymax = rcd.apolyline.box[3];
		if(is_box_within_box(&box, &region->chosen_polygon->box)) {
			inRegion = 0;
			for(i = 0; i<rcd.apolyline.nPoints; i++) {
				if (is_point_in_polygon(rcd.apolyline.points+i, region->chosen_polygon)){
					inRegion = 1;
					break;
				}
			}
			if(inRegion){
				add_road_rcd_to_region(&rcd, region);
			}
		}
		free(rcd.apolyline.parts);
		free(rcd.apolyline.points);
	}
}


void add_road_rcd_to_region(struct Polyline_record *rcd, struct Region *region)
{

  	struct Road *aNewRoad, *bNewRoad;
	struct Point *newp;
	int i;

	if (rcd->direction == DIRECTION_FORWARD || rcd->direction == DIRECTION_REVERSE ) {
		aNewRoad = (struct Road*)malloc(sizeof(struct Road));
		road_init_func(aNewRoad);
		aNewRoad->id = region->roadNums++;
		aNewRoad->width = rcd->width;
		for(i = 0; i<rcd->apolyline.nPoints; i++) {
			newp = (struct Point*)malloc(sizeof(struct Point));
			newp->x = rcd->apolyline.points[i].x;
			newp->y = rcd->apolyline.points[i].y;
			if(rcd->direction == DIRECTION_FORWARD) {
				if(aNewRoad->origPoints.nItems == 0 || (aNewRoad->origPoints.nItems > 0 && !point_equal_func(newp, (struct Point*)aNewRoad->origPoints.head->prev->datap)))
					duallist_add_to_tail(&aNewRoad->origPoints, newp);
				else
					free(newp);
			} else {
				if(aNewRoad->origPoints.nItems == 0 || (aNewRoad->origPoints.nItems > 0 && !point_equal_func(newp, (struct Point*)aNewRoad->origPoints.head->datap)))
					duallist_add_to_head(&aNewRoad->origPoints, newp);
				else
					free(newp);
			}
		}
		duallist_copy(&aNewRoad->points, &aNewRoad->origPoints, (void*(*)(void*))point_copy_func);
		duallist_add_to_tail(&(region->roads), aNewRoad);
		setup_road(aNewRoad, region);
	} else {
		aNewRoad = (struct Road*)malloc(sizeof(struct Road));
		road_init_func(aNewRoad);
		aNewRoad->id = region->roadNums++;
		aNewRoad->width = rcd->width;
		
		for(i = 0; i<rcd->apolyline.nPoints; i++) {
			newp = (struct Point*)malloc(sizeof(struct Point));
			newp->x = rcd->apolyline.points[i].x;
			newp->y = rcd->apolyline.points[i].y;
			if(aNewRoad->origPoints.nItems == 0 || (aNewRoad->origPoints.nItems > 0 && !point_equal_func(newp, (struct Point*)aNewRoad->origPoints.head->prev->datap)))
				duallist_add_to_tail(&aNewRoad->origPoints, newp);
			else
				free(newp);
		}
		offset_right_road(&aNewRoad->origPoints, aNewRoad->width/2, &aNewRoad->points);
		duallist_add_to_tail(&(region->roads), aNewRoad);
		setup_road(aNewRoad, region);

		/*reverse direction */
		bNewRoad = (struct Road*)malloc(sizeof(struct Road));
		road_init_func(bNewRoad);
		bNewRoad->id = region->roadNums++;
		bNewRoad->width = rcd->width;
		for(i = 0; i<rcd->apolyline.nPoints; i++) {
			newp = (struct Point*)malloc(sizeof(struct Point));
			newp->x = rcd->apolyline.points[i].x;
			newp->y = rcd->apolyline.points[i].y;
			if(bNewRoad->origPoints.nItems == 0 || (bNewRoad->origPoints.nItems > 0 && !point_equal_func(newp, (struct Point*)bNewRoad->origPoints.head->datap)))
				duallist_add_to_head(&bNewRoad->origPoints, newp);
			else
				free(newp);
		}
		offset_right_road(&bNewRoad->origPoints, bNewRoad->width/2, &bNewRoad->points);
		duallist_add_to_tail(&(region->roads), bNewRoad);
		setup_road(bNewRoad, region);
	}
}


void read_all_river_records(FILE *friver, struct Region *region)
{
	struct PolygonM_record pgmrcd;
	struct Box box;
	int aint;

	fseek(friver, 100, SEEK_SET);
	while (fgetc(friver)!=EOF)
	{
		fseek(friver, -1, SEEK_CUR);
		fread(&aint, sizeof(int), 1, friver);
		pgmrcd.header.number = big2little(aint);
		fread(&aint, sizeof(int), 1, friver);
		pgmrcd.header.length = big2little(aint);
		fread(&pgmrcd.type, sizeof(int), 1, friver);
		fread(pgmrcd.apolygonM.box, sizeof(double), 4, friver);
		fread(&pgmrcd.apolygonM.nParts, sizeof(int), 1, friver);
		fread(&pgmrcd.apolygonM.nPoints, sizeof(int), 1, friver);
		pgmrcd.apolygonM.parts = (int*)malloc(sizeof(int) * pgmrcd.apolygonM.nParts);
		fread(pgmrcd.apolygonM.parts, sizeof(int), pgmrcd.apolygonM.nParts, friver);
		pgmrcd.apolygonM.points = (struct Point*)malloc(sizeof(struct Point) * pgmrcd.apolygonM.nPoints);
		fread(pgmrcd.apolygonM.points, sizeof(struct Point), pgmrcd.apolygonM.nPoints, friver);
		fread(pgmrcd.apolygonM.MRange, sizeof(double), 2, friver);
		pgmrcd.apolygonM.measures =(double *) malloc(sizeof(double)*pgmrcd.apolygonM.nPoints);
		fread(pgmrcd.apolygonM.measures, sizeof(double), pgmrcd.apolygonM.nPoints, friver);

		box.xmin = pgmrcd.apolygonM.box[0], box.ymin = pgmrcd.apolygonM.box[1];
		box.xmax = pgmrcd.apolygonM.box[2], box.ymax = pgmrcd.apolygonM.box[3];
		if(are_boxes_intersected(&box, &region->chosen_polygon->box))
			add_river_rcd_to_region(&pgmrcd, region);
		free(pgmrcd.apolygonM.parts);
		free(pgmrcd.apolygonM.points);
		free(pgmrcd.apolygonM.measures);
	}
}


void read_all_district_records(FILE *fdist, struct Region *region)
{
	struct Polygon_record pgrcd;
	struct Box box;
	int aint;

	fseek(fdist, 100, SEEK_SET);
	while (fgetc(fdist)!=EOF)
	{
		fseek(fdist, -1, SEEK_CUR);
		fread(&aint, sizeof(int), 1, fdist);
		pgrcd.header.number = big2little(aint);
		fread(&aint, sizeof(int), 1, fdist);
		pgrcd.header.length = big2little(aint);
		fread(&pgrcd.type, sizeof(int), 1, fdist);
		fread(pgrcd.apolygon.box, sizeof(double), 4, fdist);
		fread(&pgrcd.apolygon.nParts, sizeof(int), 1, fdist);
		fread(&pgrcd.apolygon.nPoints, sizeof(int), 1, fdist);
		pgrcd.apolygon.parts = (int*)malloc(sizeof(int) * pgrcd.apolygon.nParts);
		fread(pgrcd.apolygon.parts, sizeof(int), pgrcd.apolygon.nParts, fdist);
		pgrcd.apolygon.points = (struct Point*)malloc(sizeof(struct Point) * pgrcd.apolygon.nPoints);
		fread(pgrcd.apolygon.points, sizeof(struct Point), pgrcd.apolygon.nPoints, fdist);
		box.xmin = pgrcd.apolygon.box[0], box.ymin = pgrcd.apolygon.box[1];
		box.xmax = pgrcd.apolygon.box[2], box.ymax = pgrcd.apolygon.box[3];
		if(are_boxes_intersected(&box, &region->chosen_polygon->box))
			add_district_rcd_to_region(&pgrcd, region);
		free(pgrcd.apolygon.parts);
		free(pgrcd.apolygon.points);

	}
}


void add_river_rcd_to_region(struct PolygonM_record *pgmrcd, struct Region *region)
{
  struct Point *point;
  struct Duallist *list;
  struct River *newRiver;
  int i, j;
  
  newRiver = (struct River*)malloc(sizeof(struct River));
  newRiver->box.xmin = pgmrcd->apolygonM.box[0];
  newRiver->box.ymin = pgmrcd->apolygonM.box[1];
  newRiver->box.xmax = pgmrcd->apolygonM.box[2];
  newRiver->box.ymax = pgmrcd->apolygonM.box[3];
  duallist_init(&(newRiver->rings));
  duallist_add_to_head(&(region->rivers), newRiver);

  for(i =0; i<pgmrcd->apolygonM.nParts-1; i++) {
	list=(struct Duallist*)malloc(sizeof(struct Duallist));
	duallist_init(list);
	for (j = pgmrcd->apolygonM.parts[i]; j < pgmrcd->apolygonM.parts[i+1]-1; j++){
		point=(struct Point*)malloc(sizeof(struct Point));
		point->x = pgmrcd->apolygonM.points[j].x;
		point->y = pgmrcd->apolygonM.points[j].y;
		duallist_add_to_tail(list, point);
	}
	duallist_add_to_tail(&(newRiver->rings), list);
  }
  list=(struct Duallist*)malloc(sizeof(struct Duallist));
  duallist_init(list);
  for (j = pgmrcd->apolygonM.parts[i]; j < pgmrcd->apolygonM.nPoints-1; j++) {
	point=(struct Point*)malloc(sizeof(struct Point));
	point->x = pgmrcd->apolygonM.points[j].x;
	point->y = pgmrcd->apolygonM.points[j].y;
	duallist_add_to_tail(list, point);
  }
  duallist_add_to_tail(&(newRiver->rings), list);
}


void add_district_rcd_to_region(struct Polygon_record *pgrcd, struct Region *region)
{
  struct Point *point;
  struct Duallist *list;
  struct District *newDist;
  int i, j;

  newDist = (struct District*)malloc(sizeof(struct District));
  newDist->box.xmin = pgrcd->apolygon.box[0];
  newDist->box.ymin = pgrcd->apolygon.box[1];
  newDist->box.xmax = pgrcd->apolygon.box[2];
  newDist->box.ymax = pgrcd->apolygon.box[3];
  duallist_init(&(newDist->rings));
  duallist_add_to_head(&(region->districts), newDist);

  for(i =0; i<pgrcd->apolygon.nParts-1; i++) {
	list=(struct Duallist*)malloc(sizeof(struct Duallist));
	duallist_init(list);
	for (j = pgrcd->apolygon.parts[i]; j < pgrcd->apolygon.parts[i+1]-1; j++){
		point=(struct Point*)malloc(sizeof(struct Point));
		point->x = pgrcd->apolygon.points[j].x;
		point->y = pgrcd->apolygon.points[j].y;
		duallist_add_to_tail(list, point);
	}
	duallist_add_to_tail(&(newDist->rings), list);
  }
  list=(struct Duallist*)malloc(sizeof(struct Duallist));
  duallist_init(list);
  for (j = pgrcd->apolygon.parts[i]; j < pgrcd->apolygon.nPoints-1; j++) {
	point=(struct Point*)malloc(sizeof(struct Point));
	point->x = pgrcd->apolygon.points[j].x;
	point->y = pgrcd->apolygon.points[j].y;
	duallist_add_to_tail(list, point);
  }
  duallist_add_to_tail(&(newDist->rings), list);
}

/*
 * create a region structure that contains existing roads 
 * and associated crosses.
 *
 */
struct Region* build_region_with_roads(struct Duallist *roads)
{
	struct Region *region;
	struct Point point;
	double xmin, xmax, ymin, ymax;
  	struct Polygon *chosen_polygon = NULL;
	struct Item *aItem, *tItem;
	struct Road *aRoad;
	struct Cross *headEnd, *tailEnd;
	int first;

	if(roads==NULL || (roads && roads->nItems == 0))
		return NULL;
	
	region=(struct Region*)malloc(sizeof(struct Region));
	duallist_init(&region->roads);
	duallist_init(&region->crosses);
	duallist_init(&region->districts);
	duallist_init(&region->rivers);
	region->maxdgr = 0;
	region->roadNums = 1;
	region->crossNums = 1;
	region->scrapLength = DEFAULT_SCRAPLENGTH;
	region->cellSize = distance_in_latitude(DEFAULT_CELLSIZE);


	first = 1;
	aItem = roads->head;
	while(aItem) {
		aRoad = (struct Road*)aItem->datap;
		if(first) {
			first = 0;
			xmin = aRoad->box.xmin;
			xmax = aRoad->box.xmax;
			ymin = aRoad->box.ymin;
			ymax = aRoad->box.ymax;
		} else {
			xmin = MIN(xmin, aRoad->box.xmin);
			xmax = MAX(xmax, aRoad->box.xmax);
			ymin = MIN(ymin, aRoad->box.ymin);
			ymax = MAX(ymax, aRoad->box.ymax);
		}
		headEnd = aRoad->headEnd;
		tailEnd = aRoad->tailEnd;
		tItem = duallist_find(&region->roads, &aRoad->id, (int(*)(void*, void*))road_has_id);
		if(!tItem)
			duallist_add_to_tail(&region->roads, road_copy_func(aRoad));

		tItem = duallist_find(&region->crosses, &headEnd->number, (int(*)(void*,void*))cross_has_number);
		if(!tItem) {
			xmin = MIN(xmin, headEnd->gPoint.x);
			xmax = MAX(xmax, headEnd->gPoint.x);
			ymin = MIN(ymin, headEnd->gPoint.y);
			ymax = MAX(ymax, headEnd->gPoint.y);
			duallist_add_to_tail(&region->crosses, cross_copy_func(headEnd));
		}
		tItem = duallist_find(&region->crosses, &tailEnd->number, (int(*)(void*,void*))cross_has_number);
		if(!tItem) {
			xmin = MIN(xmin, tailEnd->gPoint.x);
			xmax = MAX(xmax, tailEnd->gPoint.x);
			ymin = MIN(ymin, tailEnd->gPoint.y);
			ymax = MAX(ymax, tailEnd->gPoint.y);
			duallist_add_to_tail(&region->crosses, cross_copy_func(tailEnd));
		}
		aItem = aItem->next;
	}
	point.x = xmin, point.y = ymin;
	build_polygon(&chosen_polygon, &point);
	point.x = xmin, point.y = ymax;
	build_polygon(&chosen_polygon, &point);
	point.x = xmax, point.y = ymax;
	build_polygon(&chosen_polygon, &point);
	point.x = xmax, point.y = ymin;
	build_polygon(&chosen_polygon, &point);
	close_polygon(chosen_polygon);
	region->chosen_polygon = chosen_polygon;

	setup_cells_in_region(region);
	setup_roads_and_crosses_in_region(region);
	return region;
}

/*
 * load region from .map file
 */
struct Region* region_load_func(FILE *fInput, struct Polygon *chosen_polygon, double cellSize, double scrapLength)
{
	struct Region* region;

	region=(struct Region*)malloc(sizeof(struct Region));

	fread(&region->scrapLength, sizeof(double), 1, fInput);
	if(scrapLength != -1)
		region->scrapLength = scrapLength;
		
	fread(&region->cellSize, sizeof(double),1, fInput);
	if(cellSize != -1) 
		region->cellSize = distance_in_latitude(cellSize);

	fread(&region->maxdgr,sizeof(int), 1, fInput);
	fread(&region->roadNums,sizeof(int), 1, fInput);
	fread(&region->crossNums,sizeof(int), 1, fInput);

	region->chosen_polygon = polygon_load_func(fInput);
	if(chosen_polygon) {
		polygon_free_func(region->chosen_polygon);
		region->chosen_polygon = chosen_polygon;
	}


	duallist_load(fInput, &region->roads, (void*(*)(FILE*))road_load_func);
	duallist_load(fInput, &region->crosses, (void*(*)(FILE*))cross_load_func);
	duallist_load(fInput, &region->districts, (void*(*)(FILE*))district_load_func);
	duallist_load(fInput, &region->rivers, (void*(*)(FILE*))river_load_func);

	setup_cells_in_region(region);
	setup_roads_and_crosses_in_region(region);

	return region;
}


/**********************************************
Map edit file format:
add road direction[DIRECTION_FORWARD|DIRECTION_REVERSE|DIRECTION_DUAL] points x1 y1 x2 y2 ...
add road direction crossIds c1 c2
cut road roadId x y
del road roadId
del cross crossId
**********************************************/

void edit_region(FILE *fedit, struct Region *aRegion)
{
	char buf[2048], *action, *on, *direction, *type, *strp;
	int dir;
	struct Road *aNewRoad, *bNewRoad, *aRoad;
	struct Item *aItem, *bItem;
	struct Cross *aCross, *bCross;
	int aCrossId, bCrossId;
	double x, y;
	char *px, *py;
	struct Point *newp, aPoint;
	int id;
	
	while(fgets(buf, 2048, fedit)) {
		action = strtok(buf, " ");
		on = strtok(NULL, " ");
		if( action[0] == 'a') {
			direction = strtok(NULL, " ");
			dir = atoi(direction);

			type = strtok(NULL, " ");

			if(type[0]=='p') {
				if ( dir == DIRECTION_FORWARD || dir == DIRECTION_REVERSE ) {
					aNewRoad = (struct Road*)malloc(sizeof(struct Road));
					road_init_func(aNewRoad);

					px = strtok(NULL, " ");
					py = strtok(NULL, " ");
					x = atof(px);
					y = atof(py);
					newp = (struct Point*)malloc(sizeof(struct Point));
					newp->x = x, newp->y = y;
					if(dir == DIRECTION_FORWARD) {
						duallist_add_to_tail(&aNewRoad->origPoints, newp);
					} else {
						duallist_add_to_head(&aNewRoad->origPoints, newp);
					}
					while(1) {	
						px = strtok(NULL, " \n\t");
						if(px==NULL) break;
						py = strtok(NULL, " \n\t");
						x = atof(px);
						y = atof(py);
						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = x, newp->y = y;
						if(dir == DIRECTION_FORWARD) {
							duallist_add_to_tail(&aNewRoad->origPoints, newp);
						} else {
							duallist_add_to_head(&aNewRoad->origPoints, newp);
						}
					}
					offset_right_road(&aNewRoad->origPoints, aNewRoad->width/2, &aNewRoad->points);
					duallist_add_to_tail(&(aRegion->roads), aNewRoad);
					aNewRoad->id = aRegion->roadNums++;
					setup_road(aNewRoad, aRegion);
					printf("New road No.%d is added.\n", aNewRoad->id);
				} else {
					aNewRoad = (struct Road*)malloc(sizeof(struct Road));
					road_init_func(aNewRoad);

					px = strtok(NULL, " ");
					py = strtok(NULL, " ");
					x = atof(px);
					y = atof(py);
					newp = (struct Point*)malloc(sizeof(struct Point));
					newp->x = x, newp->y = y;
					duallist_add_to_tail(&aNewRoad->origPoints, newp);
					while(1) {	
						px = strtok(NULL, " ");
						if(px==NULL) break;
						py = strtok(NULL, " ");
						x = atof(px);
						y = atof(py);
						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = x, newp->y = y;
						duallist_add_to_tail(&aNewRoad->origPoints, newp);
					}
					
					offset_right_road(&aNewRoad->origPoints, aNewRoad->width/2, &aNewRoad->points);
					duallist_add_to_tail(&(aRegion->roads), aNewRoad);
					aNewRoad->id = aRegion->roadNums++;
					setup_road(aNewRoad, aRegion);
					printf("New road No.%d is added.\n", aNewRoad->id);

					/*reverse direction */
					bNewRoad = (struct Road*)malloc(sizeof(struct Road));
					road_init_func(bNewRoad);
					duallist_reverse_copy(&bNewRoad->origPoints, &aNewRoad->origPoints, (void*(*)(void*))point_copy_func);
					offset_right_road(&bNewRoad->origPoints, bNewRoad->width/2, &bNewRoad->points);
					duallist_add_to_tail(&(aRegion->roads), bNewRoad);
					bNewRoad->id = aRegion->roadNums++;
					setup_road(bNewRoad, aRegion);
					printf("New road No.%d is added.\n", bNewRoad->id);
				}

			} else if(type[0]=='c') {
				px = strtok(NULL, " ");
				py = strtok(NULL, " ");
				aCrossId = atoi(px);
				bCrossId = atoi(py);
				aItem = duallist_find(&aRegion->crosses, &aCrossId, (int(*)(void*,void*))cross_has_number);
				bItem = duallist_find(&aRegion->crosses, &bCrossId, (int(*)(void*,void*))cross_has_number);
				if(aItem!=NULL && bItem!=NULL) {
					aCross = (struct Cross*)aItem->datap;
					bCross = (struct Cross*)bItem->datap;
					if (dir == DIRECTION_FORWARD || dir == DIRECTION_REVERSE ) {
						aNewRoad = (struct Road*)malloc(sizeof(struct Road));
						road_init_func(aNewRoad);

						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = aCross->gPoint.x, newp->y = aCross->gPoint.y;
						duallist_add_to_tail(&aNewRoad->origPoints, newp);

						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = bCross->gPoint.x, newp->y = bCross->gPoint.y;
						if(dir == DIRECTION_FORWARD) {
							duallist_add_to_tail(&aNewRoad->origPoints, newp);
						} else {
							duallist_add_to_head(&aNewRoad->origPoints, newp);
						}

						offset_right_road(&aNewRoad->origPoints, aNewRoad->width/2, &aNewRoad->points);
						duallist_add_to_tail(&(aRegion->roads), aNewRoad);
						aNewRoad->id = aRegion->roadNums++;
						setup_road(aNewRoad, aRegion);
						printf("New road No.%d is added.\n", aNewRoad->id);
					} else {
						aNewRoad = (struct Road*)malloc(sizeof(struct Road));
						road_init_func(aNewRoad);
						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = aCross->gPoint.x, newp->y = aCross->gPoint.y;
						duallist_add_to_tail(&aNewRoad->origPoints, newp);
						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = bCross->gPoint.x, newp->y = bCross->gPoint.y;
						duallist_add_to_tail(&aNewRoad->origPoints, newp);
						offset_right_road(&aNewRoad->origPoints, aNewRoad->width/2, &aNewRoad->points);
						duallist_add_to_tail(&(aRegion->roads), aNewRoad);
						aNewRoad->id = aRegion->roadNums++;
						setup_road(aNewRoad, aRegion);
						printf("New road No.%d is added.\n", aNewRoad->id);

						/*reverse direction */
						bNewRoad = (struct Road*)malloc(sizeof(struct Road));
						road_init_func(bNewRoad);
						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = aCross->gPoint.x, newp->y = aCross->gPoint.y;
						duallist_add_to_head(&bNewRoad->origPoints, newp);
						newp = (struct Point*)malloc(sizeof(struct Point));
						newp->x = bCross->gPoint.x, newp->y = bCross->gPoint.y;
						duallist_add_to_head(&bNewRoad->origPoints, newp);
						offset_right_road(&bNewRoad->origPoints, bNewRoad->width/2, &bNewRoad->points);
						duallist_add_to_tail(&(aRegion->roads), bNewRoad);
						bNewRoad->id = aRegion->roadNums++;
						setup_road(bNewRoad, aRegion);
						printf("New road No.%d is added.\n", bNewRoad->id);
					}
				} else {
					if(aItem == NULL)
						printf("No cross No.%d is found.\n", aCrossId);
					if(bItem == NULL)
						printf("No cross No.%d is found.\n", bCrossId);
				}

			}
			
		} else if (action[0] == 'd') {
			if(on[0]=='r') {
				strp = strtok(NULL, " ");
				id = atoi(strp);
				aRoad = (struct Road*)duallist_pick(&aRegion->roads, &id, (int(*)(void*,void*))road_has_id);
				if(aRoad) {
					aCross=aRoad->headEnd;
					bCross=aRoad->tailEnd;
					road_free_func(aRoad);
					if(aCross->inRoads.nItems + aCross->outRoads.nItems == 0) {
						duallist_pick(&aRegion->crosses, &aCross->number, (int(*)(void*,void*))cross_has_number);
						cross_free_func(aCross);
					}
					if(bCross->inRoads.nItems + bCross->outRoads.nItems == 0) {
						duallist_pick(&aRegion->crosses, &bCross->number, (int(*)(void*,void*))cross_has_number);
						cross_free_func(bCross);
					}
					printf("Road No.%d and related crosses are deleted.\n", id);
				} else {
					printf("No road No.%d is found.\n", id);
				}


			} else if(on[0]=='c') {
				strp = strtok(NULL, " ");
				id = atoi(strp);
				aCross = (struct Cross*)duallist_pick(&aRegion->crosses, &id, (int(*)(void*,void*))cross_has_number);

				if(aCross) {
					aItem = aCross->inRoads.head;
					while (aItem != NULL) {
						aRoad = (struct Road*) aItem->datap;
						bCross = aRoad->headEnd;
						duallist_pick(&aRegion->roads, &aRoad->id, (int(*)(void*,void*))road_has_id);
						road_free_func(aRoad);
						if(bCross->inRoads.nItems + aCross->outRoads.nItems == 0){
							duallist_pick(&aRegion->crosses, &bCross->number, (int(*)(void*,void*))cross_has_number);
							cross_free_func(bCross);
						}
			
						aItem = aItem->next;
					}	

					aItem = aCross->outRoads.head;
					while (aItem != NULL) {
						aRoad = (struct Road*) aItem->datap;
						bCross = aRoad->tailEnd;
						duallist_pick(&aRegion->roads, &aRoad->id, (int(*)(void*,void*))road_has_id);
						road_free_func(aRoad);
						if(bCross->inRoads.nItems + aCross->outRoads.nItems == 0){
							duallist_pick(&aRegion->crosses, &bCross->number, (int(*)(void*,void*))cross_has_number);
							cross_free_func(bCross);
						}
						aItem = aItem->next;
					}	
					
					cross_free_func(aCross);
					printf("Cross No.%d and all connected roads are deleted.\n", id);
				} else {
					printf("No cross No.%d is found.\n", id);
				}
			
			}
		} else if (action[0] == 'c') {
			strp = strtok(NULL, " ");
			id = atoi(strp);
			aRoad = (struct Road*)duallist_pick(&aRegion->roads, &id, (int(*)(void*,void*))road_has_id);
			if(aRoad) {
				px = strtok(NULL, " ");
				py = strtok(NULL, " ");
				aPoint.x = atof(px);
				aPoint.y = atof(py);
		
				aNewRoad = (struct Road*)malloc(sizeof(struct Road));
				road_init_func(aNewRoad);
				aNewRoad->id = aRoad->id;

				bNewRoad = (struct Road*)malloc(sizeof(struct Road));
				road_init_func(bNewRoad);
				bNewRoad->id = aRegion->roadNums++;
			
				point_cut_polyline(&aRoad->origPoints, &aPoint, &aNewRoad->origPoints, &bNewRoad->origPoints);	

				road_free_func(aRoad);

				offset_right_road(&aNewRoad->origPoints, aNewRoad->width/2, &aNewRoad->points);
				duallist_add_to_tail(&(aRegion->roads), aNewRoad);
				setup_road(aNewRoad, aRegion);

				offset_right_road(&bNewRoad->origPoints, bNewRoad->width/2, &bNewRoad->points);
				duallist_add_to_tail(&(aRegion->roads), bNewRoad);
				setup_road(bNewRoad, aRegion);
				printf("Original road No.%d is cut into two roads No.%d and No.%d.\n", aNewRoad->id, aNewRoad->id, bNewRoad->id);

			} else {
				printf("No road No.%d is found.\n", id);
			}

		}
	}
}


void check_max_degree(struct Region *region)
{
	struct Item *aItem;

	if(region) {
		region->maxdgr = 0;
		aItem = region->crosses.head;
		while(aItem) {		
			if(region->maxdgr < ((struct Cross*)aItem->datap)->outRoads.nItems + ((struct Cross*)aItem->datap)->inRoads.nItems)
				region->maxdgr = ((struct Cross*)aItem->datap)->outRoads.nItems+ ((struct Cross*)aItem->datap)->inRoads.nItems;
			aItem = aItem->next;
		}
	}
}


struct Region* region_copy_func(struct Region *aRegion)
{
	struct Region *rtRegion=NULL;

	if(aRegion!=NULL) {
		rtRegion = (struct Region*)malloc(sizeof(struct Region));
		rtRegion->cellSize = aRegion->cellSize;
		rtRegion->maxdgr = aRegion->maxdgr;
		rtRegion->roadNums = aRegion->roadNums;
		rtRegion->crossNums = aRegion->crossNums;
		rtRegion->chosen_polygon = polygon_copy_func(aRegion->chosen_polygon);

		duallist_copy(&rtRegion->roads, &aRegion->roads, (void*(*)(void*))road_copy_func);
		duallist_copy(&rtRegion->crosses, &aRegion->crosses, (void*(*)(void*))cross_copy_func);
		duallist_init(&rtRegion->districts);
		duallist_init(&rtRegion->rivers);

		setup_cells_in_region(rtRegion);
		setup_roads_and_crosses_in_region(rtRegion);
	}
	return rtRegion;
}

void setup_cells_in_region(struct Region *region)
{
	int i, j;
	struct Cell *aCell;

	if(region!=NULL && region->cellSize != 0) {
		region->vCells = floor((region->chosen_polygon->box.ymax - region->chosen_polygon->box.ymin)/region->cellSize) + 1;
		region->hCells = floor((region->chosen_polygon->box.xmax - region->chosen_polygon->box.xmin)/region->cellSize) + 1;
		region->mesh = (struct Cell*)malloc(sizeof(struct Cell) * region->vCells * region->hCells);
		region->map = (char*)malloc(sizeof(char)*region->vCells*region->hCells);
		for(i=0;i<region->vCells*region->hCells;i++) 
			*(region->map+i) = 0;
		for(i = 0; i<region->hCells; i++)
			for(j = 0; j<region->vCells;j++) {
				aCell = region->mesh + i*region->vCells + j;
				aCell->xNumber = i, aCell->yNumber = j;
				cell_init_func(aCell);
				aCell->box.xmin = region->chosen_polygon->box.xmin + i*region->cellSize, aCell->box.ymin = region->chosen_polygon->box.ymin + j*region->cellSize;
				aCell->box.xmax = region->chosen_polygon->box.xmin + (i+1)*region->cellSize, aCell->box.ymax = region->chosen_polygon->box.ymin + (j+1)*region->cellSize;
			}
	} else {
		region->vCells = 0;
		region->hCells = 0;
		region->mesh = NULL;
		region->map = NULL;
	}
	duallist_init(&region->busCoveredCells);
}

void setup_roads_and_crosses_in_region(struct Region *region)
{
  	int m1, m2, n1, n2, i, j;
	struct Cell *aCell;
	struct Road *aRoad;
	struct Cross *aCross;
	struct Item *aItem, *bItem, *aCrossItem, *temp, *p;
	unsigned long k;
	struct Duallist surCells;
	int notFound;

	/* mount roads and crosses to cells first*/
	if(region->mesh != NULL) {
		aItem = region->roads.head;
		while(aItem != NULL) {
			aRoad = (struct Road*)aItem->datap;
			if(is_box_within_box(&aRoad->box, &region->chosen_polygon->box) && is_polyline_in_polygon(&aRoad->points, region->chosen_polygon)) {
			
				m1 = floor((aRoad->box.xmin - region->chosen_polygon->box.xmin)/region->cellSize);
				m2 = floor((aRoad->box.xmax - region->chosen_polygon->box.xmin)/region->cellSize);
				n1 = floor((aRoad->box.ymin - region->chosen_polygon->box.ymin)/region->cellSize);
				n2 = floor((aRoad->box.ymax - region->chosen_polygon->box.ymin)/region->cellSize);

				//if(m1>=0 && n1>= 0 && m2<region->hCells && n2<region->vCells) {
				for(i = m1; i<= m2 ; i++) {
				      for (j = n1; j <= n2; j++) {
					      aCell = region->mesh + i*region->vCells + j;
					      /* set up roads */
					      link_road(aRoad, &(aCell->roads));
				      }
				}

				aItem = aItem->next;
			} else {
				temp = aItem->next;
				road_free_func((struct Road*)duallist_pick_item(&region->roads, aItem));
				aItem = temp;
			}		
		}

		aItem = region->crosses.head;
		while (aItem != NULL) {
			aCross = (struct Cross*)aItem->datap;
			aCell = point_in_cell(region, &aCross->gPoint);
			if(aCell) {
				link_cross(aCross, &(aCell->crosses));
				aItem = aItem->next;
			} else {
				temp = aItem->next;
				cross_free_func((struct Cross*)duallist_pick_item(&region->crosses, aItem));
				aItem = temp;
			}
		}
	}

	/* setup connections between roads and crosses */
	aItem = region->roads.head;
	for(k=0;k<region->roads.nItems;k++) {
		aRoad = (struct Road*)aItem->datap;
		p = aRoad->origPoints.head;
		notFound = 1;
		duallist_init(&surCells);
		surroundings_from_point(&surCells, region, (struct Point*)p->datap);
		bItem = surCells.head;
		while(bItem != NULL && notFound) {
		      aCell = (struct Cell*)bItem->datap;
		      aCrossItem = aCell->crosses.head;
		      while(aCrossItem != NULL) {
			      if((int)aRoad->headEnd == ((struct Cross*)aCrossItem->datap)->number) {
				      aRoad->headEnd = (struct Cross*)aCrossItem->datap;
				      link_road(aRoad, &(((struct Cross*)aCrossItem->datap)->outRoads));
				      notFound = 0;
				      break;
			      }
			      aCrossItem=aCrossItem->next;
		      }
		      bItem = bItem->next;
		}
		duallist_destroy(&surCells, NULL);

		p = aRoad->origPoints.head->prev;
		notFound = 1;
		duallist_init(&surCells);
		surroundings_from_point(&surCells, region, (struct Point*)p->datap);
		bItem = surCells.head;
		while(bItem != NULL && notFound) {
		      aCell = (struct Cell*)bItem->datap;
		      aCrossItem = aCell->crosses.head;
		      while(aCrossItem != NULL) {
			      if((int)aRoad->tailEnd == ((struct Cross*)aCrossItem->datap)->number) {
				      aRoad->tailEnd = (struct Cross*)aCrossItem->datap;
				      link_road(aRoad, &(((struct Cross*)aCrossItem->datap)->inRoads));
				      notFound = 0;
				      break;
			      }
			      aCrossItem=aCrossItem->next;
		      }
		      bItem = bItem->next;
		}
		duallist_destroy(&surCells, NULL);
		aItem = aItem->next;
	}
}


/*
 * dump region
 */
void region_dump_func(FILE *fOutput, struct Region *region)
{
	fwrite(&region->scrapLength, sizeof(double), 1, fOutput);
	fwrite(&region->cellSize, sizeof(double), 1, fOutput);
	fwrite(&region->maxdgr,sizeof(int), 1, fOutput);
	fwrite(&region->roadNums,sizeof(int), 1, fOutput);
	fwrite(&region->crossNums,sizeof(int), 1, fOutput);
	polygon_dump_func(fOutput, region->chosen_polygon);
	duallist_dump(fOutput, &region->roads, (void(*)(FILE*,void*))road_dump_func); 
	duallist_dump(fOutput, &region->crosses, (void(*)(FILE*,void*))cross_dump_func); 
	duallist_dump(fOutput, &region->districts, (void(*)(FILE*,void*))district_dump_func);
	duallist_dump(fOutput, &region->rivers, (void(*)(FILE*,void*))river_dump_func);
}


void region_free_func( struct Region *aRegion)
{
  struct Cell *aCell;
  int i, j;

  if(aRegion == NULL)
	return;
  polygon_free_func(aRegion->chosen_polygon);
  duallist_destroy(&aRegion->roads, (void(*)(void*))road_free_func);
  duallist_destroy(&aRegion->crosses, (void(*)(void*))cross_free_func);
  duallist_destroy(&aRegion->rivers, (void(*)(void*))river_free_func);
  duallist_destroy(&aRegion->districts, (void(*)(void*))district_free_func);

  for(i = 0; i<aRegion->hCells; i ++) {
	for(j = 0; j<aRegion->vCells; j++) {
		aCell = aRegion->mesh + i*aRegion->vCells + j;
		cell_free_func(aCell);
	}
  }
  duallist_destroy(&aRegion->busCoveredCells, NULL);
  free(aRegion->mesh);
  free(aRegion->map);
  free(aRegion);
}




/***********************************************************
 * Polygon structures related
 * ********************************************************/
void polygon_dump_func(FILE *fOutput, struct Polygon *aPolygon)
{
	fwrite(&aPolygon->box, sizeof(double), 4, fOutput);
	duallist_dump(fOutput, &aPolygon->points, (void(*)(FILE*,void*))point_dump_func);
}


struct Polygon* polygon_load_func(FILE *fInput)
{
	struct Polygon* newPolygon;

	newPolygon = (struct Polygon*)malloc(sizeof(struct Polygon));
	fread(&newPolygon->box, sizeof(double), 4, fInput);
	duallist_load(fInput, &newPolygon->points, (void*(*)(FILE*))point_load_func);	
	return newPolygon;
}

struct Polygon* polygon_copy_func(struct Polygon *aPolygon)
{
	struct Polygon* newPolygon=NULL;

	if(aPolygon!=NULL) {
		newPolygon = (struct Polygon*)malloc(sizeof(struct Polygon));
		newPolygon->box.xmin = aPolygon->box.xmin;
		newPolygon->box.ymin = aPolygon->box.ymin;
		newPolygon->box.xmax = aPolygon->box.xmax;
		newPolygon->box.ymax = aPolygon->box.ymax;
		duallist_copy(&newPolygon->points, &aPolygon->points, (void*(*)(void*))point_copy_func);
	}
	return newPolygon;
}

/*
 * build interested region area
 */
void build_polygon(struct Polygon ** chosen_polygon, struct Point *aPoint)
{
  struct Point *newp;

  newp = (struct Point*)malloc(sizeof(struct Point));
  newp->x = aPoint->x, newp->y = aPoint->y;
  if(*chosen_polygon == NULL) {
	*chosen_polygon = (struct Polygon*)malloc(sizeof(struct Polygon));
	duallist_init(&(*chosen_polygon)->points);
	(*chosen_polygon)->currentPoint = duallist_add_to_tail(&(*chosen_polygon)->points, newp);
	(*chosen_polygon)->mouseAt = (*chosen_polygon)->currentPoint;
	(*chosen_polygon)->state = 0;
	(*chosen_polygon)->box.xmin = (*chosen_polygon)->box.xmax = newp->x;
	(*chosen_polygon)->box.ymin = (*chosen_polygon)->box.ymax = newp->y;
  } else {
	(*chosen_polygon)->currentPoint = duallist_add_to_tail(&(*chosen_polygon)->points, newp);
	(*chosen_polygon)->mouseAt = (*chosen_polygon)->currentPoint;
	(*chosen_polygon)->box.xmin = MIN((*chosen_polygon)->box.xmin, newp->x);
	(*chosen_polygon)->box.xmax = MAX((*chosen_polygon)->box.xmax, newp->x);
	(*chosen_polygon)->box.ymin = MIN((*chosen_polygon)->box.ymin, newp->y);
	(*chosen_polygon)->box.ymax = MAX((*chosen_polygon)->box.ymax, newp->y);
  }
}

void close_polygon(struct Polygon *chosen_polygon)
{
  
  struct Point *newp;

  newp = (struct Point*)malloc(sizeof(struct Point));
  newp->x = ((struct Point*)chosen_polygon->points.head->datap)->x;
  newp->y = ((struct Point*)chosen_polygon->points.head->datap)->y;
  duallist_add_to_tail(&chosen_polygon->points, newp);
  chosen_polygon->mouseAt = NULL;
  /* the polygon is completed */
  chosen_polygon->state = 1;
  chosen_polygon->scale = 1;
}

void polygon_free_func(struct Polygon * chosen_polygon)
{

	if(chosen_polygon == NULL) return;
	duallist_destroy(&chosen_polygon->points, (void(*)(void*))point_free_func);
	free(chosen_polygon);
}
			

int is_legal(struct Polygon *chosen_polygon, struct Point *aPoint)
{
  struct Item *previous, *p;
  struct Segment seg1, seg2;
  previous = chosen_polygon->points.head;

  if(chosen_polygon->points.nItems < 2) { 
	if (point_equal_func(chosen_polygon->points.head->datap, aPoint)) 
		return 0;
	else 
		return 1;
  }

  while(previous->next != chosen_polygon->currentPoint) previous = previous->next; 
  if( (equald(aPoint->x, ((struct Point*)previous->datap)->x, DELTA) && equald(aPoint->y, ((struct Point*)previous->datap)->y, DELTA)) 
   || (equald(((struct Point*)chosen_polygon->currentPoint->datap)->x, aPoint->x, DELTA) && equald(((struct Point*)chosen_polygon->currentPoint->datap)->y, aPoint->y, DELTA)))
	return 0;

  seg1.aPoint.x = aPoint->x, seg1.aPoint.y = aPoint->y;
  seg1.bPoint.x = ((struct Point*)chosen_polygon->currentPoint->datap)->x, seg1.bPoint.y = ((struct Point*)chosen_polygon->currentPoint->datap)->y;
  p = chosen_polygon->points.head;
  while(p!=previous) {
	seg2.aPoint.x = ((struct Point*)p->datap)->x, seg2.aPoint.y = ((struct Point*)p->datap)->y;
	seg2.bPoint.x = ((struct Point*)p->next->datap)->x, seg2.bPoint.y = ((struct Point*)p->next->datap)->y;
 	if(are_segments_intersected(&seg1, &seg2)) return 0;
 	p = p->next;
  }
  return 1;
}


int is_polygon(struct Polygon *chosen_polygon, struct Point *aPoint)
{
  struct Item *previous, *p;
  previous = chosen_polygon->points.head;

  if(chosen_polygon->points.nItems < 2) return 0;

  while(previous->next != chosen_polygon->currentPoint) previous = previous->next; 
  if( (equald(aPoint->x, ((struct Point*)previous->datap)->x, DELTA) && equald(aPoint->y, ((struct Point*)previous->datap)->y, DELTA))
   || (equald(((struct Point*)chosen_polygon->currentPoint->datap)->x,aPoint->x, DELTA) && equald(((struct Point*)chosen_polygon->currentPoint->datap)->y, aPoint->y, DELTA)))
	return 0;

  p = chosen_polygon->points.head;
  while(p!=previous) {
 	if(equald(aPoint->x,((struct Point*)p->datap)->x,DELTA) && equald(aPoint->y, ((struct Point*)p->datap)->y, DELTA)) return 1;
 	p = p->next;
  }
  return 0;
}

/***********************************************************
 * District structure related
 * ********************************************************/

void district_dump_func(FILE *fOutput, struct District *aDistrict)
{
	fwrite(&aDistrict->box, sizeof(double), 4, fOutput);
	duallist_dump(fOutput, &aDistrict->rings, (void(*)(FILE*,void*))polyline_dump_func);
}


struct District* district_load_func(FILE *fInput)
{
	struct District *newDistrict;
	newDistrict = (struct District*)malloc(sizeof(struct District));
	fread(&newDistrict->box, sizeof(double), 4, fInput);
	duallist_load(fInput, &newDistrict->rings, (void*(*)(FILE*))polyline_load_func);
	return newDistrict;
}


void district_free_func(struct District *aDistrict)
{
	if(aDistrict == NULL) return;
	duallist_destroy(&aDistrict->rings, (void(*)(void*))polyline_free_func);
	free(aDistrict);
}





/***********************************************************
 * River structure related
 * ********************************************************/

struct River* river_load_func(FILE *fInput)
{
	struct River *newRiver;
	newRiver = (struct River*)malloc(sizeof(struct River));
	fread(&newRiver->box, sizeof(double), 4, fInput);
	duallist_load(fInput, &newRiver->rings, (void*(*)(FILE*))polyline_load_func);
	return newRiver;
}


void river_dump_func(FILE *fOutput, struct River *aRiver)
{
	fwrite(&aRiver->box, sizeof(double), 4, fOutput);
	duallist_dump(fOutput, &aRiver->rings, (void(*)(FILE*,void*))polyline_dump_func);
}

void river_free_func(struct River *aRiver)
{
	if(aRiver == NULL) return;
	duallist_destroy(&aRiver->rings, (void(*)(void*))polyline_free_func);
	free(aRiver);
}






/***********************************************************
 * Cell structure related
 * ********************************************************/

void cell_init_func(struct Cell *aCell)
{
	if(aCell == NULL) return;
	aCell->n1 = 0;
	aCell->n2 = 0;
	aCell->n3 = 0;
	aCell->n4 = 0;
	aCell->n5 = 0;
	aCell->n6 = 0;
	aCell->slots = NULL;
	duallist_init(&aCell->reps);
	duallist_init(&aCell->roads);
	duallist_init(&aCell->crosses);
	duallist_init(&aCell->routes);
	duallist_init(&aCell->stops);
	duallist_init(&aCell->displays);
	aCell->storage = NULL;
}

void cell_free_func(struct Cell *aCell)
{
	if(aCell == NULL) return;
	duallist_destroy(&aCell->reps, NULL);
	duallist_destroy(&aCell->roads, NULL);
	duallist_destroy(&aCell->crosses, NULL);
	duallist_destroy(&aCell->routes, NULL);
	duallist_destroy(&aCell->stops, NULL);
	duallist_destroy(&aCell->displays, free);
}

struct Cell* randomly_pick_a_bus_covered_cell(struct Region *region)
{
	int index;
	struct Item *aItem;
	struct Cell *aCell;

	index = rand()%region->busCoveredCells.nItems+1;

	aItem = region->busCoveredCells.head;
	while(aItem != NULL) {
		index --;
		if(index == 0) {
			aCell = (struct Cell*)aItem->datap;
			return aCell;
		}
		aItem = aItem->next;
	}
	return NULL;
}

int cell_has_less_routes_than(struct Cell *aCell, struct Cell *bCell)
{
	return aCell->routes.nItems < bCell->routes.nItems;
}

int cell_equal_func(struct Cell *aCell, struct Cell *bCell)
{
	return aCell->xNumber == bCell->xNumber && aCell->yNumber == bCell->yNumber;
}

int cell_has_nos(char *nos, struct Cell *aCell)
{
	char buf[128];
	sprintf(buf, "%d,%d", aCell->xNumber, aCell->yNumber);
	return !strcmp(nos, buf);
}

void load_cell_displays_with_hashtable(FILE *fsource, struct Region *region, struct Hashtable *cellTable, time_t *startAt, time_t *endAt)
{
	char buf[128], *tm, *strp, key[128];
	struct Display *newDisplay;
	struct Item *aItem;
	struct Cell *aCell;
	int first, xNumber, yNumber;

	if(cellTable == NULL)
		return;
	first = 1;
	while(fgets(buf, 128, fsource)) {
		newDisplay = (struct Display*)malloc(sizeof(struct Display));

		strp = strtok(buf, ",");
		xNumber = atoi(strp);
		strp = strtok(NULL, ",");
		yNumber = atoi(strp);
		tm = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		newDisplay->value = atof(strp);
		newDisplay->timestamp = strtot(tm);

		sprintf(key, "%d,%d", xNumber, yNumber);
		aItem = hashtable_find(cellTable, key);
		if(aItem == NULL) {
			aCell = region->mesh + xNumber * region->vCells + yNumber;
			hashtable_add(cellTable, key, aCell);
		} else
			aCell = (struct Cell*)aItem->datap;
		newDisplay->party = aCell;
		duallist_add_to_tail(&aCell->displays, newDisplay);
		if(startAt != NULL && endAt!=NULL) {
			if(first) {
				*startAt = newDisplay->timestamp;
				*endAt = newDisplay->timestamp;
				first --;
			} else {
				if (newDisplay->timestamp < *startAt)
					*startAt = newDisplay->timestamp;
				if (newDisplay->timestamp > *endAt)
					*endAt = newDisplay->timestamp;
			}
		}

	}

}


void set_cell_table_time(struct Hashtable *cellTable, time_t atClock)
{
	struct Item *aItem;
	struct Item *bItem;
	struct Display *aDisplay;
	struct Cell *aCell;
	unsigned long i;

	for(i = 0; i<cellTable->size; i++) {
		aItem = cellTable->head[i];
		while (aItem!=NULL)
		{
			aCell = (struct Cell*)aItem->datap;
			bItem = aCell->displays.head;
			while(bItem != NULL) {
				aDisplay = (struct Display*)bItem->datap;
				aDisplay->shown = 1;
				if(aDisplay->timestamp >= atClock) {
					aCell->at = bItem;
					aCell->countdown = difftime(aDisplay->timestamp, atClock);
					break;
				}
				bItem = bItem->next;
			}
			while(bItem != NULL) {
				aDisplay = (struct Display*)bItem->datap;
				aDisplay->shown = 0;
				bItem = bItem->next;
			}
			aItem = aItem->next;
		}
	}
}


void cells_on_line(struct Region *region, struct Point *point1, struct Point *point2, struct Duallist *cellList)
{
	struct Item *aItem;
	struct Cell *aCell, *bCell;
	int i;
	double dx, dy, length, x, y;
	char buf[128];

	aCell = point_in_cell(region, point1);
	bCell = point_in_cell(region, point2);
	if(aCell && bCell && cellList) {
		if(fabs(bCell->xNumber - aCell->xNumber)>= fabs(bCell->yNumber - aCell->yNumber))
			length = fabs(bCell->xNumber - aCell->xNumber);
		else
			length = fabs(bCell->yNumber - aCell->yNumber);
		if(length) {
			dx = (bCell->xNumber - aCell->xNumber)/length;
			dy = (bCell->yNumber - aCell->yNumber)/length;
			i = 1;
			x = aCell->xNumber;
			y = aCell->yNumber;
			while(i<=length) {
				sprintf(buf, "%d,%d", (int)(x+0.5), (int)(y+0.5));
				aItem = duallist_find(cellList, buf, (int(*)(void*,void*))cell_has_nos);
				if(!aItem) 
					duallist_add_to_tail(cellList, region->mesh + (int)(x+0.5)*region->vCells + (int)(y+0.5));
				x = x + dx;
				y = y + dy;
				i ++;
			}
		} else {
			sprintf(buf, "%d,%d", aCell->xNumber, aCell->yNumber);
			aItem = duallist_find(cellList, buf, (int(*)(void*,void*))cell_has_nos);
			if(!aItem) 
				duallist_add_to_tail(cellList, aCell);
		}
	}
}




/***********************************************************
 * Box structure related
 * ********************************************************/

 /* Create a box with length meters wide */
void create_box(struct Point *aPoint, double length, struct Box *aBox)
{
	double unit;
	unit = distance_in_latitude(length)/2;
	aBox->xmin = aPoint->x - unit;
	aBox->ymin = aPoint->y - unit;
	aBox->xmax = aPoint->x + unit;
	aBox->ymax = aPoint->y + unit;
//	printf("unit: %lf\n", 2*unit);
//	printf("distance: %lf\n", distance_in_meter(0, 0, 0, 2*unit));
}

void merge_boxes(struct Box *targetBox, struct Box *aBox, int firsttime)
{
	if(targetBox != NULL) {
		if (firsttime == 1) {
			targetBox->xmin = aBox->xmin;
			targetBox->xmax = aBox->xmax;
			targetBox->ymin = aBox->ymin;
			targetBox->ymax = aBox->ymax;
		} else {
			targetBox->xmin = MIN(targetBox->xmin, aBox->xmin);
			targetBox->ymin = MIN(targetBox->ymin, aBox->ymin);
			targetBox->xmax = MAX(targetBox->xmax, aBox->xmax);
			targetBox->ymax = MAX(targetBox->ymax, aBox->ymax);
		}
	}
}




/***********************************************************
 * Path structure related
 * ********************************************************/


/* find the shortest path from sCross to dCross */
struct Path* find_shortest_path(struct Region* region, struct Cross* sCross, struct Cross* dCross)
{
	struct BinaryHeap *newHeap;
	struct Path *rtPath = NULL;
	struct Item *aItem;
	struct Cross *aCross, *nextCross, *prevCross;
	struct Road *aRoad;
	long selectCount;

	if(sCross == NULL || dCross == NULL)
		return NULL;

	aItem = region->crosses.head;
	while(aItem != NULL) {
		aCross = (struct Cross*)aItem->datap;
		aCross->fromCross = NULL;
		aCross->checked = 0;
		aCross->pastCost = 0;
		aCross->basicCost = 0;
		aItem = aItem->next;
	}
		
	newHeap = (struct BinaryHeap*)malloc(sizeof(struct BinaryHeap));
	binaryHeap_init(newHeap, region->crosses.nItems, (int(*)(void*,void*))cheaper_cross, (void(*)(char*, void*))get_cross_name); 
	binaryHeap_add(newHeap, sCross);
	selectCount = 0;
	while (!is_binaryHeap_empty(newHeap) && !dCross->checked && selectCount < MAXIMUM_SEARCH_COUNT) {
		aCross = binaryHeap_pick(newHeap);
		selectCount ++;
		aCross->checked = 1;
		aItem = aCross->outRoads.head;
		while(aItem!=NULL) {
			aRoad = (struct Road*)aItem->datap;
			nextCross = aRoad->tailEnd;
			if(!nextCross->checked) {
				if(is_entry_in_binaryHeap(newHeap, nextCross)) {
					if(aCross->pastCost + aRoad->length < nextCross->pastCost) {
						nextCross->fromCross = aCross;
						nextCross->pastCost = aCross->pastCost + aRoad->length; 	
						binaryHeap_resort(newHeap, nextCross);
					}
				} else {
					nextCross->fromCross = aCross;
					nextCross->pastCost = aCross->pastCost + aRoad->length;
					nextCross->basicCost = distance_in_meter(nextCross->gPoint.x, nextCross->gPoint.y, dCross->gPoint.x, dCross->gPoint.y);
					binaryHeap_add(newHeap, nextCross);
				}
			}
			aItem = aItem->next;
		}
	}

	if(dCross->fromCross != NULL) {
		rtPath = (struct Path*)malloc(sizeof(struct Path));
		rtPath->length = 0;
		rtPath->turns = 0;
		duallist_init(&rtPath->roads);
		aCross = dCross->fromCross;
		prevCross = dCross;
		while (aCross != NULL) {
			aItem = aCross->outRoads.head;
			while(aItem != NULL) {
				aRoad = (struct Road*)aItem->datap;
				if(aRoad->tailEnd == prevCross) {
					rtPath->length += aRoad->length;
					duallist_add_to_head(&rtPath->roads, aRoad);
					break;
				}
				aItem = aItem->next;
			}
			if(aCross == sCross) {
				break;
			} else {
				prevCross = aCross;
				aCross = aCross->fromCross;
			}
		}
	}
	binaryHeap_destroy(newHeap, NULL);	
	return rtPath;
}

void path_init_func(struct Path *aPath)
{
	if(aPath == NULL) return;
	duallist_init(&aPath->roads);
	aPath->length = 0;
	aPath->turns = 0;
}

void path_free_func(struct Path *aPath)
{
	if(aPath == NULL) return;
	duallist_destroy(&aPath->roads, NULL);
	free(aPath);
}

struct Duallist* polyline_on_path(struct Path *aPath, struct Point *fromPoint, struct Point *toPoint)
{
	struct Duallist *rtPoints, polyline, aPart;
	struct Road *aRoad;
	struct Point *aPoint, *newPoint;
	struct Item *p, *q;
	
	if(aPath == NULL) return NULL;

	rtPoints = (struct Duallist*)malloc(sizeof(struct Duallist));
	duallist_init(rtPoints);

	duallist_init(&polyline);
	p = aPath->roads.head;
	while(p!=NULL) {
		aRoad = (struct Road*)p->datap;
		q = aRoad->points.head;
		while(q!=NULL) {
			aPoint = (struct Point*)q->datap;
			if(!duallist_find(rtPoints, aPoint, (int(*)(void*,void*))point_equal_func)) {
				newPoint = (struct Point*)malloc(sizeof(struct Point));
				newPoint->x = aPoint->x;
				newPoint->y = aPoint->y;
				duallist_add_to_tail(&polyline, newPoint);
			}
			q=q->next;
		}
		p = p->next;
	}
	
	if(fromPoint) {
		point_cut_polyline(&polyline, fromPoint, NULL, &aPart);
		duallist_destroy(&polyline, (void(*)(void*))point_free_func);
		duallist_copy(&polyline, &aPart, (void*(*)(void*))point_copy_func);
		duallist_destroy(&aPart, (void(*)(void*))point_free_func);
	}

	if(toPoint) {
		point_cut_polyline(&polyline, toPoint, &aPart, NULL);
		duallist_destroy(&polyline, (void(*)(void*))point_free_func);
		duallist_copy(&polyline, &aPart, (void*(*)(void*))point_copy_func);
		duallist_destroy(&aPart, (void(*)(void*))point_free_func);
	}

	duallist_copy(rtPoints, &polyline, (void*(*)(void*))point_copy_func);
	duallist_destroy(&polyline, (void(*)(void*))point_free_func);
	
	return rtPoints;
}

struct Path* path_copy_func(struct Path *aPath)
{
	struct Item *aItem;
	struct Path *rtPath;

	if (aPath == NULL) return NULL;

	rtPath = (struct Path*)malloc(sizeof(struct Path));
	duallist_init(&rtPath->roads);

	aItem = aPath->roads.head;	
	while(aItem != NULL) {
		duallist_add_to_tail(&rtPath->roads, aItem->datap);
		aItem = aItem->next;
	}
	rtPath->length = aPath->length;
	rtPath->turns = aPath->turns;

	return rtPath;
}


int is_path_within_box(struct Path *aPath, struct Box *aBox)
{
	struct Item *aItem, *bItem;
	struct Road *aRoad;

	aItem = aPath->roads.head;	
	while(aItem != NULL) {
		aRoad=(struct Road*)aItem->datap;
		bItem = aRoad->points.head;
		while(bItem!=NULL) {
			if (!is_point_in_box((struct Point*)bItem->datap, aBox))
				return 0;
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
	return 1;
}



/***********************************************************
 * Road structure related
 * ********************************************************/

void road_init_func(struct Road *aRoad)
{
	if(aRoad == NULL) return;
	duallist_init(&aRoad->origPoints);
	duallist_init(&aRoad->points);
	duallist_init(&aRoad->refs);
	duallist_init(&aRoad->scraps);
	aRoad->width = ROAD_WIDTH;
	aRoad->value = 0;
	aRoad->headEnd = NULL;
	aRoad->tailEnd = NULL;
	aRoad->nSamples = 0;
	aRoad->samples = 0;
	aRoad->error = 0;
}


void road_free_func(struct Road *aRoad)
{

	if(aRoad == NULL) return;
	duallist_destroy(&aRoad->origPoints, free);
	duallist_destroy(&aRoad->points, free);
	duallist_destroy(&aRoad->refs, (void(*)(void*))ref_free_func);
	duallist_destroy(&aRoad->scraps, (void(*)(void*))scrap_free_func);
	free(aRoad);
}	

void remove_road(struct Region *aRegion, struct Road *aRoad)
{
	duallist_pick(&aRegion->roads, &aRoad->id, (int(*)(void*,void*))road_has_id);
	road_free_func(aRoad);
}


int road_equal_func(struct Road *aRoad, struct Road *bRoad)
{
	return aRoad->id == bRoad->id;
}

int road_has_id(int* id, struct Road *aRoad)
{
	return *id == aRoad->id;
}

void road_dump_func(FILE *fOutput, struct Road *aRoad)
{
	fwrite(&aRoad->id, sizeof(int), 1, fOutput);
	fwrite(&aRoad->width, sizeof(double), 1, fOutput);
	fwrite(&aRoad->length, sizeof(double), 1, fOutput);
	fwrite(&aRoad->box, sizeof(double), 4, fOutput);
	duallist_dump(fOutput, &aRoad->origPoints, (void(*)(FILE*,void*))point_dump_func);
	duallist_dump(fOutput, &aRoad->points, (void(*)(FILE*,void*))point_dump_func);
	fwrite(&aRoad->headEndAngle, sizeof(int), 1, fOutput);
	fwrite(&aRoad->tailEndAngle, sizeof(int), 1, fOutput);
	/* record the numbers of headEnd and tailEnd crosses */
	fwrite(&aRoad->headEnd->number, sizeof(int), 1, fOutput);
	fwrite(&aRoad->tailEnd->number, sizeof(int), 1, fOutput);
	duallist_dump(fOutput, &aRoad->scraps, (void(*)(FILE*,void*))scrap_dump_func);
}


struct Road* road_load_func(FILE *fInput)
{
	struct Road *newRoad;
	newRoad = (struct Road*)malloc(sizeof(struct Road));

	fread(&newRoad->id, sizeof(int), 1, fInput);
	fread(&newRoad->width, sizeof(double), 1, fInput);
	fread(&newRoad->length, sizeof(double), 1, fInput);
	fread(&newRoad->box, sizeof(double), 4, fInput);
	duallist_load(fInput, &newRoad->origPoints, (void*(*)(FILE*))point_load_func);
	duallist_load(fInput, &newRoad->points, (void*(*)(FILE*))point_load_func);
	fread(&newRoad->headEndAngle, sizeof(int), 1, fInput);
	fread(&newRoad->tailEndAngle, sizeof(int), 1, fInput);
	/* note that pointers are holding crosses' numbers */
	fread(&newRoad->headEnd, sizeof(int), 1, fInput);
	fread(&newRoad->tailEnd, sizeof(int), 1, fInput);
	duallist_load(fInput, &newRoad->scraps, (void*(*)(FILE*))scrap_load_func);
	duallist_init(&newRoad->refs);
	newRoad->value = 0;
	newRoad->nSamples = 0;
	newRoad->samples = 0;
	newRoad->error = 0;
	return newRoad;
}

struct Road* road_copy_func(struct Road* aRoad)
{
	struct Road *rtRoad=NULL;
	if(aRoad!=NULL) {
		rtRoad=(struct Road*)malloc(sizeof(struct Road));
		rtRoad->id = aRoad->id;
		rtRoad->width = aRoad->width;
		rtRoad->length = aRoad->length;
		rtRoad->box.xmin = aRoad->box.xmin;
		rtRoad->box.ymin = aRoad->box.ymin;
		rtRoad->box.xmax = aRoad->box.xmax;
		rtRoad->box.ymax = aRoad->box.ymax;
		duallist_copy(&rtRoad->origPoints, &aRoad->origPoints, (void*(*)(void*))point_copy_func);
		duallist_copy(&rtRoad->points, &aRoad->points, (void*(*)(void*))point_copy_func);
		rtRoad->headEndAngle = aRoad->headEndAngle;
		rtRoad->tailEndAngle = aRoad->tailEndAngle;
		/* note that pointers are holding crosses' numbers */
		rtRoad->headEnd = (void*)aRoad->headEnd->number;
		rtRoad->tailEnd = (void*)aRoad->tailEnd->number;
		duallist_init(&rtRoad->scraps);
		duallist_init(&rtRoad->refs);
		rtRoad->value = 0;
		rtRoad->nSamples = 0;
		rtRoad->samples = 0;
		rtRoad->error = 0;
	}
	return rtRoad;
}


struct Scrap* scrap_load_func(FILE *fInput)
{
	struct Scrap* newScrap;
	newScrap = (struct Scrap*)malloc(sizeof(struct Scrap));
	duallist_load(fInput, &newScrap->points, (void*(*)(FILE*))point_load_func);
	return newScrap;	
}


/* creat an alike road skeleton according to right-hand driving rule
 */
void offset_right_road(struct Duallist *original, double width, struct Duallist *polyline)
{
	struct Segment segment, aSegment, bSegment, farSegment;
	struct Item *p;
	unsigned long i;
	struct Point *newp, cutPoint;
	double offset, rAngle, segAngle;
	struct Duallist aPart;

	duallist_init(polyline);
	offset = distance_in_latitude(width);
	p = original->head;
	segment.aPoint.x = ((struct Point*)p->datap)->x;
	segment.aPoint.y = ((struct Point*)p->datap)->y;
	segment.bPoint.x = ((struct Point*)p->next->datap)->x;
	segment.bPoint.y = ((struct Point*)p->next->datap)->y;
	offset_right(&segment, offset, &aSegment);
	newp = (struct Point*)malloc(sizeof(struct Point));
	newp->x = aSegment.aPoint.x, newp->y = aSegment.aPoint.y;
	duallist_add_to_tail(polyline, newp);
	newp = (struct Point*)malloc(sizeof(struct Point));
	newp->x = aSegment.bPoint.x, newp->y = aSegment.bPoint.y;
	duallist_add_to_tail(polyline, newp);
	
	p = p->next;
	for(i = 1; i<original->nItems-1; i++) {
		segment.aPoint.x = ((struct Point*)p->datap)->x;
		segment.aPoint.y = ((struct Point*)p->datap)->y;
		segment.bPoint.x = ((struct Point*)p->next->datap)->x;
		segment.bPoint.y = ((struct Point*)p->next->datap)->y;
		offset_right(&segment, offset, &bSegment);
		rAngle=getRotateAngle(&aSegment, &bSegment);
		if(rAngle > 180 && rAngle < 360) {
			if(is_segment_cut_polyline(&bSegment, polyline, &cutPoint)) {
				point_cut_polyline(polyline, &cutPoint, &aPart, NULL);
				duallist_destroy(polyline, (void(*)(void*))point_free_func);
				duallist_copy(polyline, &aPart, (void*(*)(void*))point_copy_func);
				duallist_destroy(&aPart, (void(*)(void*))point_free_func);
				newp = (struct Point*)malloc(sizeof(struct Point));
				newp->x = bSegment.bPoint.x, newp->y = bSegment.bPoint.y;
				duallist_add_to_tail(polyline, newp);
			} else {
				segAngle = angle_between(bSegment.aPoint.x, bSegment.aPoint.y, bSegment.bPoint.x, bSegment.bPoint.y);	
				farSegment.aPoint.x = bSegment.aPoint.x;
				farSegment.aPoint.y = bSegment.aPoint.y;
				farSegment.bPoint.x = bSegment.aPoint.x + cos(M_PI*segAngle/180)*offset; 
				farSegment.bPoint.y = bSegment.aPoint.y + sin(M_PI*segAngle/180)*offset; 
				if(is_segment_cut_polyline(&farSegment, polyline, &cutPoint)) {
					point_cut_polyline(polyline, &cutPoint, &aPart, NULL);
					duallist_destroy(polyline, (void(*)(void*))point_free_func);
					duallist_copy(polyline, &aPart, (void*(*)(void*))point_copy_func);
					duallist_destroy(&aPart, (void(*)(void*))point_free_func);
				}
			}
		} else if ( rAngle <= 180 && rAngle >= 0  ) {
			if(is_segment_cut_polyline(&bSegment, polyline, &cutPoint)) {
				point_cut_polyline(polyline, &cutPoint, &aPart, NULL);
				duallist_destroy(polyline, (void(*)(void*))point_free_func);
				duallist_copy(polyline, &aPart, (void*(*)(void*))point_copy_func);
				duallist_destroy(&aPart, (void(*)(void*))point_free_func);
				newp = (struct Point*)malloc(sizeof(struct Point));
				newp->x = bSegment.bPoint.x, newp->y = bSegment.bPoint.y;
				duallist_add_to_tail(polyline, newp);
			} else {
				segAngle = angle_between(bSegment.aPoint.x, bSegment.aPoint.y, bSegment.bPoint.x, bSegment.bPoint.y);	
				farSegment.aPoint.x = bSegment.aPoint.x;
				farSegment.aPoint.y = bSegment.aPoint.y;
				farSegment.bPoint.x = bSegment.aPoint.x + cos(M_PI*segAngle/180)*offset; 
				farSegment.bPoint.y = bSegment.aPoint.y + sin(M_PI*segAngle/180)*offset; 
				if(is_segment_cut_polyline(&farSegment, polyline, &cutPoint)) {
					point_cut_polyline(polyline, &cutPoint, &aPart, NULL);
					duallist_destroy(polyline, (void(*)(void*))point_free_func);
					duallist_copy(polyline, &aPart, (void*(*)(void*))point_copy_func);
					duallist_destroy(&aPart, (void(*)(void*))point_free_func);
				} else {
					newp = (struct Point*)malloc(sizeof(struct Point));
					newp->x = bSegment.aPoint.x, newp->y = bSegment.aPoint.y;
					duallist_add_to_tail(polyline, newp);
					newp = (struct Point*)malloc(sizeof(struct Point));
					newp->x = bSegment.bPoint.x, newp->y = bSegment.bPoint.y;
					duallist_add_to_tail(polyline, newp);
				}
			}
		}

		//check and remove a loop in the list
//		remove_loop(polyline);
		
		aSegment.aPoint.x = bSegment.aPoint.x;
		aSegment.aPoint.y = bSegment.aPoint.y;
		aSegment.bPoint.x = bSegment.bPoint.x;
		aSegment.bPoint.y = bSegment.bPoint.y;
		p = p->next;
	}
	newp = (struct Point*)malloc(sizeof(struct Point));
	newp->x = aSegment.bPoint.x, newp->y = aSegment.bPoint.y;
	duallist_add_to_tail(polyline, newp);
}



void offset_right(struct Segment *original, double offset, struct Segment *rnSegment)
{
	double sine, cosine;
	double deltaY, deltaX;

	deltaY = original->bPoint.y - original->aPoint.y;
	deltaX = original->bPoint.x - original->aPoint.x;

	sine = deltaY/sqrt(deltaY*deltaY + deltaX*deltaX);
	cosine = deltaX/sqrt(deltaY*deltaY + deltaX*deltaX);

	rnSegment->aPoint.x = original->aPoint.x + offset * sine;
	rnSegment->aPoint.y = original->aPoint.y - offset * cosine;
	rnSegment->bPoint.x = original->bPoint.x + offset * sine;
	rnSegment->bPoint.y = original->bPoint.y - offset * cosine;
}


void setup_road (struct Road *newRoad, struct Region *region)
{

  struct Cell *aCell;
  struct Item *p,*q;
  struct Cross *newCross;
  struct Item *aItem, *aCrossItem;
  double agl;
  int m1, m2, n1, n2, i, j;
  int isNewCross;
  struct Duallist surCells;

  newRoad->length = 0;

  p = newRoad->origPoints.head;
  newRoad->box.xmin = newRoad->box.xmax = ((struct Point*)p->datap)->x;
  newRoad->box.ymin = newRoad->box.ymax = ((struct Point*)p->datap)->y;
  q = p->next;
  while(q != NULL) {
	newRoad->length = newRoad->length + distance_in_meter(((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, ((struct Point*)q->datap)->x, ((struct Point*)q->datap)->y);
  	newRoad->box.xmin = MIN(((struct Point*)q->datap)->x, newRoad->box.xmin);
  	newRoad->box.ymin = MIN(((struct Point*)q->datap)->y, newRoad->box.ymin);
  	newRoad->box.xmax = MAX(((struct Point*)q->datap)->x, newRoad->box.xmax);
  	newRoad->box.ymax = MAX(((struct Point*)q->datap)->y, newRoad->box.ymax);
  	p = q;
  	q = q->next;
  }

  if(region->mesh != NULL) {
	  m1 = floor((newRoad->box.xmin - region->chosen_polygon->box.xmin)/region->cellSize);
	  m2 = floor((newRoad->box.xmax - region->chosen_polygon->box.xmin)/region->cellSize);
	  n1 = floor((newRoad->box.ymin - region->chosen_polygon->box.ymin)/region->cellSize);
	  n2 = floor((newRoad->box.ymax - region->chosen_polygon->box.ymin)/region->cellSize);

	  for(i = m1; i<= m2 ; i++) {
		for (j = n1; j <= n2; j++) {
			aCell = region->mesh + i*region->vCells + j;
			/* set up roads */
			link_road(newRoad, &(aCell->roads));
		}
	  }
  }
  /* set up crosses */
  isNewCross = 1;
  p = newRoad->origPoints.head;
  duallist_init(&surCells);
  surroundings_from_point(&surCells, region, (struct Point*)p->datap);
  aItem = surCells.head;
  while(isNewCross && aItem != NULL) {
	aCell = (struct Cell*)aItem->datap;
	aCrossItem = aCell->crosses.head;
	while(aCrossItem != NULL) {
		if(is_point_in_box(((struct Point*)p->datap), &(((struct Cross*)aCrossItem->datap)->box))) {
			isNewCross = 0;
			agl = angle_between(((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, ((struct Point*)p->next->datap)->x, ((struct Point*)p->next->datap)->y);
			newRoad->headEnd = (struct Cross*)aCrossItem->datap;
			newRoad->headEndAngle = agl;
			link_road(newRoad, &(((struct Cross*)aCrossItem->datap)->outRoads));
			/* the maximum degree of crosses in the region */
			if(region->maxdgr < ((struct Cross*)aCrossItem->datap)->outRoads.nItems + ((struct Cross*)aCrossItem->datap)->inRoads.nItems)
				region->maxdgr = ((struct Cross*)aCrossItem->datap)->outRoads.nItems+ ((struct Cross*)aCrossItem->datap)->inRoads.nItems;
			break;
		}
		aCrossItem=aCrossItem->next;
	}
	aItem = aItem->next;
  }
	
  if(isNewCross) {
	newCross = (struct Cross*)malloc(sizeof(struct Cross));	
	cross_init_func(newCross);
	newCross->number = region->crossNums++;
	newCross->gPoint.x = ((struct Point*)p->datap)->x;
	newCross->gPoint.y = ((struct Point*)p->datap)->y;
	agl = angle_between(((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, ((struct Point*)p->next->datap)->x, ((struct Point*)p->next->datap)->y);
	newRoad->headEnd = newCross;
	newRoad->headEndAngle = agl;
	link_road(newRoad, &(newCross->outRoads));

	create_box(((struct Point*)p->datap), CROSS_WIDTH, &(newCross->box));

	duallist_add_to_tail(&(region->crosses), newCross);
	link_cross(newCross, &(((struct Cell*)surCells.head->datap)->crosses));
  }
  duallist_destroy(&surCells, NULL);


  isNewCross = 1;
  p = newRoad->origPoints.head->prev;
  duallist_init(&surCells);
  surroundings_from_point(&surCells, region, (struct Point*)p->datap);
  aItem = surCells.head;
  while(isNewCross && aItem != NULL) {
	aCell = (struct Cell*)aItem->datap;
	aCrossItem = aCell->crosses.head;
	while(aCrossItem != NULL) {
		if(is_point_in_box(((struct Point*)p->datap), &(((struct Cross*)aCrossItem->datap)->box))) {
			isNewCross = 0;
			agl = angle_between(((struct Point*)p->prev->datap)->x, ((struct Point*)p->prev->datap)->y, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y);
			newRoad->tailEnd = (struct Cross*)aCrossItem->datap;
			newRoad->tailEndAngle = agl;
			link_road(newRoad, &(((struct Cross*)aCrossItem->datap)->inRoads));
			/* the maximum degree of crosses in the region */
			if(region->maxdgr < ((struct Cross*)aCrossItem->datap)->outRoads.nItems + ((struct Cross*)aCrossItem->datap)->inRoads.nItems)
				region->maxdgr = ((struct Cross*)aCrossItem->datap)->outRoads.nItems+ ((struct Cross*)aCrossItem->datap)->inRoads.nItems;
			break;
		}
		aCrossItem=aCrossItem->next;
	}
	aItem = aItem->next;
  }
	
  if(isNewCross) {
	newCross = (struct Cross*)malloc(sizeof(struct Cross));	
	cross_init_func(newCross);
	newCross->number = region->crossNums++;
	newCross->gPoint.x = ((struct Point*)p->datap)->x;
	newCross->gPoint.y = ((struct Point*)p->datap)->y;
	agl = angle_between(((struct Point*)p->prev->datap)->x, ((struct Point*)p->prev->datap)->y, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y);
	newRoad->tailEnd = newCross;
	newRoad->tailEndAngle = agl;
	link_road(newRoad, &(newCross->inRoads));

	create_box(((struct Point*)p->datap), CROSS_WIDTH, &(newCross->box));

	duallist_add_to_tail(&(region->crosses), newCross);
	link_cross(newCross, &(((struct Cell*)surCells.head->datap)->crosses));
  }
  duallist_destroy(&surCells, NULL);

  /* check whether a road has the same cross as both headEnd and tailEnd */
  if(newRoad->headEnd == newRoad->tailEnd) {
	remove_road(region, newRoad);
  }
}


/*
 * Link a road to some referer
 */
void link_road(struct Road *newRoad, struct Duallist *roadList)
{
  struct Ref *newRef;
 
  newRef = (struct Ref*)malloc(sizeof(struct Ref));
  newRef->byWho = roadList;
  newRef->at = duallist_add_to_head(roadList, newRoad); 
  duallist_add_to_head(&(newRoad->refs), newRef);
}


void slide_free_func(struct Slide *aSlide)
{
	if(aSlide == NULL) return;
	duallist_destroy(&aSlide->samples, free);
	free(aSlide);
}

void scrap_free_func(struct Scrap *aScrap)
{
	if(aScrap == NULL) return;
	duallist_destroy(&aScrap->points, (void(*)(void*))point_free_func);
	duallist_destroy(&aScrap->slides, (void(*)(void*))slide_free_func);
	free(aScrap);
}


void scrap_dump_func(FILE *fOutput, struct Scrap *aScrap)
{
	duallist_dump(fOutput, &aScrap->points, (void(*)(FILE*,void*))point_dump_func);
}




/***********************************************************
 * Cross structure related
 * ********************************************************/

void cross_init_func(struct Cross *aCross)
{
	if(aCross == NULL) return;
	duallist_init(&aCross->points);
	duallist_init(&aCross->inRoads);
	duallist_init(&aCross->outRoads);
	duallist_init(&aCross->refs);
	aCross->count = 0;
}

void cross_free_func(struct Cross *aCross)
{
	if(aCross == NULL) return;
	duallist_destroy(&aCross->points, free);
	duallist_destroy(&aCross->inRoads, NULL);
	duallist_destroy(&aCross->outRoads, NULL);
	duallist_destroy(&aCross->refs, (void(*)(void*))ref_free_func);
	free(aCross);
}

void cross_dump_func(FILE *fOutput, struct Cross *aCross)
{
	fwrite(&aCross->number, sizeof(int), 1, fOutput);
	point_dump_func(fOutput, &aCross->gPoint);
	duallist_dump(fOutput, &aCross->points, (void(*)(FILE*,void*))point_dump_func);
	fwrite(&aCross->box, sizeof(double), 4, fOutput);
}

struct Cross* cross_load_func(FILE *fInput)
{
	struct Cross *newCross;
	
	newCross = (struct Cross*)malloc(sizeof(struct Cross));
	fread(&newCross->number, sizeof(int), 1, fInput);
	fread(&newCross->gPoint, sizeof(double), 2, fInput);
	duallist_load(fInput, &newCross->points, (void *(*)(FILE*))point_load_func);
	fread(&newCross->box, sizeof(double), 4, fInput);
	duallist_init(&newCross->inRoads);
	duallist_init(&newCross->outRoads);
	duallist_init(&newCross->refs);
	newCross->range = 0;
	newCross->fromCross = NULL;
	newCross->checked = 0;
	newCross->pastCost = 0;
	newCross->basicCost = 0;
	newCross->count = 0;

	return newCross;
}


struct Cross* cross_copy_func(struct Cross* aCross)
{
	struct Cross *newCross;
	
	newCross = (struct Cross*)malloc(sizeof(struct Cross));
	newCross->number = aCross->number;
	newCross->gPoint.x = aCross->gPoint.x;
	newCross->gPoint.y = aCross->gPoint.y;
	duallist_copy(&newCross->points, &aCross->points, (void*(*)(void*))point_copy_func);
	newCross->box.xmin = aCross->box.xmin;
	newCross->box.ymin = aCross->box.ymin;
	newCross->box.xmax = aCross->box.xmax;
	newCross->box.ymax = aCross->box.ymax;
	duallist_init(&newCross->inRoads);
	duallist_init(&newCross->outRoads);
	duallist_init(&newCross->refs);
	newCross->range = 0;
	newCross->fromCross = NULL;
	newCross->checked = 0;
	newCross->pastCost = 0;
	newCross->basicCost = 0;
	newCross->count = 0;

	return newCross;
}



void remove_cross(struct Region *aRegion, struct Cross *currentCross)
{
	struct Item *aItem;
	struct Road *aRoad;

	duallist_pick(&aRegion->crosses, &currentCross->number, (int(*)(void*,void*))cross_has_number);

	aItem = currentCross->inRoads.head;
	while (aItem != NULL) {
		aRoad = (struct Road*) aItem->datap;
		remove_road(aRegion, aRoad);
		aItem = aItem->next;
	}	

	aItem = currentCross->outRoads.head;
	while (aItem != NULL) {
		aRoad = (struct Road*) aItem->datap;
		remove_road(aRegion, aRoad);
		aItem = aItem->next;
	}	
	
	cross_free_func(currentCross);
}

/*
 * Link a cross to some referer 
 */
void link_cross(struct Cross *newCross, struct Duallist *crossList)
{
	struct Ref *newRef;

	newRef = (struct Ref *)malloc(sizeof(struct Ref));
	newRef->byWho = crossList;
	newRef->at = duallist_add_to_head(crossList, newCross);
	duallist_add_to_head(&(newCross->refs), newRef);
}


void get_cross_name(char *name, struct Cross *aCross)
{
	sprintf(name, "%d", aCross->number);
}

int cross_has_number(int* number, struct Cross *aCross)
{
	return *number == aCross->number;
}

int cross_has_location(struct Point *gPoint, struct Cross *aCross)
{
	return point_equal_func(gPoint, &aCross->gPoint);
}


int cheaper_cross(struct Cross *aCross, struct Cross *bCross)
{
	if (aCross->pastCost + aCross->basicCost < bCross->pastCost + bCross->basicCost)
		return 1;
	else
		return 0;
}

int cross_equal_func(struct Cross *aCross, struct Cross *bCross)
{
	return aCross->number == bCross->number;
}





/***********************************************************
 * Segment structure related
 * ********************************************************/
double getRotateAngle(struct Segment *aSegment, struct Segment *bSegment)
{
 const double epsilon = 1.0e-6;
 const double nyPI = acos(-1.0);
 double dist, dot, degree, angle;
 double x1, y1, x2, y2;

 x1 = aSegment->bPoint.x - aSegment->aPoint.x;
 y1 = aSegment->bPoint.y - aSegment->aPoint.y;
 x2 = bSegment->bPoint.x - bSegment->aPoint.x;
 y2 = bSegment->bPoint.y - bSegment->aPoint.y;
 // normalize
 dist = sqrt( x1 * x1 + y1 * y1 );
 x1 /= dist;
 y1 /= dist;
 dist = sqrt( x2 * x2 + y2 * y2 );
 x2 /= dist;
 y2 /= dist;
 // dot product
 dot = x1 * x2 + y1 * y2;
 if ( fabs(dot-1.0) <= epsilon )
    angle = 0.0;
 else if ( fabs(dot+1.0) <= epsilon )
    angle = nyPI;
 else {
    double cross;
                  
    angle = acos(dot);
    //cross product
    cross = x1 * y2 - x2 * y1;
    // vector p2 is clockwise from vector p1
    // with respect to the origin (0.0)
    if (cross < 0 ) {
        angle = 2 * nyPI - angle;
    }    
 }
 degree = angle *  180.0 / nyPI;
 return degree;
}

int is_segment_cut_polyline(struct Segment *aSegment, struct Duallist *polyline, struct Point *cutPoint)
{
	struct Segment segment;
	struct Point bPoint;
	struct Item *p;

	if(polyline == NULL) {
		cutPoint->x = NOTSET;
		return 0;
	}

	p = polyline->head;
	while(p->next!=NULL) {
		segment.aPoint.x = ((struct Point*)p->datap)->x, segment.aPoint.y = ((struct Point*)p->datap)->y;
		segment.bPoint.x = ((struct Point*)p->next->datap)->x, segment.bPoint.y = ((struct Point*)p->next->datap)->y;
		if(are_segments_intersected(aSegment, &segment)) {
			segment_cut_segment(aSegment, &segment, cutPoint, &bPoint);
			return 1;
		}
		p = p->next;
	}
	return 0;
}


int are_segments_intersected(struct Segment *aSegment, struct Segment *bSegment)
{
  double ax, ay, bx, by, cx, cy;
  double unit1, unit2;
  struct Box aBox, bBox;

  aBox.xmin = MIN(aSegment->aPoint.x, aSegment->bPoint.x);
  aBox.xmax = MAX(aSegment->aPoint.x, aSegment->bPoint.x);
  aBox.ymin = MIN(aSegment->aPoint.y, aSegment->bPoint.y);
  aBox.ymax = MAX(aSegment->aPoint.y, aSegment->bPoint.y);

  bBox.xmin = MIN(bSegment->aPoint.x, bSegment->bPoint.x);
  bBox.xmax = MAX(bSegment->aPoint.x, bSegment->bPoint.x);
  bBox.ymin = MIN(bSegment->aPoint.y, bSegment->bPoint.y);
  bBox.ymax = MAX(bSegment->aPoint.y, bSegment->bPoint.y);

  if(!are_boxes_intersected(&aBox, &bBox)) return 0;

  ax = (aSegment->bPoint.x - aSegment->aPoint.x);
  ay = (aSegment->bPoint.y - aSegment->aPoint.y);

  bx = (bSegment->aPoint.x - aSegment->aPoint.x);
  by = (bSegment->aPoint.y - aSegment->aPoint.y);

  cx = (bSegment->bPoint.x - aSegment->aPoint.x);
  cy = (bSegment->bPoint.y - aSegment->aPoint.y);
 
  unit1 = (ax*by - ay*bx) * (ax*cy - ay*cx);
  
  ax = (bSegment->bPoint.x - bSegment->aPoint.x);
  ay = (bSegment->bPoint.y - bSegment->aPoint.y);

  bx = (aSegment->aPoint.x - bSegment->aPoint.x);
  by = (aSegment->aPoint.y - bSegment->aPoint.y);

  cx = (aSegment->bPoint.x - bSegment->aPoint.x);
  cy = (aSegment->bPoint.y - bSegment->aPoint.y);
 
  unit2 = (ax*by - ay*bx) * (ax*cy - ay*cx);

  if (!greaterd(unit1, 0, 0) && !greaterd(unit2 ,0, 0))
	return 1;

  return 0;
}

void copy_segment(struct Segment *dest, struct Segment *source)
{
	if(dest == NULL || source == NULL)
		return;
	dest->aPoint.x = source->aPoint.x;
	dest->aPoint.y = source->aPoint.y;
	dest->bPoint.x = source->bPoint.x;
	dest->bPoint.y = source->bPoint.y;
}

void segment_cut_segment(struct Segment *aSegment, struct Segment *bSegment, struct Point *aPoint, struct Point *bPoint)
{
  double a, c;
  double maxx, minx,maxy, miny;

  aPoint->x = bPoint->x = NOTSET;
  if(equald(aSegment->bPoint.x-aSegment->aPoint.x, 0, DELTA) && equald(bSegment->bPoint.x-bSegment->aPoint.x, 0, DELTA)){
	maxy=miny=aSegment->aPoint.y;
	if(greaterd(aSegment->bPoint.y,maxy,DELTA)) {maxy = aSegment->bPoint.y;}
	if(greaterd(bSegment->aPoint.y,maxy,DELTA)) {maxy = bSegment->aPoint.y;}
 	if(greaterd(bSegment->bPoint.y,maxy,DELTA)) {maxy = bSegment->bPoint.y;}
	if(smallerd(aSegment->bPoint.y,miny,DELTA)) {miny = aSegment->bPoint.y;}
	if(smallerd(bSegment->aPoint.y,miny,DELTA)) {miny = bSegment->aPoint.y;}
 	if(smallerd(bSegment->bPoint.y,miny,DELTA)) {miny = bSegment->bPoint.y;}

 	if(!greaterd(aSegment->aPoint.y, maxy, DELTA) && !smallerd(aSegment->aPoint.y, miny, DELTA))
		aPoint->x = aSegment->aPoint.x, aPoint->y =aSegment->aPoint.y;
	
	if(!greaterd(aSegment->bPoint.y, maxy, DELTA) && !smallerd(aSegment->bPoint.y, miny, DELTA)) {
		if (aPoint->x == NOTSET)
			aPoint->x = aSegment->bPoint.x, aPoint->y =aSegment->bPoint.y;
		else 
			bPoint->x = aSegment->bPoint.x, bPoint->y=aSegment->bPoint.y;
	}
	if(!greaterd(bSegment->aPoint.y, maxy, DELTA) && !smallerd(bSegment->aPoint.y, miny, DELTA)) {
		if (aPoint->x == NOTSET)
			aPoint->x = bSegment->aPoint.x, aPoint->y =bSegment->aPoint.y;
 		else
			bPoint->x = bSegment->aPoint.x, bPoint->y =bSegment->aPoint.y;
	}
 	if(!greaterd(bSegment->bPoint.y, maxy, DELTA) && !smallerd(bSegment->bPoint.y, miny, DELTA))
		bPoint->x = bSegment->bPoint.x, bPoint->y =bSegment->bPoint.y;
	return;
  }
  
  if(equald(aSegment->bPoint.x-aSegment->aPoint.x, 0, DELTA)) {
	aPoint->x = aSegment->aPoint.x, aPoint->y = (bSegment->bPoint.y-bSegment->aPoint.y)*(aSegment->aPoint.x-bSegment->aPoint.x)/(bSegment->bPoint.x-bSegment->aPoint.x)+bSegment->aPoint.y;
	return;
  } 

  if(equald(bSegment->bPoint.x-bSegment->aPoint.x, 0,DELTA)) {
	aPoint->x = bSegment->aPoint.x, aPoint->y = (aSegment->bPoint.y-aSegment->aPoint.y)*(bSegment->aPoint.x-aSegment->aPoint.x)/(aSegment->bPoint.x-aSegment->aPoint.x)+aSegment->aPoint.y;
	return;
  }

  a = (aSegment->bPoint.y-aSegment->aPoint.y)/(aSegment->bPoint.x-aSegment->aPoint.x);
  c = (bSegment->bPoint.y-bSegment->aPoint.y)/(bSegment->bPoint.x-bSegment->aPoint.x);
  if(equald(a, c, DELTA)) {
	maxx=minx=aSegment->aPoint.x;
	if(greaterd(aSegment->bPoint.x,maxx, DELTA)) {maxx = aSegment->bPoint.x;}
	if(greaterd(bSegment->aPoint.x,maxx, DELTA)) {maxx = bSegment->aPoint.x;}
 	if(greaterd(bSegment->bPoint.x,maxx, DELTA)) {maxx = bSegment->bPoint.x;}
	if(smallerd(aSegment->bPoint.x,minx, DELTA)) {minx = aSegment->bPoint.x;}
	if(smallerd(bSegment->aPoint.x,minx, DELTA)) {minx = bSegment->aPoint.x;}
 	if(smallerd(bSegment->bPoint.x,minx, DELTA)) {minx = bSegment->bPoint.x;}

 	if(!greaterd(aSegment->aPoint.x, maxx, DELTA) && !smallerd(aSegment->aPoint.x, minx, DELTA)) 
		aPoint->x = aSegment->aPoint.x, aPoint->y =aSegment->aPoint.y;
	if(!greaterd(aSegment->bPoint.x, maxx, DELTA) && !smallerd(aSegment->bPoint.x, minx, DELTA)) {
		if (aPoint->x == NOTSET)
			aPoint->x = aSegment->bPoint.x, aPoint->y =aSegment->bPoint.y;
		else 
			bPoint->x = aSegment->bPoint.x, bPoint->y=aSegment->bPoint.y;
	}
	if(!greaterd(bSegment->aPoint.x, maxx,DELTA) && !smallerd(bSegment->aPoint.x, minx, DELTA)) {
		if (aPoint->x == NOTSET)
			aPoint->x = bSegment->aPoint.x, aPoint->y =bSegment->aPoint.y;
		else
			bPoint->x = bSegment->aPoint.x, bPoint->y =bSegment->aPoint.y;
	}
 	if(!greaterd(bSegment->bPoint.x, maxx, DELTA) && !smallerd(bSegment->bPoint.x, minx, DELTA)) 
		bPoint->x = bSegment->bPoint.x, bPoint->y =bSegment->bPoint.y;
  }  else {
	aPoint->x = ((aSegment->bPoint.x-aSegment->aPoint.x)*(bSegment->bPoint.x-bSegment->aPoint.x)*(bSegment->aPoint.y-aSegment->aPoint.y)+aSegment->aPoint.x*(aSegment->bPoint.y-aSegment->aPoint.y)*(bSegment->bPoint.x-bSegment->aPoint.x)-bSegment->aPoint.x*(bSegment->bPoint.y-bSegment->aPoint.y)*(aSegment->bPoint.x-aSegment->aPoint.x))/((aSegment->bPoint.y-aSegment->aPoint.y)*(bSegment->bPoint.x-bSegment->aPoint.x)-(bSegment->bPoint.y-bSegment->aPoint.y)*(aSegment->bPoint.x-aSegment->aPoint.x));
	aPoint->y = ((bSegment->bPoint.x-bSegment->aPoint.x)*(aSegment->bPoint.y-aSegment->aPoint.y)*bSegment->aPoint.y-(aSegment->bPoint.y-aSegment->aPoint.y)*(bSegment->bPoint.y-bSegment->aPoint.y)*(bSegment->aPoint.x-aSegment->aPoint.x)-(aSegment->bPoint.x-aSegment->aPoint.x)*(bSegment->bPoint.y-bSegment->aPoint.y)*aSegment->aPoint.y)/((aSegment->bPoint.y-aSegment->aPoint.y)*(bSegment->bPoint.x-bSegment->aPoint.x)-(bSegment->bPoint.y-bSegment->aPoint.y)*(aSegment->bPoint.x-aSegment->aPoint.x));
  }
}

int segment_equal_func(struct Segment *aSeg, struct Segment *bSeg)
{
	return point_equal_func(&aSeg->aPoint, &bSeg->aPoint) && 
	       point_equal_func(&aSeg->bPoint, &bSeg->bPoint);

}

/***********************************************************
 * Point structures related
 * ********************************************************/
void point_dump_func(FILE *fOutput, struct Point *aPoint)
{
	fwrite(aPoint, sizeof(double), 2, fOutput);
}


struct Point* point_load_func(FILE *fInput)
{
	struct Point *newPoint;
	newPoint = (struct Point*)malloc(sizeof(struct Point));
	fread(newPoint, sizeof(double), 2, fInput);
	return newPoint;
}

int point_equal_func(struct Point *aPoint, struct Point *bPoint)
{
	return equald(aPoint->x, bPoint->x, DELTA) && equald(aPoint->y, bPoint->y, DELTA);
}


void point_free_func(struct Point *aPoint)
{
	if(aPoint != NULL)
		free(aPoint);
}

struct Point* point_copy_func(struct Point *aPoint)
{
	struct Point *rtPoint=NULL;

	if(aPoint!=NULL) {
		rtPoint = (struct Point*)malloc(sizeof(struct Point));
		rtPoint->x = aPoint->x;
		rtPoint->y = aPoint->y;
	}
	return rtPoint;
}

void polyline_dump_func(FILE *fOutput, struct Duallist *aPolyline)
{
	duallist_dump(fOutput, aPolyline, (void(*)(FILE*,void*))point_dump_func);
}


struct Duallist* polyline_load_func(FILE *fInput)
{
	struct Duallist *newPolyline;
	newPolyline = (struct Duallist*)malloc(sizeof(struct Duallist));
	duallist_load(fInput, newPolyline, (void *(*)(FILE*))point_load_func);
	return newPolyline;
}

int polyline_equal_func(struct Duallist *aPolyline, struct Duallist *bPolyline)
{
	unsigned long i;
	int rst=1;
	struct Item *p, *q;

	if (aPolyline->nItems == bPolyline->nItems) {
		p = aPolyline->head, q = bPolyline->head;
		for (i=0; i<aPolyline->nItems; i++) {
			if (!point_equal_func((struct Point*)p->datap, (struct Point*)q->datap)) {
				rst = 0;
				break;
			}
			p = p->next, q = q->next;
		}
	}
	return rst;
}



void polyline_free_func(struct Duallist *aPolyline)
{
	if(aPolyline == NULL) return;
	duallist_destroy(aPolyline, free);
	free(aPolyline);
}


struct District *point_in_district(struct Region *region, struct Point *aPoint)
{
  int count = 0;
  struct Item *distp, *pllp, *pp;
  struct Point point1, point2;
  struct Segment seg1, seg2;

  seg1.aPoint.x = aPoint->x;
  seg1.aPoint.y = aPoint->y;
  /* the end point on the left edge of the screen */
  seg1.bPoint.x = 0;
  seg1.bPoint.y = aPoint->y;

  distp = region->districts.head;
  while(distp!=NULL) {
	pllp = ((struct District*)distp->datap)->rings.head;
	while(pllp!=NULL) {
  		count = 0;
		pp = ((struct Duallist*)pllp->datap)->head;
		while(pp!=NULL && pp->next != NULL) {
			seg2.aPoint.x = point1.x = ((struct Point*)pp->datap)->x;
			seg2.aPoint.y = point1.y = ((struct Point*)pp->datap)->y;
			seg2.bPoint.x = point2.x = ((struct Point*)pp->next->datap)->x;
			seg2.bPoint.y = point2.y = ((struct Point*)pp->next->datap)->y;

			if (is_point_on_segment(aPoint, &seg2)) 
				return distp->datap;
			if (seg2.aPoint.y != seg2.bPoint.y) {
				if (is_point_on_segment(&point1, &seg1) || is_point_on_segment(&point2, &seg1)) {
					if ( is_point_on_segment(&point1, &seg1) && (seg2.aPoint.y > seg2.bPoint.y)) count++;
					if ( is_point_on_segment(&point2, &seg1) && (seg2.bPoint.y > seg2.aPoint.y)) count++;
				} else if (are_segments_intersected(&seg1, &seg2))
					count ++;
			}
			pp = pp->next;
		}
		if(pp!=NULL && pp->next == NULL) {
			seg2.aPoint.x = point1.x = ((struct Point*)pp->datap)->x;
			seg2.aPoint.y = point1.y = ((struct Point*)pp->datap)->y;
			seg2.bPoint.x = point2.x = ((struct Point*)((struct Duallist*)pllp->datap)->head->datap)->x;
			seg2.bPoint.y = point2.y = ((struct Point*)((struct Duallist*)pllp->datap)->head->datap)->y;

			if (is_point_on_segment(aPoint, &seg2)) 
				return distp->datap;
			if (seg2.aPoint.y != seg2.bPoint.y) {
				if (is_point_on_segment(&point1, &seg1) || is_point_on_segment(&point2, &seg1)) {
					if ( is_point_on_segment(&point1, &seg1) && (seg2.aPoint.y > seg2.bPoint.y)) count++;
					if ( is_point_on_segment(&point2, &seg1) && (seg2.bPoint.y > seg2.aPoint.y)) count++;
				} else if (are_segments_intersected(&seg1, &seg2))
					count ++;
			}
		}

		if(count % 2 == 1) return distp->datap;
		pllp=pllp->next;
	}
	distp = distp->next;
  }
  return NULL;		
}

struct Cell *point_in_cell(struct Region *region, struct Point *aPoint)
{
	int i,j;
	struct Cell *aCell = NULL;

	if(region!=NULL && region->mesh!=NULL) {
		i = floor((aPoint->x - region->chosen_polygon->box.xmin)/region->cellSize);
		j = floor((aPoint->y - region->chosen_polygon->box.ymin)/region->cellSize);
		if(i>=0 && i<region->hCells && j>=0 && j<region->vCells)
			aCell = region->mesh + i*region->vCells + j;
	}
	return aCell;
}

void surroundings_from_point(struct Duallist *list, struct Region *region, struct Point *aPoint)
{
	int i,j;

	if(region!=NULL && region->mesh!=NULL) {
		i = floor((aPoint->x - region->chosen_polygon->box.xmin)/region->cellSize);
		j = floor((aPoint->y - region->chosen_polygon->box.ymin)/region->cellSize);
		if(i-1>=0&&i-1<region->hCells) {
			if(j-1>=0 && j-1<region->vCells) {
				duallist_add_to_head(list, region->mesh + (i-1)*region->vCells + (j-1));
			} 
			if(j>=0 && j<region->vCells) {
				duallist_add_to_head(list, region->mesh + (i-1)*region->vCells + j);
			}
			if(j+1>=0 && j+1<region->vCells) {
				duallist_add_to_head(list, region->mesh + (i-1)*region->vCells + (j+1));
			}
		}	
		if(i+1>=0&&i+1<region->hCells) {
			if(j-1>=0 && j-1<region->vCells) {
				duallist_add_to_head(list, region->mesh + (i+1)*region->vCells + (j-1));
			} 
			if(j>=0 && j<region->vCells) {
				duallist_add_to_head(list, region->mesh + (i+1)*region->vCells + j);
			}
			if(j+1>=0 && j+1<region->vCells) {
				duallist_add_to_head(list, region->mesh + (i+1)*region->vCells + (j+1));
			}
		}	
		if(i>=0&&i<region->hCells) {
			if(j-1>=0 && j-1<region->vCells) {
				duallist_add_to_head(list, region->mesh + i*region->vCells + (j-1));
			} 
			if(j+1>=0 && j+1<region->vCells){
				duallist_add_to_head(list, region->mesh + i*region->vCells + (j+1));
			}
			if(j>=0 && j<region->vCells) {
				duallist_add_to_head(list, region->mesh + i*region->vCells + j);
			}
		}	
	}
}

int is_point_in_polygon(struct Point *aPoint, struct Polygon *chosen_polygon)
{
  int count = 0, i = 0;
  struct Item *p;
  struct Point point1, point2;
  struct Segment seg1, seg2;

  seg1.aPoint.x = aPoint->x;
  seg1.aPoint.y = aPoint->y;
  /* the end point on the left edge of the screen */
  seg1.bPoint.x = 0;
  seg1.bPoint.y = aPoint->y;

  p = chosen_polygon->points.head;
  for(i=0;i<chosen_polygon->points.nItems-1;i++) {
  	seg2.aPoint.x = point1.x = ((struct Point*)p->datap)->x;
        seg2.aPoint.y = point1.y = ((struct Point*)p->datap)->y;
	seg2.bPoint.x = point2.x = ((struct Point*)p->next->datap)->x;
	seg2.bPoint.y = point2.y = ((struct Point*)p->next->datap)->y;

	if (is_point_on_segment(aPoint, &seg2)) 
		return 1;
	if (seg2.aPoint.y != seg2.bPoint.y) {
		if (is_point_on_segment(&point1, &seg1) || is_point_on_segment(&point2, &seg1)) {
			if ( is_point_on_segment(&point1, &seg1) && (seg2.aPoint.y > seg2.bPoint.y)) count++;
			if ( is_point_on_segment(&point2, &seg1) && (seg2.bPoint.y > seg2.aPoint.y)) count++;
		} else if (are_segments_intersected(&seg1, &seg2))
			count ++;
	}
	p = p->next;
  }
  if(count % 2 == 1) return 1;
  else return 0;		
}

int is_cell_in_polygon(struct Cell *aCell, struct Polygon *chosen_polygon)
{
	struct Point aPoint;

	if(aCell && chosen_polygon) {
		aPoint.x = aCell->box.xmin;
		aPoint.y = aCell->box.ymin;
		if(is_point_in_polygon(&aPoint, chosen_polygon))
			return 1;

		aPoint.x = aCell->box.xmin;
		aPoint.y = aCell->box.ymax;
		if(is_point_in_polygon(&aPoint, chosen_polygon))
			return 1;

		aPoint.x = aCell->box.xmax;
		aPoint.y = aCell->box.ymin;
		if(is_point_in_polygon(&aPoint, chosen_polygon))
			return 1;
		
		aPoint.x = aCell->box.xmax;
		aPoint.y = aCell->box.ymax;
		if(is_point_in_polygon(&aPoint, chosen_polygon))
			return 1;
	}
	return 0;
}


int is_polyline_in_polygon(struct Duallist *polyline, struct Polygon *chosen_polygon)
{
	struct Item *aItem;

	if(polyline && chosen_polygon) {
		aItem = polyline->head;
		while(aItem) {
			if(!is_point_in_polygon((struct Point*)aItem->datap, chosen_polygon))
				return 0;
			aItem = aItem->next;
		}
		return 1;
	}
	return 0;
}


int is_point_on_segment(struct Point *aPoint, struct Segment* aSegment)
{
  double dist;

  dist =distance_point_to_segment(aPoint, aSegment, NULL);
  if (equald(dist, 0, PRECISION)) return 1;
  else return 0;
}


int is_point_in_box(struct Point *aPoint, struct Box *aBox)
{ if(greaterd(aPoint->x,aBox->xmin, DELTA) && smallerd(aPoint->x,aBox->xmax, DELTA) && greaterd(aPoint->y,aBox->ymin, DELTA) && smallerd(aPoint->y,aBox->ymax, DELTA) ) 
	return 1; 
  else 
	return 0; 
}




/***********************************************************
 * Other functions 
 * ********************************************************/

/* 
 * Calculate the distance from a point to a segment
 * in meters; if crossPoint is not NULL, set nearestPoint
 * with the coordinates of the nearest point on the segment
 * from the point.
 */
double distance_point_to_segment(struct Point *aPoint, struct Segment*aSegment, struct Point *crossPoint)
{
	double dist, a;
	struct Point cPoint;

	if(aSegment->aPoint.x == aSegment->bPoint.x) {
		cPoint.x = aSegment->aPoint.x;
		cPoint.y = aPoint->y;
	} else if(aSegment->aPoint.y == aSegment->bPoint.y) {
		cPoint.x = aPoint->x;
		cPoint.y = aSegment->aPoint.y;
	} else {
		a = (aSegment->bPoint.y-aSegment->aPoint.y)/(aSegment->bPoint.x-aSegment->aPoint.x);
		cPoint.x = (a * a * aSegment->aPoint.x + a * (aPoint->y - aSegment->aPoint.y) + aPoint->x)/(a*a+1);
		cPoint.y = a * (cPoint.x - aSegment->aPoint.x) + aSegment->aPoint.y;
	}
		
	if(MIN(aSegment->aPoint.x, aSegment->bPoint.x) <= cPoint.x &&
	   MAX(aSegment->aPoint.x, aSegment->bPoint.x) >= cPoint.x &&
	   MIN(aSegment->aPoint.y, aSegment->bPoint.y) <= cPoint.y &&
	   MAX(aSegment->aPoint.y, aSegment->bPoint.y) >= cPoint.y ) {
		if (crossPoint != NULL) {
			crossPoint->x = cPoint.x;
			crossPoint->y = cPoint.y;
		} 
		dist = distance_in_meter(aPoint->x, aPoint->y, cPoint.x, cPoint.y);
	} else if (fabs(aSegment->aPoint.x - cPoint.x) <= fabs(aSegment->bPoint.x - cPoint.x) &&
		   fabs(aSegment->aPoint.y - cPoint.y) <= fabs(aSegment->bPoint.y - cPoint.y)) {
		if (crossPoint != NULL) {
			crossPoint->x = aSegment->aPoint.x;
			crossPoint->y = aSegment->aPoint.y;
		} 
		dist = distance_in_meter(aPoint->x, aPoint->y, aSegment->aPoint.x, aSegment->aPoint.y);
	} else {
		if (crossPoint != NULL) {
			crossPoint->x = aSegment->bPoint.x;
			crossPoint->y = aSegment->bPoint.y;
		} 
		dist = distance_in_meter(aPoint->x, aPoint->y, aSegment->bPoint.x, aSegment->bPoint.y);
	}
	
	return dist;
}


double distance_point_to_polyline(struct Point *aPoint, const struct Duallist *polyline, struct Point *crossPoint, struct Segment* onSegment)
{
  struct Item *p;
  struct Point point;
  struct Segment aSegment;
  double dist, dmin;
  int first = 1;

  p = polyline->head;
  while(p->next!=NULL) {
	aSegment.aPoint.x = ((struct Point*)p->datap)->x, aSegment.aPoint.y = ((struct Point*)p->datap)->y;
	aSegment.bPoint.x = ((struct Point*)p->next->datap)->x, aSegment.bPoint.y = ((struct Point*)p->next->datap)->y;
	dist = distance_point_to_segment(aPoint, &aSegment, &point);
	if(first) {
		dmin = dist;
		if(crossPoint)
			crossPoint->x = point.x, crossPoint->y = point.y;
		if(onSegment) {
			onSegment->aPoint.x = aSegment.aPoint.x, onSegment->aPoint.y = aSegment.aPoint.y;
			onSegment->bPoint.x = aSegment.bPoint.x, onSegment->bPoint.y = aSegment.bPoint.y;
		}
		first --;
	} else if(dist < dmin) {
		dmin = dist;
		if(crossPoint)
			crossPoint->x = point.x, crossPoint->y = point.y;
		if(onSegment) {
			onSegment->aPoint.x = aSegment.aPoint.x, onSegment->aPoint.y = aSegment.aPoint.y;
			onSegment->bPoint.x = aSegment.bPoint.x, onSegment->bPoint.y = aSegment.bPoint.y;
		}
	}
	p = p->next;
  }
  return dmin;
}


/* distance in meters on the earth */
double distance_in_meter(double lng1, double lat1, double lng2, double lat2)
{
  double x, y;
  x = (lng2 - lng1) * M_PI * RADIUS_A * cos(((lat1+lat2)/2) * M_PI / 180) / 180;
  y = (lat2 - lat1) * M_PI * RADIUS_A / 180; 
  return sqrt(x*x + y*y);
}

/* distance in pixels on the screen */
double distance_in_pixel(double x1, double y1, double x2, double y2)
{
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

double distance_in_latitude(double dist)
{
	return dist*180/(M_PI*RADIUS_A);
}


double polyline_length(struct Duallist *points)
{
	double retdist = 0;
	struct Item *p;

	if(points==NULL) return -1;
	p = points->head;
	while(p != NULL) {
		if(p->next != NULL) {
			retdist += distance_in_meter(((struct Point*)p->datap)->x,
						  ((struct Point*)p->datap)->y,
						  ((struct Point*)p->next->datap)->x,
						  ((struct Point*)p->next->datap)->y);
		}
		p = p->next;
	}

	return retdist;
}


void point_cut_polyline(const struct Duallist *aPolyline, struct Point *cutPoint, struct Duallist *aPart, struct Duallist *bPart)
{
	struct Point crossPoint, *newPoint;
	struct Segment crossSegment, aSegment;
	struct Item *p;

	if(aPart != NULL)
		duallist_init(aPart);
	if(bPart != NULL)
		duallist_init(bPart);

	distance_point_to_polyline(cutPoint, aPolyline, &crossPoint, &crossSegment);

	p = aPolyline->head;
	while(p->next!=NULL) {
		aSegment.aPoint.x = ((struct Point*)p->datap)->x, aSegment.aPoint.y = ((struct Point*)p->datap)->y;
		aSegment.bPoint.x = ((struct Point*)p->next->datap)->x, aSegment.bPoint.y = ((struct Point*)p->next->datap)->y;
		if(aPart) {
			newPoint = (struct Point*)malloc(sizeof(struct Point));
			newPoint->x = ((struct Point*)p->datap)->x, newPoint->y = ((struct Point*)p->datap)->y;
			duallist_add_to_tail(aPart, newPoint);
		}
		if(segment_equal_func(&aSegment, &crossSegment)) 
			break;
		p = p->next;
  	}

	p = p->next;
	while(p!=NULL) {
		if(bPart) {
			newPoint = (struct Point*)malloc(sizeof(struct Point));
			newPoint->x = ((struct Point*)p->datap)->x, newPoint->y = ((struct Point*)p->datap)->y;
			duallist_add_to_tail(bPart, newPoint);
		}
		p = p->next;
	}

	if(aPart) {
		newPoint = (struct Point*)malloc(sizeof(struct Point));
		newPoint->x = crossPoint.x, newPoint->y = crossPoint.y;
		if(aPart->nItems == 0 || (aPart->nItems > 0 && !point_equal_func(newPoint, (struct Point*)aPart->head->prev->datap)))
			duallist_add_to_tail(aPart, newPoint);
		else
			free(newPoint);
	}

	if(bPart) {
		newPoint = (struct Point*)malloc(sizeof(struct Point));
		newPoint->x = crossPoint.x, newPoint->y = crossPoint.y;
		if(bPart->nItems == 0 || (bPart->nItems > 0 && !point_equal_func(newPoint, (struct Point*)bPart->head->datap)))
			duallist_add_to_head(bPart, newPoint);
		else
			free(newPoint);
	}
}


double distance_to_head_cross(struct Road *aRoad, struct Point *fromPoint)
{
	struct Duallist aPart;
	double rt;

	point_cut_polyline(&aRoad->points, fromPoint, &aPart, NULL);
	rt = polyline_length(&aPart);
	duallist_destroy(&aPart, (void(*)(void*))point_free_func);
	return rt;
}	

double distance_to_tail_cross(struct Road *aRoad, struct Point *fromPoint)
{
	struct Duallist aPart;
	double rt;

	point_cut_polyline(&aRoad->points, fromPoint, NULL, &aPart);
	rt = polyline_length(&aPart);
	duallist_destroy(&aPart, (void(*)(void*))point_free_func);
	return rt;
}	

double distance_on_path(struct Path *aPath, struct Point *fromPoint, struct Point *toPoint)
{
	struct Duallist *polyline;
	double rt = 0;
	
	polyline = polyline_on_path(aPath, fromPoint, toPoint);
	if(polyline) {
		rt = polyline_length(polyline);
		duallist_destroy(polyline, (void(*)(void*))point_free_func);
		free(polyline);
	}
	return rt;
}


void remove_loop(struct Duallist *polyline)
{
  struct Item *p, *q;
  struct Segment seg1, seg2;
  struct Point *newp, bpoint;
  int i;
 
  if (polyline->nItems < 2) return;

  p = polyline->head;
  seg1.aPoint.x = ((struct Point*)p->prev->prev->datap)->x;
  seg1.aPoint.y = ((struct Point*)p->prev->prev->datap)->y;
  seg1.bPoint.x = ((struct Point*)p->prev->datap)->x;
  seg1.bPoint.y = ((struct Point*)p->prev->datap)->y;
  if (point_equal_func(&seg1.aPoint, &seg1.bPoint)) {
	free(duallist_pick_item(polyline, p->prev));
	return;
  }
 
  if(polyline->nItems > 2) {
	seg2.aPoint.x = ((struct Point*)p->prev->prev->prev->datap)->x;
	seg2.aPoint.y = ((struct Point*)p->prev->prev->prev->datap)->y;
	seg2.bPoint.x = ((struct Point*)p->prev->prev->datap)->x;
	seg2.bPoint.y = ((struct Point*)p->prev->prev->datap)->y;
	if(is_point_on_segment(&seg1.bPoint, &seg2) || is_point_on_segment(&seg2.aPoint, &seg1)) {
		free(duallist_pick_item(polyline, p->prev->prev));
		return;
	}
	for(i = 0; i<polyline->nItems-3; i++) {
		seg2.aPoint.x = ((struct Point*)p->datap)->x;
		seg2.aPoint.y = ((struct Point*)p->datap)->y;
		seg2.bPoint.x = ((struct Point*)p->next->datap)->x;
		seg2.bPoint.y = ((struct Point*)p->next->datap)->y;

		if(are_segments_intersected(&seg1, &seg2)) {
			newp = (struct Point*)malloc(sizeof(struct Point));
			segment_cut_segment(&seg1, &seg2, newp, &bpoint);
			q = p->next;
			while(q != NULL) {
				free(duallist_pick_item(polyline, q));
				q = p->next;
			}
			duallist_add_to_tail(polyline, newp);
			newp = (struct Point*)malloc(sizeof(struct Point));
			newp->x = seg1.bPoint.x, newp->y = seg1.bPoint.y;
			duallist_add_to_tail(polyline, newp);
			return;
		}
		p = p->next;
	}
  }
  
}

/* 
 * Return the angle of a vetor p1->p2, [0-360]
 */
double angle_between(double x1, double y1, double x2, double y2)
{
  double dx, dy, angle = -1;


  dx = x2-x1, dy = y2-y1;
  if (equald(dx,0, DELTA)) {
	if( equald(dy, 0, DELTA)) return 0;
	else if (greaterd(dy, 0, DELTA)) return 90;
	else return 270;
  }
  angle = atan(ABS(dy)/ABS(dx));
  if(greaterd(dx,0,DELTA) && (greaterd(dy,0,DELTA)||equald(dy,0,DELTA)) )
	return angle*180/M_PI;
  else if (smallerd(dx, 0,DELTA) && (greaterd(dy,0,DELTA)||equald(dy,0,DELTA)) )
	return 180-angle*180/M_PI;
  else if (smallerd(dx, 0,DELTA) && (smallerd(dy,0,DELTA)||equald(dy,0,DELTA)) )
	return 180+angle*180/M_PI;
  else if (greaterd(dx, 0,DELTA) && (smallerd(dy,0,DELTA)||equald(dy,0,DELTA)) )
	return 360-angle*180/M_PI;
  return -1;
}

/* return the interangle between two angles, [0-180]*/
double inter_angle(double angle1, double angle2)
{
	double big, small, inter;
	big = MAX(angle1, angle2);
	small = MIN(angle1, angle2);
	inter = big - small;
	if(inter > 180)
		inter = 360 - inter;
	return inter;
} 

int are_boxes_intersected(struct Box *aBox, struct Box *bBox)
{
	if(ABS((aBox->xmin+aBox->xmax)/2-(bBox->xmin+bBox->xmax)/2) <= (aBox->xmax+bBox->xmax-aBox->xmin-bBox->xmin)/2 
	&& ABS((aBox->ymin+aBox->ymax)/2-(bBox->ymin+bBox->ymax)/2) <= (aBox->ymax+bBox->ymax-aBox->ymin-bBox->ymin)/2)
		return 1; 
	return 0;
}

int is_box_within_box(struct Box *aBox, struct Box *bBox)
{
	if(!greaterd(bBox->xmin, aBox->xmin, DELTA) && !greaterd(bBox->ymin, aBox->ymin, DELTA)
	&& !smallerd(bBox->xmax, aBox->xmax, DELTA) && !smallerd(bBox->ymax, aBox->ymax, DELTA))
		return 1;
	return 0;

}

int turns_on_path(struct Path *aPath)
{
	struct Item *aItem;
	double interAngl;
	int turns = 0;

	if(aPath == NULL) return -1;
	aItem = aPath->roads.head;
	while(aItem != NULL) {
		if(aItem->next != NULL) {
			interAngl = inter_angle(((struct Road*)aItem->datap)->tailEndAngle,
						((struct Road*)aItem->next->datap)->headEndAngle);
			if(interAngl > 45 && interAngl < 135)
				turns ++;
			if(interAngl > 135)
				turns += 2;
		}
		aItem = aItem->next;
	}
	return turns;	
}

void ref_free_func(struct Ref *aRef)
{
	if(aRef == NULL) return;
	duallist_pick_item(aRef->byWho, aRef->at);
	free(aRef);
}  


