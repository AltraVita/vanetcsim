#include<stdio.h>
#include<stdlib.h>

int main(int argc, char**argv)
{
	FILE *fsource, *fdest;
	char fname[128], *strp;
	char buf[128];
	int modify = 0;

	if(argc < 3) {
		printf("Usage: %s [-m] magicNumber file1 file2 ...\n", argv[0]);
		exit(1);
	}

	if(argv[1][0] == '-' ) {
		if(argv[1][1] == 'm')
			modify = 1;
		argc --;
		argv ++;
	}

	strp = argv[1];
	argc --;
	argv ++;

	while(argc > 1) {
		if(modify) {
			fsource = fopen(argv[1], "r+");
			sprintf(buf, "%s\n", strp);
			fputs(buf, fsource);
			fclose(fsource);
		} else {
			sprintf(fname, "%s_", argv[1]);
			rename(argv[1], fname);
			fsource=fopen(fname, "r");
			fdest =fopen(argv[1], "w");
			sprintf(buf, "%s\n", strp);
			fputs(buf, fdest);
			while(fgets(buf, 128, fsource)) {
				fputs(buf, fdest);
			}
			fclose(fsource);
			fclose(fdest);
			remove(fname);
		}
		argc --;
		argv ++;
	}
			
}
