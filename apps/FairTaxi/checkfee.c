#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include "common.h"

#define SYN_TIME 180 

struct Run
{
	char vId[32];
	time_t start;
	time_t end;
	int duration;
	char type;
	double dist;
	double nightdist;
};

void load_runs(FILE *fruns, struct Duallist *runs)
{
	time_t start, end;
	double dist;
	struct Run *newRun;
	char buf[256], *startp, *endp, *strp;

	while(fgets(buf, 255, fruns)) {
		strp = strtok(buf, ",");
		startp = strtok(NULL, ",");	
		endp = strtok(NULL, ",");	
		strp = strtok(NULL, ",");
		dist = atof(strp);

		start = strtot(startp);
		end = strtot(endp);
		newRun = (struct Run*)malloc(sizeof(struct Run));
		newRun->start = start;
		newRun->end = end;
		newRun->duration = end - start;
		newRun->dist = dist;
		duallist_add_to_tail(runs, newRun);
	}

}



void load_recs(FILE *frecs, struct Duallist *recs)
{
	time_t start, end;
	double dist;
	struct Run *newRun;
	char buf[256], *startp, *endp, *strp;

	while(fgets(buf, 255, frecs)) {
		newRun = (struct Run*)malloc(sizeof(struct Run));

		strp = strtok(buf, ",");
		strp = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		strncpy(newRun->vId, strp, 32);

		startp = strtok(NULL, ",");	
		endp = strtok(NULL, ",");	
		strp = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		dist = atof(strp)*1000;

		start = strtot(startp);
		end = strtot(endp);
		newRun->start = start;
		newRun->end = end;
		newRun->duration = end - start;
		newRun->dist = dist;
		duallist_add_to_tail(recs, newRun);
	}
}



void compare_runs_with_recs(struct Duallist *runs, struct Duallist *recs, double cmpdist, FILE *fdull)
{
	struct Run *aRun, *aRec;
	struct Item *aItem, *bItem, *onItem;
	char buf[20];

	if(runs->nItems == 0 && recs->nItems > 0 && fdull != NULL)
		fprintf(fdull, "%s\n", ((struct Run*)recs->head->datap)->vId);

	free(duallist_pick_head(runs));	
	free(duallist_pick_tail(runs));	

	aItem = runs->head;
	onItem = recs->head;
	while(aItem != NULL ) {
		aRun = (struct Run*)aItem->datap;
		bItem = onItem;
		while(bItem != NULL) {
			aRec = (struct Run*)bItem->datap;
			if(aRun->start - aRec->start >= SYN_TIME ) {
				bItem = bItem->next;
				onItem = bItem;
			} else if(fabs(aRun->start - aRec->start) < SYN_TIME ) {
				if( fabs(aRun->duration - aRec->duration) <= 60 
				 && aRec->dist >= cmpdist 
				 && aRun->dist < aRec->dist
				 && aRun->dist != -1) {
					ttostr(aRun->start, buf);
					printf("%s,%s,%4ld,%.0lf,%.0lf,%2.0lf\n",
						aRec->vId,
						buf, 
						aRec->start - aRun->start,
						aRun->dist,
						aRec->dist,
						(aRec->dist-aRun->dist)*100/aRun->dist);
					onItem = bItem->next;
					break;
				} else {
					bItem = bItem->next;
				}
			} else 
				break; 
		}
		aItem = aItem->next;
	}
}



int main(int argc, char **argv)
{
	FILE *fruns, *frecs, *fdull = NULL;
	struct Duallist runs, recs;
	double cmpdist = 5000;

	if(argc < 3) {
	      printf("%s is used to compare distances derived and recorded.\n", argv[0]);
	      printf("Usage: %s [-l cmpdist] [-f dullmeter] file1.run file2.rec\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'l':
		      cmpdist = atof(argv[2]);
		      argc-=2;
		      argv+=2;
		      break;
	      case 'f':
		      fdull = fopen(argv[2], "a");
		      argc-=2;
		      argv+=2;
		      break;
	      }
		
	}
	
	if( (fruns=fopen(argv[1], "r")) == NULL || (frecs=fopen(argv[2],"r"))==NULL )
		return 0;

	duallist_init(&runs);
	duallist_init(&recs);

	load_runs(fruns, &runs);
	load_recs(frecs, &recs);

	compare_runs_with_recs(&runs, &recs, cmpdist, fdull);

	duallist_destroy(&runs, free);
	duallist_destroy(&recs, free);
	fclose(fruns);
	fclose(frecs);
	if(fdull) { fflush(fdull);fclose(fdull);}
	return 0;
}

