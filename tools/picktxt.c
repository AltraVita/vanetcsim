#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

int main(int argc, char**argv)
{
	FILE *fp;
	char *dchar = " ";
	int i, which = 0, from = 0, lines = 20;
	char buf[256], *strp;

	if(argc < 2) {
		printf("Usage: %s [-d cutchar] [-f column] [-h from] [-l lines] filename\n", argv[0]);
		exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'd':
			dchar = argv[2];
			argc-=2;
			argv+=2;
			break;
		case 'f':
			which = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		case 'h':
			from = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		case 'l':
			lines = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		}
	}

	if( (fp=fopen(argv[1], "r"))== NULL) 
		return -1;

	for(i=0;i<from;i++)
		fgets(buf, 255, fp);

	for(i=0;i<lines;i++) {
		fgets(buf, 255, fp);
		if(which == 0) {
			printf("%s", buf);
		} else {
			strp = strtok(buf, dchar);
			if(which > 1) {
				for(i = 0; i < which-1; i++) 
					strp = strtok(NULL, dchar);
			}
			printf("%s",strp);
		}
	}

	return 0;
}
