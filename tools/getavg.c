#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"common.h"

#define BUFF_SIZE 32768

struct Data
{
  double total;
  double min;
  double max;
};

void mount_a_file(FILE *fp, struct Duallist *dataCurtain, char *dchars, int first);

int main(int argc, char**argv)
{
	FILE *fp;
	int which = 0, i;
	char *dchars = " \n";
	int files, first = 1, direction = 0;
	struct Data *aValue, *bValue;
	struct Duallist dataCurtain, *aLineDuallist, *bLineDuallist;
	struct Item *aValueItem, *bValueItem, *aLineDuallistItem, *bLineDuallistItem;

	if(argc < 2) {
		printf("Usage: %s [-d cutchar] [-f column] [-vertical] file1 file2 ...\n", argv[0]);
		printf("Note: if only one file is provided, then average all lines vertically.\n");
		printf("If multiple files are given, the averages are taken on all files based on the same layout of these files\n");
		exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'd':
			dchars = argv[2];
			argc-=2;
			argv+=2;
			break;
		case 'v':
			direction = 1;
			argc-=1;
			argv+=1;
			break;
		case 'f':
			which = atoi(argv[2]);
			argc-=2;
			argv+=2;
			break;
		}
	}

	first = 1;
	files = 0;
	duallist_init(&dataCurtain);
	while(argc>1) {
		if( (fp=fopen(argv[1], "r"))!= NULL) {
			files ++;
			if(first) {
				mount_a_file(fp, &dataCurtain, dchars, first);
				first = 0;
			} else
				mount_a_file(fp, &dataCurtain, dchars, first);
			fclose(fp);
		}
		argc--;
		argv++;
	}

	if( files == 1) {
		if(direction) {
			/* if there is only one file provided, then vertically average the lines in the file*/
			aLineDuallistItem = dataCurtain.head;
			aLineDuallist = (struct Duallist*)aLineDuallistItem->datap;
			bLineDuallistItem = aLineDuallistItem->next;
			while(bLineDuallistItem) {
				bLineDuallist = (struct Duallist*)bLineDuallistItem->datap;
				aValueItem = aLineDuallist->head;	
				bValueItem = bLineDuallist->head;	
				for(i=0;i<aLineDuallist->nItems;i++) {
					aValue = (struct Data*)aValueItem->datap;
					bValue = (struct Data*)bValueItem->datap;
					aValue->total += bValue->total;
					if(aValue->min > bValue->min)
						aValue->min = bValue->min;
					if(aValue->max < bValue->max)
						aValue->max = bValue->max;
					aValueItem = aValueItem->next;
					bValueItem = bValueItem->next;
				}
				bLineDuallistItem = bLineDuallistItem->next;
			}
			aValueItem = aLineDuallist->head;
			if(which == 0) {
				while (aValueItem != NULL) {
					aValue = (struct Data*)aValueItem->datap;
					printf("%.2lf %.2lf %.2lf  ", aValue->min, aValue->total/dataCurtain.nItems, aValue->max);
					aValueItem = aValueItem->next;
				}
				printf("\n");
			} else {
				for (i=0;i<which-1;i++) {
					aValueItem = aValueItem->next;
				}
				if(aValueItem != NULL) {
					aValue = (struct Data*)aValueItem->datap;
					printf("%.2lf %.2lf %.2lf\n", aValue->min, aValue->total/dataCurtain.nItems, aValue->max);
				}
			}
		} else {
			aLineDuallistItem = dataCurtain.head;
			while(aLineDuallistItem) {
				aLineDuallist = (struct Duallist*)aLineDuallistItem->datap;
				aValueItem = aLineDuallist->head;	
				aValue = (struct Data*)aValueItem->datap;
				bValueItem = aValueItem->next;
				for(i=1;i<aLineDuallist->nItems;i++) {
					bValue = (struct Data*)bValueItem->datap;
					aValue->total += bValue->total;
					if(aValue->min > bValue->min)
						aValue->min = bValue->min;
					if(aValue->max < bValue->max)
						aValue->max = bValue->max;
					bValueItem = bValueItem->next;
				}
				aLineDuallistItem = aLineDuallistItem->next;
			}
			i = 0;
			aLineDuallistItem = dataCurtain.head;
			while(aLineDuallistItem) {
				i++;
				aLineDuallist = (struct Duallist*)aLineDuallistItem->datap;
				aValueItem = aLineDuallist->head;
				if(which==0 || which==i) {
					aValue = (struct Data*)aValueItem->datap;
					printf("%.2lf %.2lf %.2lf \n", aValue->min, aValue->total/aLineDuallist->nItems, aValue->max);
				}
				aLineDuallistItem = aLineDuallistItem->next;
			}
		}
	} else {
		/* if multiple files are provided, then output the averages among all files based on the layout of files */
		aLineDuallistItem = dataCurtain.head;
		while(aLineDuallistItem) {
			aLineDuallist = (struct Duallist*)aLineDuallistItem->datap;
			aValueItem = aLineDuallist->head;
			if(which == 0) {
				while (aValueItem != NULL) {
					aValue = (struct Data*)aValueItem->datap;
					printf("%.2lf %.2lf %.2lf  ", aValue->min, aValue->total/files, aValue->max);
					aValueItem = aValueItem->next;
				}
				printf("\n");
			} else {
				for (i=0;i<which-1;i++) {
					aValueItem = aValueItem->next;
				}
				if(aValueItem != NULL) {
					aValue = (struct Data*)aValueItem->datap;
					printf("%.2lf %.2lf %.2lf\n", aValue->min, aValue->total/files, aValue->max);
				}
			}
			aLineDuallistItem = aLineDuallistItem->next;
		}
	}
	curtain_destroy(&dataCurtain, free);
	return 0;
}

void mount_a_file(FILE *fp, struct Duallist *dataCurtain, char *dchars, int first)
{
	double newbee;
	struct Data *aValue;
	struct Item *aValueItem;
	struct Item *aLineDuallistItem;
	struct Duallist *aLineDuallist;
	char buf[BUFF_SIZE], *strp;

	if(first) {
		memset(buf, 0, BUFF_SIZE);
		while(fgets(buf, BUFF_SIZE, fp)) {
			aLineDuallist = (struct Duallist*)malloc(sizeof(struct Duallist));
			duallist_add_to_tail(dataCurtain, aLineDuallist);
			duallist_init(aLineDuallist);
			strp = strtok(buf, dchars);
			aValue = (struct Data*)malloc(sizeof(struct Data));
			aValue->total = atof(strp);
			aValue->min = aValue->total;
			aValue->max = aValue->total;
			duallist_add_to_tail(aLineDuallist, aValue);
			
			while((strp = strtok(NULL, dchars))!=NULL) {
				aValue = (struct Data*)malloc(sizeof(struct Data));
				aValue->total = atof(strp);
				aValue->min = aValue->total;
				aValue->max = aValue->total;
				duallist_add_to_tail(aLineDuallist, aValue);
			}
			memset(buf, 0, BUFF_SIZE);
		}
	} else {
		aLineDuallistItem = dataCurtain->head;
		memset(buf, 0, BUFF_SIZE);
		while(fgets(buf, BUFF_SIZE, fp)) {
			aLineDuallist = (struct Duallist*)aLineDuallistItem->datap;
			aValueItem = aLineDuallist->head;
			aValue = (struct Data*)aValueItem->datap;

			strp = strtok(buf, dchars);
			newbee = atof(strp);
			aValue->total += newbee;
			if(aValue->min > newbee)
				aValue->min = newbee;
			if(aValue->max < newbee)
				aValue->max = newbee;
			while((strp = strtok(NULL, dchars))!=NULL) {
				aValueItem = aValueItem->next;
				aValue = (struct Data*)aValueItem->datap;
				newbee = atof(strp);
				aValue->total += newbee;
				if(aValue->min > newbee)
					aValue->min = newbee;
				if(aValue->max < newbee)
					aValue->max = newbee;
			}
			aLineDuallistItem = aLineDuallistItem->next;
			memset(buf, 0, BUFF_SIZE);
		}
	}
}
