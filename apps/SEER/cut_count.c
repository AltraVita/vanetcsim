#include<stdio.h>
#include<stdlib.h>
int main(int argc, char**argv)
{
	FILE *froadinfo;
	int i, j, rId, rDi, atS2B, atB2S, at, nSlides, flag;
	char *buffer, *p, *q;
	double count, count1, cond, maxcond, mincond;

	if(argc < 4) {
		printf("Usage: %s nSlides dumpfile flag\n", argv[0]);
		exit(0);
	}
	nSlides=atoi(argv[1]);
	flag=atoi(argv[3]);
	buffer = (char*)malloc(32*(nSlides+1));

	char ch;


	if((froadinfo=fopen(argv[2], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[3]);
	} else {
		for (i=0;i<10;i++)
			fgets(buffer, 256, froadinfo);

		while (fgetc(froadinfo)!=EOF)
		{
			fseek(froadinfo, -1, SEEK_CUR);

			fgets(buffer, 32*(nSlides+1), froadinfo);
			p = strtok(buffer, " ");
			if(atoi(p)==0) {
				p = strtok(NULL, " ");
				rId = atoi(p);
				p = strtok(NULL, " ");
				rDi = atoi(p);
				printf("\n%d %d ", rId, rDi);
			} else {
				count = 0;
				count1 = 0;
				cond = 0;
				maxcond = 0;
				mincond = 1000;
				for(i=0;i<nSlides; i++) {
					p = strtok(NULL, " ");
					q = strtok(NULL, " ");
					count += atof(q);
					if(atof(p)>=0) {
						count1 ++;
						cond += atof(p);
						if(atof(p)>maxcond) 
							maxcond = atof(p);
						if(atof(p)<mincond)
							mincond = atof(p);
					}
				}
				if(flag ==0) 
					printf(" %.0lf", count);
				else if (count1 >0)			
					printf(" %0.2lf %0.2lf %0.2lf", mincond, cond/count1, maxcond);
				else
					printf(" -1 -1 -1");
			}
		}
		fclose(froadinfo);
	}
}
