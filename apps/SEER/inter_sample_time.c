#include <stdlib.h>
#include <stdio.h>

int main(int argc, char**argv)
{
	FILE *fdump;
	char line[256], *p, *buffer;
	unsigned *count, nSlides, acount, first, i;

	if(argc < 3) {
		printf("Usage: %s nSlides dumpfile \n", argv[0]);
		exit(0);
	}
	if((fdump=fopen(argv[2], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[0]);
		exit(-1);
	}

	nSlides=atoi(argv[1]);
	count=(unsigned*)malloc(sizeof(unsigned)*nSlides);
	for (i = 0; i<nSlides; i++) {
		count[i] = 0;
	}
	buffer = (char*)malloc(32*(nSlides+1));
	while (fgetc(fdump)!=EOF)
	{
		fseek(fdump, -1, SEEK_CUR);

		fgets(buffer, 32*(nSlides+1), fdump);
		acount = 0;
		first = 1;
		for(i=0;i<nSlides; i++) {
			if(i==0) {
				p = strtok(buffer, " ");
			} else {
				p = strtok(NULL, " ");
			}
			if(strcmp(p, "NaN")==0)
				if(first) {
				} else {
					acount ++;	
				}
			else {
				if(first) {
					first = 0;
					acount = 0;
				} else {
					acount ++;
					count[acount] ++;
					acount = 0;
				}
			}
		}
	}
	for (i = 0; i<nSlides; i++) {
		printf("%ld ", count[i]);
	}
	free(count);
	free(buffer);
	fclose(fdump);
}

