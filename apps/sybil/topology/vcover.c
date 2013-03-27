#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include"common.h"
#include"geometry.h"


#define METHOD_MAXDEGREE 0
#define METHOD_RANDOMEDGE 1

struct Cross_record
{
  int number;
  double x;
  double y;
};


struct Mesh
{
  struct Duallist *duallists;
  double cellSize;
  int vCells;
  int hCells;
  double xmin;
  double ymin;
};

void vicinity_from_point(struct Duallist *list, struct Mesh *aMesh, struct Point *aPoint);
void add_chosen_cross(struct Cross *aCross, struct Mesh *aMesh);
int is_cross_far_away(struct Cross *aCross, struct Mesh *aMesh, double distance);


int main(int argc, char **argv)
{
	FILE *fsource;
	struct Region *region;

	struct Hashtable *dgrTables, *currentTable;
	struct Duallist crossHolder;
	struct Item *aItem;
	struct Cross *currentCross, *aCross;
	struct Road *aRoad;
	int method = 0;
	unsigned int rand_seed = time(0);

	int i, j;
	unsigned long rndIndex, count, total, dnum, cover;
	double distance = -1;

	struct Mesh aMesh;

	if(argc < 2) {
	      printf("%s is used to find a vetex cover of a region.\n", argv[0]);
	      printf("Usage: %s [-o maxdegree distance | randomedge ] [-s rand_seed] .map\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] =='-') {
	  switch ( argv[1][1]) {
	    case 'o':
		if (strcmp(argv[2], "maxdegree")==0) {
			method = METHOD_MAXDEGREE;
			distance = atof(argv[3]);
			argc-=3;
			argv+=3;
		} else if (strcmp(argv[2], "randomedge") == 0) {
			method = METHOD_RANDOMEDGE;
			argc-=2;
			argv+=2;
		}

		break;

	    case 's':
		rand_seed = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	    default:
	      printf("Bad option %s\n", argv[1]);
	      printf("Usage: %s [-o maxdegree | randomedge] .map\n", argv[0]);
	      exit(1);
	    }
	}

	srand(rand_seed);

	region = NULL;
	if((fsource=fopen(argv[1], "rb"))!=NULL) {
	      region = region_load_func(fsource, NULL, -1, -1);
	      fclose(fsource);
	}
	

	total = region->crosses.nItems;
	
	if(method == METHOD_MAXDEGREE) {
		if(region->mesh && distance > 0) {
			aMesh.cellSize = distance_in_latitude(distance);
			aMesh.vCells = (region->chosen_polygon->box.ymax - region->chosen_polygon->box.ymin)/aMesh.cellSize + 1;
			aMesh.hCells = (region->chosen_polygon->box.xmax - region->chosen_polygon->box.xmin)/aMesh.cellSize + 1;
			aMesh.xmin = region->chosen_polygon->box.xmin;
			aMesh.ymin = region->chosen_polygon->box.ymin;
			aMesh.duallists = (struct Duallist*)malloc(sizeof(struct Duallist)*aMesh.vCells*aMesh.hCells);
			for(i = 0; i<aMesh.hCells; i++)
				for(j = 0; j<aMesh.vCells;j++) 
					duallist_init(aMesh.duallists+i*aMesh.vCells+j);
		} else 
			aMesh.duallists = NULL;

		/* first, sort degrees of crosses in hashtables */
		dgrTables = (struct Hashtable*)malloc(sizeof(struct Hashtable)*region->maxdgr);
		for(i=0;i<region->maxdgr;i++) 
			hashtable_init(dgrTables+i, MIN(10000, (region->maxdgr-i)*500), (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))cross_has_number);

		if(region!=NULL) {
			aItem = region->crosses.head;
			while(aItem != NULL) {
				aCross = (struct Cross*)aItem->datap;
				hashtable_add(dgrTables + aCross->inRoads.nItems+aCross->outRoads.nItems-1, &aCross->number, aCross);
				aItem = aItem->next;
			}
		}
		duallist_init(&crossHolder);

		/* then, pick the maximum degree cross and the chosen cross is at least certain distance away from others */
		cover = 0;
		count = 0;
		dnum = 0;
		for (i = region->maxdgr-1; i>=0 ; i--) {
			dnum += dgrTables[i].count;
		}
		while(dnum > 0) {
			for (i = region->maxdgr-1; i>=0 ; i--) {
				if(dgrTables[i].count > 0) {
					currentTable = dgrTables+i;
					break;
				}
			}
			currentCross = NULL;
			rndIndex = rand()%currentTable->count;
			for(i = 0; i<currentTable->size; i++) {
				aItem = currentTable->head[i];
				while(aItem != NULL) {
					if(rndIndex == 0) {
						currentCross = (struct Cross*)aItem->datap;
						break;
					} else {
						rndIndex --;
					}
					aItem = aItem->next;
				}
				if(currentCross != NULL)
					break;
			}
			hashtable_pick(currentTable, &currentCross->number);	
			duallist_pick(&region->crosses, &currentCross->number, (int(*)(void*,void*))cross_has_number);

			if(is_cross_far_away(currentCross, &aMesh, distance)) {
				cover ++;
				printf("%d ", currentCross->number);

				aItem = currentCross->inRoads.head;
				while (aItem != NULL) {
					aRoad = (struct Road*) aItem->datap;
					aCross = aRoad->headEnd;
					remove_road(region, aRoad);
					if(hashtable_pick(dgrTables+aCross->inRoads.nItems+aCross->outRoads.nItems, &aCross->number))
						if(aCross->inRoads.nItems+aCross->outRoads.nItems>0) {
							hashtable_add(dgrTables+aCross->inRoads.nItems+aCross->outRoads.nItems-1, &aCross->number, aCross);
						}
					aItem = aItem->next;
				}	

				aItem = currentCross->outRoads.head;
				while (aItem != NULL) {
					aRoad = (struct Road*) aItem->datap;
					aCross = aRoad->tailEnd;
					remove_road(region, aRoad);
					if(hashtable_pick(dgrTables+aCross->inRoads.nItems+aCross->outRoads.nItems, &aCross->number))
						if(aCross->inRoads.nItems+aCross->outRoads.nItems>0) {
							hashtable_add(dgrTables+aCross->inRoads.nItems+aCross->outRoads.nItems-1, &aCross->number, aCross);
						}
					aItem = aItem->next;
				}	

				add_chosen_cross(currentCross, &aMesh);
				cross_free_func(currentCross);
			} else {
				duallist_add_to_tail(&crossHolder, currentCross);

			} 
			
			dnum = 0;
			for (i = region->maxdgr-1; i>=0 ; i--) {
				dnum += dgrTables[i].count;
			}
			
		}

		for(i=0;i<region->maxdgr;i++) 
			hashtable_destroy(dgrTables+i, NULL);
		free(dgrTables);

		/* we continue to output the rest crosses after the vetex cover finding */
		count = cover;
		while(region->crosses.nItems > 0) {
			rndIndex = rand()%region->crosses.nItems;
			aItem = region->crosses.head;
			for(i = 0; i<rndIndex; i++) {
				aItem = aItem->next;
			}
			currentCross = (struct Cross*)aItem->datap;	
			duallist_pick(&region->crosses, &currentCross->number, (int(*)(void*,void*))cross_has_number);

			if(is_cross_far_away(currentCross, &aMesh, distance)) {
				count ++;
				printf("%d ", currentCross->number);
				add_chosen_cross(currentCross, &aMesh);
				cross_free_func(currentCross);
			} else {
				duallist_add_to_tail(&crossHolder, currentCross);
			} 
		}
		
		if(region->roads.nItems)	
			printf("-1\nNo vetex cover found, chosen: %ld, total size: %ld\n", count, total);
		else
			printf("-1\nCover size: %ld, chosen: %ld, total size: %ld\n", cover, count, total);

		duallist_add_to_tail(&region->crosses, duallist_pick_head(&crossHolder));
		if(aMesh.duallists) {
			for(i = 0; i<aMesh.hCells; i++)
				for(j = 0; j<aMesh.vCells;j++) 
					duallist_destroy(aMesh.duallists+i*aMesh.vCells+j, free);
		}

	/* random pick up an edge from the region */
	} else {
		count = 0;
		while(region->roads.nItems > 0) {
			rndIndex = rand()%region->roads.nItems;
			aItem = region->roads.head;	
			for(i = 0; i<rndIndex; i++) {
				aItem = aItem->next;
			}

			currentCross = ((struct Road*)aItem->datap)->headEnd;
			aCross = ((struct Road*)aItem->datap)->tailEnd;
			printf("%d %d ", currentCross->number, aCross->number);
			remove_cross(region, currentCross);
			remove_cross(region, aCross);
			count += 2;
		}

		while(region->crosses.nItems) {
			rndIndex = rand()%region->crosses.nItems;
			aItem = region->crosses.head;	
			for(i = 0; i<rndIndex; i++) {
				aItem = aItem->next;
			}
			currentCross = (struct Cross*)aItem->datap;
			printf("%d ", currentCross->number);
			remove_cross(region, currentCross);
		}
		printf("-1\nCover size: %ld, total size: %ld\n", count, total);
	}

	region_free_func(region);
	return 0;
}



int is_cross_far_away(struct Cross *aCross, struct Mesh *aMesh, double distance)
{
	struct Cross_record *aRec;
	struct Duallist *aDuallist, surs;
	struct Item *aItem, *bItem;

	if (distance == -1)
		return 1;

	duallist_init(&surs);
	vicinity_from_point(&surs, aMesh, &aCross->gPoint);
	aItem = surs.head;
	while(aItem != NULL) {
		aDuallist = (struct Duallist*)aItem->datap;
		bItem = aDuallist->head;
		while (bItem != NULL) {
			aRec = (struct Cross_record*)bItem->datap;
			if(distance_in_meter(aRec->x, aRec->y, aCross->gPoint.x, aCross->gPoint.y)<distance)
				return 0;
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
	duallist_destroy(&surs, NULL);
	return 1;
}
	

void add_chosen_cross(struct Cross *aCross, struct Mesh *aMesh)
{
	int i,j;
	struct Cross_record *newRec;

	i = (aCross->gPoint.x - aMesh->xmin)/aMesh->cellSize;
	j = (aCross->gPoint.y - aMesh->ymin)/aMesh->cellSize;

	newRec = (struct Cross_record*)malloc(sizeof(struct Cross_record));
	newRec->number = aCross->number;
	newRec->x = aCross->gPoint.x;
	newRec->y = aCross->gPoint.y;

	duallist_add_to_tail(aMesh->duallists + i*aMesh->vCells + j, newRec);
}



void vicinity_from_point(struct Duallist *list, struct Mesh *aMesh, struct Point *aPoint)
{
	int i,j;

	if(aMesh!=NULL && aMesh->duallists!=NULL) {
		i = floor((aPoint->x - aMesh->xmin)/aMesh->cellSize);
		j = floor((aPoint->y - aMesh->ymin)/aMesh->cellSize);
		if(i-1>=0&&i-1<aMesh->hCells) {
			if(j-1>=0 && j-1<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + (i-1)*aMesh->vCells + (j-1));
			} 
			if(j>=0 && j<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + (i-1)*aMesh->vCells + j);
			}
			if(j+1>=0 && j+1<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + (i-1)*aMesh->vCells + (j+1));
			}
		}	
		if(i+1>=0&&i+1<aMesh->hCells) {
			if(j-1>=0 && j-1<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + (i+1)*aMesh->vCells + (j-1));
			} 
			if(j>=0 && j<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + (i+1)*aMesh->vCells + j);
			}
			if(j+1>=0 && j+1<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + (i+1)*aMesh->vCells + (j+1));
			}
		}	
		if(i>=0&&i<aMesh->hCells) {
			if(j-1>=0 && j-1<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + i*aMesh->vCells + (j-1));
			} 
			if(j+1>=0 && j+1<aMesh->vCells){
				duallist_add_to_head(list, aMesh->duallists + i*aMesh->vCells + (j+1));
			}
			if(j>=0 && j<aMesh->vCells) {
				duallist_add_to_head(list, aMesh->duallists + i*aMesh->vCells + j);
			}
		}	
	}
}

