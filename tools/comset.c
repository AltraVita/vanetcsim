#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"

struct VidCount
{
  char id[256];
  int count;
};

int count_has_id(char *name, struct VidCount *aVid)
{
	return !strncmp(name, aVid->id, strlen(name));
}

char* basename (char* path)
{
    char *ptr = strrchr (path, '/');
    
    return ptr ? ptr + 1 : (char*)path;
}


int main(int argc, char**argv)
{
  FILE *fsource;
  char buf[128], *key;
  struct Hashtable vidTable;
  struct VidCount *aVid;
  struct Item *aItem; 
  char *strp = NULL;
  int total=0, i, fcomplete = 0, uncommon = 0;

  if(argc < 2) {
	printf("Usage: %s [-fullset] [-uncommon] [-p prefix] .lst ... \n", argv[0]);
	exit(1);
  }
	
  
  while(argv[1][0] == '-') {
	  switch(argv[1][1]) {
	  case 'f':
		  fcomplete = 1;
		  argc-=1;
		  argv+=1;
		  break;
	  case 'u':
		  uncommon = 1;
		  argc-=1;
		  argv+=1;
		  break;
	  case 'p':
		  strp = argv[2];
		  argc-=2;
		  argv+=2;
		  break;
	  }
  }

  hashtable_init(&vidTable, 3000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))count_has_id);
  while(argc > 1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fgets(buf, 127, fsource);
		while (fgets(buf, 127, fsource))
		{
			key = basename(buf);
			aItem = hashtable_find(&vidTable, key);
			if(aItem == NULL) {
				aVid = (struct VidCount*)malloc(sizeof(struct VidCount));
				strncpy(aVid->id, key, strlen(key)+1);
				aVid->count = 1;
				hashtable_add(&vidTable, key, aVid);
			} else {
				aVid = (struct VidCount*)aItem->datap;
				aVid->count ++;
			}
		}
	}
	fclose(fsource);
	total ++;
	argc --;
	argv ++;
  }

  for (i=0;i<vidTable.size;i++) {
	aItem = vidTable.head[i];
	while (aItem != NULL) {
		aVid = (struct VidCount*)aItem->datap;
		if(fcomplete == 0 && uncommon == 0 && aVid->count == total) {
			if(strp==NULL)
				printf("%s", aVid->id);
			else
				printf("%s%s", strp, aVid->id);
		} else if (fcomplete == 0 && uncommon == 1 && aVid->count != total) {
			if(strp==NULL)
				printf("%s", aVid->id);
			else
				printf("%s%s", strp, aVid->id);
		} else if(fcomplete) {
			if(strp==NULL)
				printf("%s", aVid->id);
			else
				printf("%s%s", strp, aVid->id);
		}
		aItem = aItem->next;
	}
  }
  
  hashtable_destroy(&vidTable, free);
  return 0;
}
