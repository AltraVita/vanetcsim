#include<stdio.h>
#include<stdlib.h>

#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

int main(int argc, char**argv)
{
	FILE *fp, *fp2;
	double bytes = 0;
	double fromloc = 0;
	char *tofile = NULL;
	char *tailfile = NULL;
	int docount = 0;
	int c;

	if(argc < 2) {
		printf("Usage: %s [-c] [-s from_loc to_file] [-a tail_file] filename\n", argv[0]);
		exit(1);
	}
	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'c':
			docount = 1;
			argc-=1;
			argv+=1;
			break;
		case 's':
			fromloc = atof(argv[2]);
			tofile = argv[3];
			argc-=3;
			argv+=3;
			break;
		case 'a':
			tailfile = argv[2];
			argc-=2;
			argv+=2;
			break;
		}
	}
	if (docount) {	
		if((fp = fopen(argv[1], "r"))!=NULL) {
			while((c=fgetc(fp))!=EOF) 
				bytes += 1;	
			printf("There are %.0f bytes\n", bytes);
			fclose(fp);
		} else {
			perror(NULL);
		}
	} else if (tofile != NULL) {
		fp = fopen(argv[1], "r");
		fp2 = fopen(tofile, "w");
		bytes = 0;
		while(bytes<fromloc-1) {
			fgetc(fp);
			bytes += 1;
		}
		while((c=fgetc(fp))!=EOF)
			fputc(c, fp2);
		fclose(fp);
		fclose(fp2);
	} else if (tailfile!=NULL) {
		fp = fopen(argv[1], "r+");
		fp2 = fopen(tailfile, "r");
		fseeko(fp, -1L, SEEK_END);
		while((c=fgetc(fp2))!=EOF)
			fputc(c, fp);
		fclose(fp);
		fclose(fp2);
	}
	return 0;
}
