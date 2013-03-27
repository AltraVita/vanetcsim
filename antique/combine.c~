#include<stdio.h>
#include<stdlib.h>

struct Slide
{
  double condition;
  double nSamples;
};

struct Scrap
{
  struct Slide *slidesS2B;
  struct Slide *slidesB2S;
  int atS2B;
  int atB2S;
  struct Scrap* next;
};

struct Road
{
  struct Scrap *scraps;
  int first;
  double nSamplesS2B;
  double nSamplesB2S;
};

struct RoadList
{
  struct Road *aRoad;
  int rId;
  int direction;
  double nSamples;
  struct RoadList *next;
};

struct RoadList * sort_road_by_trace(struct Road *roads)
{
	struct RoadList *rtList = NULL, *newp, *bRoad, *dRoad;
	int i;

	for(i=0;i<35000; i++) {
		if(roads[i].nSamplesS2B != 0) {
			newp = (struct RoadList*)malloc(sizeof(struct RoadList));
			if(newp == NULL) {
				printf("No memory available!\n");
				exit(-1);
			}
			newp->rId = i;
			newp->aRoad = roads+i;
			newp->direction = 0;
			newp->nSamples = roads[i].nSamplesS2B;
			newp->next = NULL;
			if (rtList == NULL) {
				rtList = newp;
			} else {
				bRoad = rtList;
				while(bRoad->next!=NULL && bRoad->nSamples >= newp->nSamples) {
					dRoad = bRoad;
					bRoad = bRoad->next;
				}
				if(bRoad->nSamples < newp->nSamples) {
					newp->next = bRoad;
					if(bRoad == rtList) {
						rtList = newp;
					} else {
						dRoad->next = newp;
					}
				} else {
					newp->next = bRoad->next;
					bRoad->next = newp;
				}
			}
		} 

		if(roads[i].nSamplesB2S != 0) {
			newp = (struct RoadList*)malloc(sizeof(struct RoadList));
			if(newp == NULL) {
				printf("No memory available!\n");
				exit(-1);
			}
			newp->rId = i;
			newp->aRoad = roads+i;
			newp->direction = 1;
			newp->nSamples = roads[i].nSamplesB2S;
			newp->next = NULL;
			if (rtList == NULL) {
				rtList = newp;
			} else {
				bRoad = rtList;
				while(bRoad->next!=NULL && bRoad->nSamples >= newp->nSamples) {
					dRoad = bRoad;
					bRoad = bRoad->next;
				}
				if(bRoad->nSamples < newp->nSamples) {
					newp->next = bRoad;
					if(bRoad == rtList) {
						rtList = newp;
					} else {
						dRoad->next = newp;
					}
				} else {
					newp->next = bRoad->next;
					bRoad->next = newp;
				}
			}
		} 
	}
	return rtList;
}


int main(int argc, char**argv)
{
	FILE *fdump;
	struct Road *roads, *whichRoad, *lastRoad;
	int i, j, rId, rDi, atS2B, atB2S, at, nSlides, first=1, files;
	char *buffer, *line, *p, *q;
	struct RoadList *roadlist=NULL, *bRoad;
	struct Scrap *newp, *whichScrap;
	int lower=-1, upper=-1;

	if(argc < 3) {
		printf("Usage: %s nSlides dumpfile1 dumpfile2 ...\n", argv[0]);
		exit(0);
	}
	nSlides=atoi(argv[1]);
	files = argc-2;
	buffer = (char*)malloc(32*(nSlides+1));
	line = (char*)malloc(256);
	roads = (struct Road*)malloc(sizeof(struct Road)*35000);
	for (i=0; i<35000; i++) {
		roads[i].scraps = NULL;
		roads[i].first = 1;
		roads[i].nSamplesS2B = 0;
		roads[i].nSamplesB2S = 0;
	}
	
	argc -=2;
	argv +=2;
	whichRoad = NULL;
	while(argc!=0) {
		if((fdump=fopen(argv[0], "r"))==NULL) {
			printf("Cannot open %s to read!\n", argv[0]);
			exit(-1);
		} else {
			for (i=0;i<10;i++) {
				memset(line, 0, 256);
				fgets(line, 256, fdump);
				if(first&&i<2) {
					printf(line);
					if (i==1)
						first = 0;
				}
				if(argc == 1&&i>1)
					printf(line);
			}
			while (fgetc(fdump)!=EOF)
			{
				fseek(fdump, -1, SEEK_CUR);

				fgets(buffer, 32*(nSlides+1), fdump);
				p = strtok(buffer, " ");
				if(atoi(p)==0) {
					lastRoad = whichRoad;
					if(lastRoad!=NULL)
						lastRoad->first = 0;
					p = strtok(NULL, " ");
					rId = atoi(p);
					p = strtok(NULL, " ");
					rDi = atoi(p);
					whichRoad = roads+rId;
					whichScrap = whichRoad->scraps;

				} else {
					if(whichRoad->first) {
						newp = (struct Scrap*)malloc(sizeof(struct Scrap));
						newp->slidesS2B = (struct Slide*)malloc(sizeof(struct Slide)*nSlides*files);
						newp->slidesB2S = (struct Slide*)malloc(sizeof(struct Slide)*nSlides*files);
						for (j=0;j<nSlides*files;j++) {
							newp->slidesS2B[j].condition = -1;
							newp->slidesS2B[j].nSamples = 0;
							newp->slidesB2S[j].condition = -1;
							newp->slidesB2S[j].nSamples = 0;
						}
						newp->atS2B = (files-argc)*nSlides;
						newp->atB2S = (files-argc)*nSlides;
						if (whichRoad->scraps == NULL) {
							whichRoad->scraps = newp;
						} else {
							whichScrap->next = newp;
						}
						whichScrap = newp;
					}

					while(1){
						p = strtok(NULL, " ");
						q = strtok(NULL, " ");
						if(p[0]=='\n') 
							break;
						if(rDi==0) {
							at=whichScrap->atS2B;
							whichScrap->slidesS2B[at].condition = atof(p);
							whichScrap->slidesS2B[at].nSamples = atof(q);
							whichScrap->atS2B ++;
							if(whichScrap->slidesS2B[at].condition != -1)
								roads[rId].nSamplesS2B += whichScrap->slidesS2B[at].nSamples;
						} else {
							at = whichScrap->atB2S;
							whichScrap->slidesB2S[at].condition = atof(p);
							whichScrap->slidesB2S[at].nSamples = atof(q);
							whichScrap->atB2S ++;
							if(whichScrap->slidesB2S[at].condition != -1)
								roads[rId].nSamplesB2S += whichScrap->slidesB2S[at].nSamples;
						}
					}
					if(!whichRoad->first)
						whichScrap = whichScrap->next;
				}
			}
		}
		argc -= 1;
		argv += 1;
	}
	roadlist = sort_road_by_trace(roads);
	bRoad = roadlist;
	while (bRoad != NULL) {
/*
		printf("0 %d %d -1 %.2lf\n", 	bRoad->rId,
						bRoad->direction,
						bRoad->nSamples);
*/		if(bRoad->direction ==0) {
			whichScrap = bRoad->aRoad->scraps;
			while(whichScrap!=NULL) {
//				printf("1  ");
				for (j=0;j<nSlides*files;j++) {
/*					if((whichScrap->slidesS2B)[j].condition==-1)
						printf("NaN ");
					else
						printf("%0.2lf ", (whichScrap->slidesS2B)[j].condition);
*/					printf("%.0lf ", (whichScrap->slidesS2B)[j].nSamples);
				}
				printf("\n");
				whichScrap = whichScrap->next;
			}
		}
		if(bRoad->direction ==1) {
			whichScrap = bRoad->aRoad->scraps;
			while(whichScrap!=NULL) {
//				printf("1  ");
				for (j=0;j<nSlides*files;j++) {
/*					if((whichScrap->slidesB2S)[j].condition==-1)
						printf("NaN ");
					else
						printf("%0.2lf ", (whichScrap->slidesB2S)[j].condition);
*/					printf("%.0lf ", (whichScrap->slidesB2S)[j].nSamples);
				}
				printf("\n");
				whichScrap = whichScrap->next;
			}
		}
		bRoad = bRoad->next;
	}
	for (i = 0; i< 35000; i++) {
		whichScrap = roads[i].scraps;
		while(whichScrap!=NULL) {
			free(whichScrap->slidesS2B);
			free(whichScrap->slidesB2S);
			whichScrap = whichScrap->next;
		}
	}

	free(roads);
	free(buffer);
	free(line);
	bRoad = roadlist;
	while(bRoad != NULL) {
		roadlist = bRoad->next;
		free(bRoad);
		bRoad = roadlist;
	}
	return 0;
}
