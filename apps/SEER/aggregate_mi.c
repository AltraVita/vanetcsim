#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct SampleList
{	
	double value;
	struct SampleList *next;
};

struct RoadList
{
	int rId;
	int rDi;

	double *HUnion;
	double *HCond;
	double *redundancy;

	struct SampleList *samples;
	struct RoadList *next;
};

struct PUnion
{
	int *x;
	int nx;
	double value;
	struct PUnion *next;
};

int main(int argc, char**argv)
{
	FILE *fdump;
	char line[256], *q, *p, *buffer;
	struct SampleList *samples = NULL, *tmp, *newp, *sp;
	int i, j, nSlides, rId, rDi,interval, M;
	struct RoadList *roads = NULL, *newRoad, *tmproad;
	struct PUnion **cp;

	if(argc < 5) {
		printf("Usage: %s nSlides dumpfile interval M\n", argv[0]);
		exit(0);
	}
	if((fdump=fopen(argv[2], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[0]);
		exit(-1);
	}

	nSlides=atoi(argv[1]);
	interval = atoi(argv[3]);
	M = atoi(argv[4]);

	buffer = (char*)malloc(32*(nSlides+1));
	for (i=0;i<10;i++)
		fgets(line, 256, fdump);

	while (fgetc(fdump)!=EOF)
	{
		fseek(fdump, -1, SEEK_CUR);

		fgets(buffer, 32*(nSlides+1), fdump);
		p = strtok(buffer, " ");
		if(atoi(p)==0) {
			p = strtok(NULL, " ");
			rId = atoi(p);
			p = strtok(NULL, " ");
			rDi = atoi(p);
			
			newRoad = (struct RoadList*)malloc(sizeof(struct RoadList));
			newRoad->rId = rId;
			newRoad->rDi = rDi;
			newRoad->samples = NULL;
			newRoad->next = NULL;
			newRoad->HUnion = (double *)malloc(sizeof(double)*M);
			for(i=0;i<M;i++)
				newRoad->HUnion[i] = 0;
			newRoad->HCond = (double *)malloc(sizeof(double)*(M-1));
			for(i=0;i<M-1;i++)
				newRoad->HCond[i] = 0;
			newRoad->redundancy = (double *)malloc(sizeof(double)*(M-1));
			for(i=0;i<M-1;i++)
				newRoad->redundancy[i] = 0;
			
			if(roads == NULL) {
				roads = newRoad;
				tmproad = newRoad;
			} else {
				tmproad->next = newRoad;
				tmproad = newRoad;
			}	
		} else {
			for(i=0;i<nSlides; i++) {
				p = strtok(NULL, " ");
				newp = (struct SampleList*)malloc(sizeof(struct SampleList));
				if(strcmp(p, "NaN")==0)
					newp->value = -1;
				else {
					newp->value = floor(atof(p)/interval);
				}
				newp->next = NULL;
			
				if(newRoad->samples == NULL) {
					newRoad->samples = newp;
					tmp = newp;
				} else {
					tmp->next = newp;
					tmp = newp;
				}
			}

		}
	}
	fclose(fdump);

	unsigned nSamples;
	double vlg;
	int doit, match, positive;
	struct PUnion *newpx, *apx;

	cp=(struct PUnion**)malloc(sizeof(struct PUnion*)*M);
	for(i=0;i<M;i++)
		cp[i]=NULL;

	tmproad = roads;
	while(tmproad!=NULL) {
		for(i=0;i<M;i++) {
			nSamples = 0;
			tmp = tmproad->samples;
			while(tmp!=NULL) {
				sp=tmp;
				doit = 1;
				for(j=0;j<=i;j++) {
					if(sp!=NULL&&sp->value!=-1)
						sp = sp->next;
					else {
						doit = 0;
						break;
					}
				}
				
				if(doit) {
					apx = cp[i];
					while(apx!=NULL) {
						match=1;
						sp=tmp;
						for(j=0;j<=i;j++) {
							if(apx->x[j] ==	sp->value)
								sp = sp->next;
							else {
								match = 0;
								break;
							}
						}

						if(match) {
							apx->value += 1;
							nSamples ++;
							break;
						}
						apx = apx->next;
					}
					if(apx == NULL) {
						newpx=(struct PUnion*)malloc(sizeof(struct PUnion));
						newpx->x = (int *)malloc(sizeof(int)*(i+1));
						sp=tmp;
						for(j=0;j<=i;j++) {
							newpx->x[j] = sp->value;
							sp = sp->next;
						}
						newpx->value = 1;
						nSamples ++;
						newpx->next = NULL;
						if(cp[i] == NULL) 
							cp[i] = newpx;
						else {
							newpx->next = cp[i];
							cp[i] = newpx;
						}
					}
				}
				tmp = tmp->next;
			}
			apx = cp[i];
			while(apx!=NULL) {
				apx->value = apx->value/nSamples;
				vlg=log(apx->value)/log(2);
				tmproad->HUnion[i] = tmproad->HUnion[i] - apx->value*vlg;
				apx = apx->next;
			}
			if(i>0) {
				tmproad->HCond[i-1] = tmproad->HUnion[i]-tmproad->HUnion[i-1];
				tmproad->redundancy[i-1]= 1+(tmproad->HUnion[i-1]-tmproad->HUnion[i])/(tmproad->HUnion[0]);
			}
		}
		
		if(tmproad->HUnion[0] > 0) {
			positive = 1;
			for(i=0;i<M-1;i++) {
				if(tmproad->HCond[i] <= 0) {
					positive = 0;
					break;
				}
			}
			if(positive) {
				printf("%0.4lf ", tmproad->HUnion[0]);
				for(i=0;i<M-1;i++) {
					printf("%.4lf ", tmproad->HCond[i]);
//					printf("%.4lf ", tmproad->redundancy[i]);
				
				}
				printf("\n");
			}
		}
		for(i=0;i<M;i++) {
			apx = cp[i];
			while(apx!=NULL) {
				cp[i]=apx->next;
				free(apx->x);
				free(apx);
				apx = cp[i];
			}
			cp[i]=NULL;
		}
				
		tmproad = tmproad->next;
	}

	tmproad = roads;
	while(tmproad!=NULL) {
		tmp = tmproad->samples;
		while(tmp!=NULL) {
			tmproad->samples=tmp->next;
			free(tmp);
			tmp = tmproad->samples;
		}
		roads = tmproad->next;
		free(tmproad->HUnion);
		free(tmproad->HCond);
		free(tmproad);
		tmproad = roads;
	}
}
