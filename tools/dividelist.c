#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"
#include"files.h"

int main(int argc, char**argv)
{
  FILE *fsource, *fdump;
  char buf[1024], *strp;
  struct Duallist aDuallist;
  struct Item *aItem; 
  int seed = 0, n1=0, n2=0;
  unsigned long index;

  if(argc < 4) {
	printf("Usage: %s [-random seed] .lst n1 n2\n", argv[0]);
	exit(1);
  }
	
  
  while(argv[1][0] == '-') {
	  switch(argv[1][1]) {
	  case 'r':
		  seed = atoi(argv[2]);
		  argc-=2;
		  argv+=2;
		  break;
	  }
  }

  if(seed)
	srand(seed);
  else
	srand(time(0));
  
  if((fsource=fopen(argv[1], "r"))!=NULL) {
	  duallist_init(&aDuallist);
	  fgets(buf, 1024, fsource);
	  while (fgets(buf, 1024, fsource))
	  {
		strp = (char*)malloc(sizeof(char)*1024);
		strncpy(strp, buf, strlen(buf)+1);
		duallist_add_to_tail(&aDuallist, strp);
	  }
  }
  fclose(fsource);

  n1 = atoi(argv[2]);
  n2 = atoi(argv[3]);
 

  if(n1) {
	  fdump=fopen("first.lst", "w");  
	  fprintf(fdump,"%d\n", FILE_LIST);
	  while(n1 && aDuallist.nItems) {
	      index = rand()%aDuallist.nItems;
	      aItem = aDuallist.head;
	      while(index) {
		  aItem = aItem->next;
		  index--;
	      }
	      strp = (char*)duallist_pick_item(&aDuallist, aItem);
	      fprintf(fdump, "%s", strp);
	      free(strp);
	      n1 --;
	  }
	  fclose(fdump);
  }

  if(n2) {
	  fdump=fopen("second.lst", "w");  
	  fprintf(fdump,"%d\n", FILE_LIST);
	  while(n2 && aDuallist.nItems) {
	      index = rand()%aDuallist.nItems;
	      aItem = aDuallist.head;
	      while(index) {
		  aItem = aItem->next;
		  index--;
	      }
	      strp = (char*)duallist_pick_item(&aDuallist, aItem);
	      fprintf(fdump, "%s", strp);
	      free(strp);
	      n2 --;
	  }
	  fclose(fdump);
  }

  duallist_destroy(&aDuallist, free);

  return 0;
}
