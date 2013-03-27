#include<stdio.h>
#include<stdlib.h>
#include"files.h"
#include"traj.h"

int main(int argc, char **argv)
{
	FILE *fsource;

	struct Duallist trajs;
	struct Item *aItem, *bItem;
	struct Trajectory *aTraj;
	struct Landmark *aLandmark, *bLandmark;

	int first;
	unsigned long count=0;
	double minTime = 0, avgTime =0, maxTime=0, interval;
	char *minName = NULL;

	if(argc < 2) {
	      printf("%s is used to check the time to travel between two RSUs recorded in .trj files.\n", argv[0]);
	      printf("Usage: %s .trj | .lst ...\n", argv[0]);
	      exit(1);
	}


	duallist_init(&trajs);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			load_source_file(fsource, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, &trajs, (void*(*)(FILE*, void*))load_traj_with_duallist, NULL, NULL, NULL,NULL,NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	first = 1;
	aItem = trajs.head;
	while(aItem != NULL ) {
		aTraj = (struct Trajectory*)aItem->datap;
		bItem = aTraj->landmarks.head;
		while (bItem != NULL && bItem->next != NULL) {
			aLandmark = (struct Landmark*)bItem->datap;
			bLandmark = (struct Landmark*)bItem->next->datap;
			if(aLandmark-> crossId != bLandmark->crossId) {
				interval = bLandmark->timestamp - aLandmark->timestamp;
				if(first) {
					first = 0;
					count = 1;
					minTime = interval;
					avgTime = interval;
					maxTime = interval;
				} else {
					count ++;
					if(interval < minTime) {
						minTime = interval;
						minName = aTraj->vName;
					}
					if(interval > maxTime)
						maxTime = interval;
					avgTime = avgTime + (interval - avgTime)/count;
				}
			}
			bItem = bItem->next;
		}
		aItem = aItem->next;
	}
				
	printf("minTime:%.0lf (%s), avgTime:%.0lf, maxTime:%.0lf among %ld landmarks\n", minTime, minName, avgTime, maxTime, count);
	duallist_destroy(&trajs, (void(*)(void*))traj_free_func);
	return 0;
}




