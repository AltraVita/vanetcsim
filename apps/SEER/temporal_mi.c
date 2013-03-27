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
	int i, j, nSlides, rId, rDi,interval, M, urId, urDi;
	struct RoadList *roads = NULL, *newRoad, *tmproad;
	struct PUnion **cp;

	if(argc < 5) {
		printf("Usage: %s nSlides dumpfile interval M \n", argv[0]);
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
	double vlg, hxy, hx, *redundancy;
	int doit, match, positive, toprint;
	struct PUnion *newpx, *apx, *pxs, *pxys, *pys;

	redundancy = (double *)malloc(sizeof(double)*(M-1));

	tmproad = roads;
	while(tmproad!=NULL) {
		nSamples = 0;
		pxs = NULL;
		tmp=tmproad->samples;
		for(i=0;i<nSlides;i++) {
			if(tmp->value != -1) {
				apx = pxs;
				while(apx!=NULL) {
					if(apx->x[0]==tmp->value) {
						apx->value += 1;
						nSamples ++;
						break;
					}
					apx = apx->next;
				}
				if(apx == NULL) {
					newpx=(struct PUnion*)malloc(sizeof(struct PUnion));
					newpx->x = (int *)malloc(sizeof(int));
					newpx->x[0] = tmp->value;
					newpx->value = 1;
					nSamples ++;
					newpx->next = NULL;
					if(pxs == NULL) 
						pxs = newpx;
					else {
						newpx->next = pxs;
						pxs = newpx;
					}
				}
			}
			tmp = tmp->next;
		}
		hx = 0;
		if(nSamples >100) {
			apx = pxs;
			while(apx!=NULL) {
				apx->value = apx->value/nSamples;
				vlg=log(apx->value)/log(2);
				hx = hx - apx->value*vlg;
				apx = apx->next;
			}
		}
		apx = pxs;
		while(apx!=NULL) {
			pxs=apx->next;
			free(apx->x);
			free(apx);
			apx = pxs;
		}
		for(i=0;i<M-1;i++)
			redundancy[i] = 0;

		for(i=1;i<M;i++) {
			nSamples = 0;
			pxys = NULL;
			tmp = tmproad->samples;
			while(tmp!=NULL) {
				sp=tmp;
				j = 0;
				while(j<i) {
					if(sp!=NULL) {
						sp = sp->next;
						j ++;
					} else 
						break;
				}
				if(tmp->value!=-1 && (j==i && sp!=NULL && sp->value!=-1)) {
					apx = pxys;
					while(apx!=NULL) {
						if(tmp->value == apx->x[0] && sp->value == apx->x[1]) {
							apx->value += 1;
							nSamples ++;
							break;
						}
						apx = apx->next;
					}
					if(apx == NULL) {
						newpx=(struct PUnion*)malloc(sizeof(struct PUnion));
						newpx->x = (int *)malloc(sizeof(int)*2);
						newpx->x[0]=tmp->value;
						newpx->x[1]=sp->value;
						newpx->value = 1;
						nSamples ++;
						newpx->next = NULL;
						if(pxys == NULL) 
							pxys = newpx;
						else {
							newpx->next = pxys;
							pxys = newpx;
						}
					}
				}
				tmp = tmp->next;
			}
			hxy = 0;
			if(nSamples >100) {
				apx = pxys;
				while(apx!=NULL) {
					apx->value = apx->value/nSamples;
					vlg=log(apx->value)/log(2);
					hxy = hxy - apx->value*vlg;
					apx = apx->next;
				}
			}
			apx = pxys;
			while(apx!=NULL) {
				pxys=apx->next;
				free(apx->x);
				free(apx);
				apx = pxys;
			}
			if(hxy>0&&hx>0) {
				redundancy[i-1] = hxy-hx;
			}
		}
		toprint = 1;
		for(i=0;i<M-1;i++) {
			if(redundancy[i] == 0) {
				toprint = 0;
				break;
			}
		}
		if(toprint) {
			for(i=0;i<M-1;i++) 
				printf("%.4lf ", redundancy[i]);
			printf("\n");
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
		free(tmproad);
		tmproad = roads;
	}
}
