#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include"contact.h"
#include"common.h"
#include"files.h"

#define OUTPUT_PAIR_NAME 0
#define OUTPUT_TEMP_DIST 1
#define OUTPUT_SPAC_DIST 2
#define OUTPUT_CONTACTS 3

struct PairName
{
  char vName1[32];
  char vName2[32];
};
int pairName_has_names(char *names, struct PairName *aPairName)
{
	char buf[128];
	sprintf(buf, "%s,%s", aPairName->vName1, aPairName->vName2);
	return !strcmp(names, buf);
}

int main( int   argc,
          char *argv[] )
{
  char key[128], buf[128];
  FILE *fsource, *fdump;

  unsigned long t[24], tt;
  
  struct Hashtable pairNameTable, pairTable;

  struct Item *aPairNameItem, *aPairItem;
  struct PairName *aPairName;
  struct Pair *aPair;

  struct Item *aContactItem;
  struct Contact *aContact;

  int magicNumber;

  unsigned long pairTableSize = 10e6;
  int cGran = 1;
  unsigned long i, count;
  int k, j;
  time_t startAt = 0, endAt = 0, daytime;
  int output = OUTPUT_PAIR_NAME;
  char *pairlist = NULL, *strp, *strp1, *cntdumpfile=NULL;
  char interval = 1;

  if(argc < 2) {
	printf("Usage: %s [-c meeting_at_least(times)] [-l pair_list] [-o \"temp_dist interval(hours)\"|\"spac_dist\"|\"pair_name\"|\"contacts dump.cont\"] .cont ...\n", argv[0]);
	exit(1);
  }
  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 'c':
		cGran = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'l':
		pairlist = argv[2];
		argc-=2;
		argv+=2;
		break;

	case 'o':
		if(strcmp(argv[2],"pair_name")==0) {
			output = OUTPUT_PAIR_NAME;
			argc-=2;
			argv+=2;
		} else if(strcmp(argv[2],"temp_dist")==0) {
			output = OUTPUT_TEMP_DIST;
			interval = atoi(argv[3]);
			argc-=3;
			argv+=3;
		} else if(strcmp(argv[2],"spac_dist")==0) {
			output = OUTPUT_SPAC_DIST;
			argc-=2;
			argv+=2;
		} else if(strcmp(argv[2],"contacts")==0) {
			output = OUTPUT_CONTACTS;
			cntdumpfile = argv[3];
			argc-=3;
			argv+=3;
		}
		break;

	default:
		printf("Usage: %s [-c meeting_at_least(times)] [-l pair_list] [-o \"temp_dist interval(hours)\"|\"spac_dist\"|\"pair_name\"|\"contacts dump.cont\"] .cont ...\n", argv[0]);
		exit(1);
	}
  }
  if(cntdumpfile) {
	fdump = fopen(cntdumpfile, "w");
	fprintf(fdump, "%d\n", FILE_CONTACT);
  }

  hashtable_init(&pairNameTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pairName_has_names);
  if(pairlist && (fsource=fopen(pairlist, "r"))!=NULL) {
	while(fgets(buf, 1024, fsource)) {
		strp = strtok(buf, ",");
		strp1 = strtok(NULL, ",");
		sprintf(key, "%s,%s", strp, strp1);
		aPairNameItem = hashtable_find(&pairNameTable, key);
		if(aPairNameItem==NULL) {
			aPairName = (struct PairName*)malloc(sizeof(struct PairName));
			strncpy(aPairName->vName1, strp, strlen(strp)+1);
			strncpy(aPairName->vName2, strp1, strlen(strp1)+1);
			hashtable_add(&pairNameTable, key, aPairName);
		}
	}
  }

  hashtable_init(&pairTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  while(argc>1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fscanf(fsource, "%d\n", &magicNumber);
		if(magicNumber == FILE_CONTACT) {
			startAt = 0;
			endAt = 0;
			load_contacts_with_hashtable(fsource, NULL, &pairTable, PAIRWISE_TABLE, &startAt, &endAt);
		} else {
			printf("Wrong file type! File %s has been ignored.\n", argv[1]);
		}
		fclose(fsource);
	}
	argc--;
	argv++;
  }

  for (i=0;i<pairTable.size;i++) {
	  aPairItem = pairTable.head[i];
	  while (aPairItem != NULL) {
		  aPair = (struct Pair*)aPairItem->datap;
		  if(pairNameTable.count) {
			  sprintf(key, "%s,%s", aPair->vName1, aPair->vName2);
			  aPairNameItem = hashtable_find(&pairNameTable, key);
		  } else 
			  aPairNameItem = (void*)1;

		  if(aPairNameItem) {
			  if(aPair->contents.nItems >= cGran) {
				  for (k=0;k<24;k++)
					t[k] = 0;
				  tt = 0;

				  aContactItem = aPair->contents.head;
				  while(aContactItem) {
					aContact = (struct Contact*)aContactItem->datap;
					daytime = aContact->startAt%86400;
					t[daytime/3600] ++;
					tt ++;
					aContactItem = aContactItem->next;
				  }
				  if(output == OUTPUT_TEMP_DIST) {
					  for(j=0;j<24/interval;j++) {
						  count = 0;
						  for (k = 0;k<interval;k++) {
							count += t[j*interval + k];
						  }
						  printf("%.2lf ", count*1.0/tt);
					  }
					  printf("\n");
				  } else if (output == OUTPUT_PAIR_NAME) {
					  printf("%s,%s,%ld\n", aPair->vName1, aPair->vName2, aPair->contents.nItems);
				  } else if (output == OUTPUT_CONTACTS && fdump) {
						dump_contact_pair(aPair, fdump);
				  }

			  }
		  }
		  aPairItem = aPairItem->next;
	  }
  }
  if(fdump) fclose(fdump);
  hashtable_destroy(&pairNameTable, free);
  hashtable_destroy(&pairTable, (void(*)(void*))pair_free_func);
  return 0;
}
