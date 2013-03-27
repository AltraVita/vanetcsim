#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"files.h"

void rm_defact_func(FILE *fsource, int magicNumber, char *fname)
{
	char buf[256];
	FILE *fp;
	
	fp = fopen("temp.mgd", "w");
	fprintf(fp, "%d\n", magicNumber);
	while(fgets(buf, 256, fsource)) {
		if(strstr(buf, "-1") == NULL) 
			fputs(buf, fp);
	}
	fclose(fsource);
	fclose(fp);
	remove(fname);
	rename("temp.mgd", fname);	
}


void process_source_file(FILE *fsource, char *fname)
{
	char buf[128], filename[128];
	FILE *fp;
	int magicNumber;

	fscanf(fsource, "%d\n", &magicNumber);
	if(magicNumber == FILE_LIST) {
		memset(buf, '\0', 128);
		while (fgets(buf, 127, fsource)) {
			sscanf(buf, "%s", filename);
			if( (fp=fopen(filename, "r"))!=NULL) {
				process_source_file(fp, filename);
			}
			memset(buf, '\0', 128);
		}
	} else if (magicNumber == FILE_MODIFIED_GPS_TAXI
		|| magicNumber == FILE_MODIFIED_GPS_BUS) {
		rm_defact_func(fsource, magicNumber, fname);
	}
}

int main(int argc, char**argv)
{
	FILE *fsource;

	if(argc < 2) {
	      printf("%s is used to remove defact reports in mgd files\n", argv[0]);
	      printf("Usage: %s .mgd|.lst ...\n", argv[0]);
	      exit(1);
	}

	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			printf("Loading %s file ...\n", argv[1]);
			process_source_file(fsource, argv[1]);
		}
		argc--;
		argv++;
	}
	return 0;
}

