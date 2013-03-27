#include<stdio.h>
#include<stdlib.h>

struct Slide
{
  double condition;
  double nSamples;
};

struct Road
{
  struct Slide *slidesS2B;
  struct Slide *slidesB2S;
  double nSamplesS2B;
  double nSamplesB2S;
  int atS2B;
  int atB2S;
};


int main(int argc, char**argv)
{
	FILE *fdump;
	struct Road *roads;
	int i, j, rId, rDi, atS2B, atB2S, at, nSlides,flag, thresold;
	unsigned count, total;
	char *buffer, *line, *p, *q;
	struct RoadList *roadlist=NULL, *bRoad;

	if(argc < 5) {
		printf("Usage: %s nSlides dumpfile1 flag thresold\n", argv[0]);
		exit(0);
	}
	nSlides=atoi(argv[1]);
	flag = atoi(argv[3]);
	thresold = atoi(argv[4]);

	buffer = (char*)malloc(32*(nSlides+1));
	line = (char*)malloc(256);
	roads = (struct Road*)malloc(sizeof(struct Road)*35000);
	for (i=0; i<35000; i++) {
		roads[i].slidesS2B = (struct Slide*)malloc(sizeof(struct Slide)*nSlides);
		roads[i].slidesB2S = (struct Slide*)malloc(sizeof(struct Slide)*nSlides);
		for (j=0;j<nSlides;j++) {
			roads[i].slidesS2B[j].condition = -1;
			roads[i].slidesS2B[j].nSamples = 0;
			roads[i].slidesB2S[j].condition = -1;
			roads[i].slidesB2S[j].nSamples = 0;
		}
		roads[i].nSamplesS2B = 0;
		roads[i].nSamplesB2S = 0;
		roads[i].atS2B = 0;
		roads[i].atB2S = 0;
	}
	
	if((fdump=fopen(argv[2], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[0]);
		exit(-1);
	} else {
		for (i=0;i<10;i++) {
			memset(line, 0, 256);
			fgets(line, 256, fdump);
			if(flag == 1)
				printf(line);
		}
		while (fgetc(fdump)!=EOF)
		{
			fseek(fdump, -1, SEEK_CUR);
			fgets(line, 256,fdump);
			p = strtok(line, " ");
			p = strtok(NULL, " ");
			rId = atoi(p);
			p = strtok(NULL, " ");
			rDi = atoi(p);

			fgets(buffer, 32*(nSlides+1), fdump);
			p = strtok(buffer, " ");
			while(1) {
				p = strtok(NULL, " ");
				q = strtok(NULL, " ");
				if(p[0]=='\n') 
					break;
				if(rDi==0) {
					at=roads[rId].atS2B;
					roads[rId].slidesS2B[at].condition = atof(p);
					roads[rId].slidesS2B[at].nSamples = atof(q);
					
					roads[rId].atS2B ++;
					if(atof(q)>0)
						roads[rId].nSamplesS2B ++;
				} else {
					at = roads[rId].atB2S;
					roads[rId].slidesB2S[at].condition = atof(p);
					roads[rId].slidesB2S[at].nSamples = atof(q);
					roads[rId].atB2S ++;
					if(atof(q)>0)
						roads[rId].nSamplesB2S ++;
				}
			}
		}
	}

	count = 0;
	total = 0;
	for(i =35000-1;i>=0; i--) {
		if(flag == 0) {
			if(roads[i].nSamplesS2B != 0) {
				printf("%d 0", i);
				for (j=0;j<nSlides;j++) {
					printf(" %.0lf", roads[i].slidesS2B[j].nSamples);
				}
				printf("\n");
			}
			if(roads[i].nSamplesB2S != 0) {
				printf("%d 1", i);
				for (j=0;j<nSlides;j++) {
					printf(" %.0lf", roads[i].slidesB2S[j].nSamples);
				}
				printf("\n");
			}
		}
		if(flag ==1) {
			if(roads[i].nSamplesS2B != 0) {
				if(roads[i].nSamplesS2B > thresold)
					count ++;
				total ++;
		//		printf("0 %d 0\n", i);
		//		printf("1 %0.2lf %lf\n", 1-roads[i].nSamplesS2B/nSlides, roads[i].nSamplesS2B);
				printf("%0.5lf ", 1-roads[i].nSamplesS2B/nSlides);
			}
			if(roads[i].nSamplesB2S != 0) {
				if(roads[i].nSamplesB2S > thresold)
					count ++;
//				printf("0 %d 1\n", i);
				total ++;
//				printf("1 %0.2lf %lf\n", 1-roads[i].nSamplesB2S/nSlides, roads[i].nSamplesB2S);
				printf("%0.5lf ", 1-roads[i].nSamplesB2S/nSlides);
			}
		}
	}

	if(total!= 0)
//		printf("%0.2lf\n", count*1.0/total);

	for (i = 0; i< 35000; i++) {
		free(roads[i].slidesS2B);
		free(roads[i].slidesB2S);
	}

	free(buffer);
	free(line);
	return 0;
}
