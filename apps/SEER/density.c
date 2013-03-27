#include <stdlib.h>
#include <stdio.h>


int main(int argc, char**argv)
{
	FILE *fdump;
	int rId, rDi, nSlides, i;
	char *buffer, line[256], *p, *q;
	double samples;

	if(argc < 3) {
		printf("Usage: %s nslides dumpfile1\n", argv[0]);
		exit(0);
	}
	nSlides = atoi(argv[1]);
	buffer = (char*)malloc(32*(nSlides+1));
	if((fdump=fopen(argv[2], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[0]);
		exit(-1);
	}
	for (i=0;i<10;i++) {
		memset(line, 0, 256);
		fgets(line, 256, fdump);
		printf("%s", line);
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
		p = strtok(NULL, " ");
		p = strtok(NULL, " ");
		samples = atof(p);
		fgets(buffer, 32*(nSlides+1), fdump);
		printf("0 %d %d %lf\n", rId, rDi, samples);
		printf("1 %lf %lf\n", samples, samples);
	}
	free(buffer);
	fclose(fdump);
}
