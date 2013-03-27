#include<stdio.h>
#include<stdlib.h>
#include<math.h>

int main(int argc, char**argv)
{
	FILE *froadinfo;
	int i, j, rId, rDi, atS2B, atB2S, at, nSlides, flag = 0;
	int whichroad, direction, interval;
	int speed;
	char *buffer, *p, *q;

	if(argc < 6) {
		printf("Usage: %s nSlides dumpfile rId rDi interval\n", argv[0]);
		exit(0);
	}
	nSlides=atoi(argv[1]);
	whichroad = atoi(argv[3]);
	direction = atoi(argv[4]);
	interval = atoi(argv[5]);

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
				if(rId==whichroad&&rDi==direction)
					flag = 1;
			} else {
				if(flag==1) {
					for(i=0;i<nSlides; i++) {
						p = strtok(NULL, " ");
						if(strcmp(p, "NaN")==0)
							printf("NaN\n");	
						else {
							speed = ceil(atof(p)/interval);			
							printf("%d\n", speed);
						}
					}
					break;
				}
			}
		}
		fclose(froadinfo);
	}
}
