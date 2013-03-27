#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<limits.h>
#include"geometry.h"
#include"common.h"

int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	struct Region *region, *aRegion, *rtRegion;
	char buf[256];

	struct Duallist crossNumbers, *pathPoints;

	struct Item *aItem, *bItem, *tItem, *pItem, *p, *q;
	struct Path *path;
	struct Point *newPoint;

	struct Cross *bCross, *aCross, *aRtCross, *bRtCross;
	struct Road *aNewRoad, *bNewRoad;

	int numRsu = INT_MAX, *crossNumber, line = 1, aNumber;
	unsigned long pathCount = 0;
	double minLeng, maxLeng;

	if(argc < 3) {
	      printf("%s is used to construct the corresponding RSU topology given the deployment.\n", argv[0]);
	      printf("Usage: %s [-n #rsu] [-l line|path] .map vetexlist\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] =='-') {
	  switch ( argv[1][1]) {
	    case 'n':
		numRsu = atoi(argv[2]);
		if(numRsu<1)
			numRsu = 1;
		argc-=2;
		argv+=2;
		break;
	
	    case 'l':
		if(strcmp(argv[2], "path")==0)
			line = 0;
		argc-=2;
		argv+=2;
		break;
	

	    default:
	      printf("Bad option %s\n", argv[1]);
	      printf("Usage: %s [-n #rsu] .map vetexlist\n", argv[0]);
	      exit(1);
	    }
	}
	
	duallist_init(&crossNumbers);	
	if((fsource=fopen(argv[2], "r"))!=NULL) {
		fscanf(fsource, "%d", &aNumber);
		while( numRsu>0 && aNumber!=-1) {
			crossNumber = (int*)malloc(sizeof(int));
			*crossNumber = aNumber;
			duallist_add_to_tail(&crossNumbers, crossNumber);
			numRsu--;
			if (numRsu) {
				fscanf(fsource, "%d", &aNumber);
			}
		}
		fclose(fsource);
	}

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      	region = region_load_func(fsource, NULL, -1, -1);
	      	fclose(fsource);
	      	if(region == NULL)
			exit(3);
	}

	/* build up the rsu topoloty with .map format by hand */
	rtRegion = (struct Region*)malloc(sizeof(struct Region));
	rtRegion->cellSize = region->cellSize;
	rtRegion->chosen_polygon = polygon_copy_func(region->chosen_polygon);
	setup_cells_in_region(rtRegion);
	duallist_init(&rtRegion->roads);
	duallist_init(&rtRegion->crosses);
	duallist_init(&rtRegion->districts);
	duallist_init(&rtRegion->rivers);


	aItem = crossNumbers.head;
	while(aItem!=NULL) {
		tItem = duallist_find(&rtRegion->crosses, (int*)aItem->datap, (int(*)(void*,void*))cross_has_number);
		if(tItem == NULL) {
			tItem = duallist_find(&region->crosses, (int*)aItem->datap, (int(*)(void*,void*))cross_has_number);
			aRtCross = cross_copy_func((struct Cross*)tItem->datap);
			duallist_add_to_tail(&rtRegion->crosses, aRtCross);	
		} else
			aRtCross = (struct Cross*)tItem->datap;

		bItem = aItem->next;
		while(bItem!=NULL) {
			tItem = duallist_find(&rtRegion->crosses, (int*)bItem->datap, (int(*)(void*,void*))cross_has_number);
			if(tItem == NULL) {
				tItem = duallist_find(&region->crosses, (int*)bItem->datap, (int(*)(void*,void*))cross_has_number);
				bRtCross = cross_copy_func((struct Cross*)tItem->datap);
				duallist_add_to_tail(&rtRegion->crosses, bRtCross);	
			} else
				bRtCross = (struct Cross*)tItem->datap;

			aRegion = region_copy_func(region);
			pItem = crossNumbers.head;
			while (pItem!=NULL) {
				if(pItem!=aItem && pItem!=bItem) {
					tItem = duallist_find(&aRegion->crosses, (int*)pItem->datap, (int(*)(void*,void*))cross_has_number);
					remove_cross(aRegion, (struct Cross*)tItem->datap);
				}
				pItem = pItem->next;
			}
			tItem = duallist_find(&aRegion->crosses, (int*)aItem->datap, (int(*)(void*,void*))cross_has_number);
			aCross = (struct Cross*)tItem->datap;
			tItem = duallist_find(&aRegion->crosses, (int*)bItem->datap, (int(*)(void*,void*))cross_has_number);
			bCross = (struct Cross*)tItem->datap;
			
			path = find_shortest_path(aRegion, aCross,bCross);
			if(path!=NULL) {
				printf("%d %d\n", aCross->number, bCross->number);
				pathCount ++;
				if(pathCount == 1) {
					minLeng = path->length;
					maxLeng = path->length;
				} else {
					if(minLeng > path->length)
						minLeng = path->length;
					if(maxLeng < path->length)
						maxLeng = path->length;
				}
				pathPoints = polyline_on_path(path, NULL, NULL);
				
				aNewRoad = (struct Road*)malloc(sizeof(struct Road));
				aNewRoad->id = pathCount;
				aNewRoad->width = ROAD_WIDTH;
				aNewRoad->length = path->length;
				if(line) {
					duallist_init(&aNewRoad->origPoints);
					newPoint = (struct Point*)malloc(sizeof(struct Point));
					newPoint->x = aCross->gPoint.x;
					newPoint->y = aCross->gPoint.y;
					duallist_add_to_tail(&aNewRoad->origPoints, newPoint);
					newPoint = (struct Point*)malloc(sizeof(struct Point));
					newPoint->x = bCross->gPoint.x;
					newPoint->y = bCross->gPoint.y;
					duallist_add_to_tail(&aNewRoad->origPoints, newPoint);
					duallist_copy(&aNewRoad->points, &aNewRoad->origPoints, (void*(*)(void*))point_copy_func);
			
				} else {
					duallist_copy(&aNewRoad->origPoints, pathPoints, (void*(*)(void*))point_copy_func);
					duallist_copy(&aNewRoad->points, pathPoints, (void*(*)(void*))point_copy_func);
				}
				p = aNewRoad->points.head;
				aNewRoad->headEndAngle = angle_between(((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, ((struct Point*)p->next->datap)->x, ((struct Point*)p->next->datap)->y);
  				p = aNewRoad->points.head->prev;
				aNewRoad->tailEndAngle = angle_between(((struct Point*)p->prev->datap)->x, ((struct Point*)p->prev->datap)->y, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y);
				  
				p = aNewRoad->points.head;
				aNewRoad->box.xmin = aNewRoad->box.xmax = ((struct Point*)p->datap)->x;
				aNewRoad->box.ymin = aNewRoad->box.ymax = ((struct Point*)p->datap)->y;
				q = p->next;
				while(q != NULL) {
				      aNewRoad->length = aNewRoad->length + distance_in_meter(((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, ((struct Point*)q->datap)->x, ((struct Point*)q->datap)->y);
				      aNewRoad->box.xmin = MIN(((struct Point*)q->datap)->x, aNewRoad->box.xmin);
				      aNewRoad->box.ymin = MIN(((struct Point*)q->datap)->y, aNewRoad->box.ymin);
				      aNewRoad->box.xmax = MAX(((struct Point*)q->datap)->x, aNewRoad->box.xmax);
				      aNewRoad->box.ymax = MAX(((struct Point*)q->datap)->y, aNewRoad->box.ymax);
				      p = q;
				      q = q->next;
				}
				/* note that pointers are holding crosses' numbers */
				aNewRoad->headEnd = (void*)aRtCross->number;
				aNewRoad->tailEnd = (void*)bRtCross->number;
				duallist_init(&aNewRoad->scraps);
				duallist_init(&aNewRoad->refs);
				duallist_add_to_tail(&rtRegion->roads, aNewRoad);


				/* another direction (though this may not be necessary bur for consistent reason) */
				pathCount ++;
				bNewRoad = (struct Road*)malloc(sizeof(struct Road));
				bNewRoad->id = pathCount;
				bNewRoad->width = ROAD_WIDTH;
				bNewRoad->length = path->length;
				duallist_reverse_copy(&bNewRoad->origPoints, &aNewRoad->origPoints, (void*(*)(void*))point_copy_func);
				duallist_reverse_copy(&bNewRoad->points, &aNewRoad->points, (void*(*)(void*))point_copy_func);
				p = bNewRoad->points.head;
				bNewRoad->headEndAngle = angle_between(((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, ((struct Point*)p->next->datap)->x, ((struct Point*)p->next->datap)->y);
  				p = bNewRoad->points.head->prev;
				bNewRoad->tailEndAngle = angle_between(((struct Point*)p->prev->datap)->x, ((struct Point*)p->prev->datap)->y, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y);
				  
				bNewRoad->box.xmin = aNewRoad->box.xmin;
				bNewRoad->box.xmax = aNewRoad->box.xmax;
				bNewRoad->box.ymin = aNewRoad->box.ymin;
				bNewRoad->box.ymax = aNewRoad->box.ymax;
				/* note that pointers are holding crosses' numbers */
				bNewRoad->headEnd = (void*)bRtCross->number;
				bNewRoad->tailEnd = (void*)aRtCross->number;
				duallist_init(&bNewRoad->scraps);
				duallist_init(&bNewRoad->refs);
				duallist_add_to_tail(&rtRegion->roads, bNewRoad);

				duallist_destroy(pathPoints, free);
				free(pathPoints);
				path_free_func(path);
			}
			region_free_func(aRegion);
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}

	setup_roads_and_crosses_in_region(rtRegion);	

	if(line)	
		sprintf(buf, "v%ld_e%ld_%.2lf_line.vmap", crossNumbers.nItems, pathCount, minLeng);
	else
		sprintf(buf, "v%ld_e%ld_%.2lf_path.vmap", crossNumbers.nItems, pathCount, minLeng);

  	if((fdump=fopen(buf, "wb")) != NULL) {
		region_dump_func(fdump, rtRegion);
	}
	region_free_func(rtRegion);
	fclose(fdump);

	duallist_destroy(&crossNumbers, free);
	region_free_func(region);
	return 0;
}
