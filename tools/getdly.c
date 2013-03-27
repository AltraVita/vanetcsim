#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"

#define BUFF_SIZE 32768

/* input the pkgdumpfile of each method, 
 * the program will generate a compared
 * delay result file with each colume 
 * presenting the result of each method.
 */
int main(int argc, char**argv)
{
	int i,j,lines;
	FILE *fp, *fpp, *fdump;
	char *dchar = " \n";
	double  delay, *newDly;
	struct Item *aPkglistItem, *temp;
	struct Duallist pkgLists, *aPkglist;
	char buf[BUFF_SIZE], *strp, *dlydumpfile=NULL;

	if(argc < 2) {
		printf("Usage: %s [-d cutchar] [-w dlydump] pkgdumpfile1 pkgdumpfile2 ...\n", argv[0]);
		exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'd':
			dchar = argv[2];
			argc-=2;
			argv+=2;
			break;
		case 'w':
			dlydumpfile = argv[2];
			argc-=2;
			argv+=2;
			break;
		}
	}


	if( (fp=fopen(argv[1], "r"))== NULL) 
		return -1;
	fdump=fopen(dlydumpfile, "w");

	lines = 0;
	while(fgets(buf,BUFF_SIZE, fp)) {
		duallist_init(&pkgLists);
		strp = strtok(buf, dchar);
		newDly = (double*)malloc(sizeof(double));
		*newDly = atof(strp);
		aPkglist = (struct Duallist*)malloc(sizeof(struct Duallist));
		duallist_init(aPkglist);
		duallist_add_to_tail(&pkgLists, aPkglist);
		if(*newDly == -1) 
			free(newDly);
		else 
			duallist_add_to_tail(aPkglist, newDly);
		
		while((strp = strtok(NULL, dchar))!=NULL) {
			newDly = (double*)malloc(sizeof(double));
			*newDly = atof(strp);
			aPkglist = (struct Duallist*)malloc(sizeof(struct Duallist));
			duallist_init(aPkglist);
			duallist_add_to_tail(&pkgLists, aPkglist);
			if(*newDly == -1) 
				free(newDly);
			else 
				duallist_add_to_tail(aPkglist, newDly);
		}
		j = 2;
		while (j< argc) {
			fpp = fopen(argv[j],"r");
			if(fpp) {
				for(i=0;i<lines;i++)
					fgets(buf, 2048, fpp);
				if(fgets(buf, 2048, fpp)) {
					aPkglistItem = pkgLists.head;
					aPkglist = (struct Duallist*)aPkglistItem->datap;

					strp = strtok(buf, dchar);
					newDly = (double*)malloc(sizeof(double));
					*newDly = atof(strp);
					if(*newDly == -1) 
						free(newDly);
					else 
						duallist_add_to_tail(aPkglist, newDly);
					
					while((strp = strtok(NULL, dchar))!=NULL) {
						aPkglistItem = aPkglistItem->next;
						aPkglist = (struct Duallist*)aPkglistItem->datap;

						newDly = (double*)malloc(sizeof(double));
						*newDly = atof(strp);
						if(*newDly == -1) 
							free(newDly);
						else 
							duallist_add_to_tail(aPkglist, newDly);
					}
				}
				fclose(fpp);
			}
			j++;
		}

		aPkglistItem = pkgLists.head;
		while (aPkglistItem != NULL) {
			temp = aPkglistItem->next;
			aPkglist = (struct Duallist*)aPkglistItem->datap;
			if(aPkglist->nItems < argc-1) {
				duallist_destroy(duallist_pick_item(&pkgLists, aPkglistItem), free);
				aPkglistItem = temp;
				continue;
			}
			aPkglistItem = aPkglistItem->next;
		}

		if(fdump) {
			for(i=0;i<argc-1;i++) {
				delay = 0;
				aPkglistItem = pkgLists.head;
				while (aPkglistItem != NULL) {
					aPkglist = (struct Duallist*)aPkglistItem->datap;
					newDly = (double*)duallist_pick_head(aPkglist);
					delay = delay + *newDly;
					free(newDly);

					aPkglistItem = aPkglistItem->next;
				}
				fprintf(fdump, "%.2lf ", delay/pkgLists.nItems);
			}
			fprintf(fdump, "\n");
			duallist_destroy(&pkgLists, free);
		}
		lines++;
	}

	if(fp) fclose(fp);
	if(fdump) fclose(fdump);
	return 0;
}
