#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include"common.h"
#include"traffic.h"
#include"pkg.h"

#define GET_DELAY 0
#define GET_RATIO 1
#define GET_COST 2
#define GET_HOPS 3
#define PUT_ERRORBAR 0
#define PUT_PDF 1

int main(int argc, char**argv)
{
	int i;
	FILE *fp, *fdump;
	struct Item *aItem;
	struct Duallist pkgs;
	char *dumpfile=NULL;
	int purpose=GET_DELAY, output=PUT_ERRORBAR;
	double confidence;
	int *count;
	unsigned long traffic;
	double min, max, interval=1;	
	int first, index, numIntervals;
	struct Pkg *aPkg;
	

	if(argc < 2) {
		printf("Usage: %s [-p delay|ratio|cost|hops] [-o errorbar confidence | pdf intreval] [-w tofile] pkgdumpfile\n", argv[0]);
		exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'p':
			if(strcmp(argv[2],"delay")==0)
				purpose = GET_DELAY;
			if(strcmp(argv[2],"ratio")==0)
				purpose = GET_RATIO;
			if(strcmp(argv[2],"cost")==0)
				purpose = GET_COST;
			if(strcmp(argv[2],"hops")==0)
				purpose = GET_HOPS;
			argc-=2;
			argv+=2;
			break;
		case 'w':
			dumpfile = argv[2];
			argc-=2;
			argv+=2;
			break;
		case 'o':
			if(strcmp(argv[2],"errorbar")==0) {
				output = PUT_ERRORBAR;
				confidence = atof(argv[3]);
			}
			if(strcmp(argv[2],"pdf")==0) {
				output = PUT_PDF;
				interval = atof(argv[3]);
			}
			argc-=3;
			argv+=3;
			break;
		}
	}

	if( (fp=fopen(argv[1], "r"))== NULL) 
		return -1;

	duallist_init(&pkgs);
	load_traffic(fp, &pkgs, &traffic, LOAD_ALL_PKGS);

	if(purpose == GET_DELAY) {

	} else if(purpose == GET_RATIO) {


	} else if(purpose == GET_COST) {


	} else if(purpose == GET_HOPS) {
		aItem = pkgs.head;
		first = 1;
		while(aItem) {
			aPkg = (struct Pkg*)aItem->datap;
			duallist_remove_loops(&aPkg->routingRecord, (int(*)(void*,void*))are_strings_equal, free);
			if(first) {
				first = 0;
				min = aPkg->routingRecord.nItems;
				max = aPkg->routingRecord.nItems;
			} else {
				if(aPkg->routingRecord.nItems < min)
					min = aPkg->routingRecord.nItems;
				if(aPkg->routingRecord.nItems > max)
					max = aPkg->routingRecord.nItems;
			}
			aItem = aItem->next;
		}
		numIntervals = ceil((max-min)/interval);
		count = (int*)malloc(sizeof(int)*numIntervals);
		memset(count, 0, sizeof(int)*numIntervals);
		aItem = pkgs.head;
		while(aItem) {
			aPkg = (struct Pkg*)aItem->datap;
			index = (int)(aPkg->routingRecord.nItems-min)/interval;	
			if(index == numIntervals)
				index --;
			count[index] += 1;
			aItem = aItem->next;
		}
	}

	if((fdump=fopen(dumpfile, "a"))!=NULL) {
		for(i=0;i<ceil((max-min)/interval);i++) 
			fprintf(fdump, "%.0lf %d\n", min+i*interval, count[i]);
		fclose(fdump);
	}
	duallist_destroy(&pkgs, (void(*)(void*))pkg_free_func);
	fclose(fp);

	return 0;
}
