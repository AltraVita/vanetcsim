#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"common.h"
#include"files.h"
#include"contact.h"
#include"geometry.h"
 

struct Pair_Result
{
  char vName1[NAME_LENGTH];
  char vName2[NAME_LENGTH];

  time_t last1StartAt;
  time_t last1EndAt;

  unsigned long numContacts;
  unsigned long total;

  unsigned long *X_a;
  double *redundency;
  double *redundency_a;

  struct Duallist icts;
};

void pair_result_init_func(struct Pair_Result *aPairResult)
{
	if(aPairResult == NULL) 
		return;
	memset(aPairResult->vName1, 0, NAME_LENGTH);
	memset(aPairResult->vName2, 0, NAME_LENGTH);
	aPairResult->last1StartAt = 0;
	aPairResult->last1EndAt = 0;
	aPairResult->X_a = NULL;
	aPairResult->redundency = NULL;
	aPairResult->redundency_a = NULL;
	duallist_init(&aPairResult->icts);
}

int pair_result_has_names(char *names, struct Pair_Result *aPairResult)
{
	char buf[128];
	sprintf(buf, "%s,%s", aPairResult->vName1, aPairResult->vName2);
	return !strcmp(names, buf);
}

void pair_result_free_func(struct Pair_Result *aPairResult)
{
	if (aPairResult == NULL)
		return;
	if (aPairResult->X_a != NULL)
		free(aPairResult->X_a);
	if (aPairResult->redundency != NULL)
		free(aPairResult->redundency);
	if (aPairResult->redundency_a != NULL)
		free(aPairResult->redundency_a);
	duallist_destroy(&aPairResult->icts, free);
	free(aPairResult);
}



int main( int   argc,
          char *argv[] )
{
  char *ictevolving=NULL, *ictevolving_a=NULL;
  char *fromDay=NULL;
  char key[128];
  char buf[1024];
  char *strp, *strp1;
  FILE *fsource;
  
  struct Item *aContactItem;
  struct Contact *aContact, *bContact;

  struct Hashtable pairTable;
  struct Hashtable globalTable;

  struct Item *aPairItem;
  struct Pair *aPair;
  struct Item *aPairResultItem;
  struct Pair_Result *aPairResult;

  int magicNumber;
  int periods = 1;
  unsigned long *X;
  unsigned long *X_n;
  unsigned long *X_a;

  unsigned long pairTableSize = 10e6;
  int tGran = DEFAULT_MEETING_TEMPORAL_GRAN;
  int cGran = DEFAULT_MEETING_COUNTING_GRAN;
  unsigned long k;

  unsigned long T;
  int ii, jj;
  FILE *fdump, *fdump1;
  double entropy1, entropy2, jointEntropy;

  struct Item *aIctItem;
  struct ICT *aIct;

  time_t fromTime = 0;

  time_t previousTime;
  time_t interval = 0;
  int t1, i;

  struct Item *tempItem;


  if(argc < 3) {
	printf("Usage: %s [-t T meeting_temporal_gran(sec)] [-gc meeting_counting_gran(times)] [-o fromTime interval periods] [-w ict_nmi ict_nmia] pair_list .cont ...\n", argv[0]);
	exit(1);
  }
  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		T = atol(argv[2]);
		tGran = atoi(argv[3]);
		argc-=3;
		argv+=3;
		break;

	case 'g':
		if(argv[1][2] == 'c')
			cGran = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'o':
		fromDay = argv[2];
		interval = atoi(argv[3]);
		periods = atoi(argv[4]);
		argc-=4;
		argv+=4;
		break;

	case 'w':
		ictevolving = argv[2];
		ictevolving_a = argv[3];
		argc-=3;
		argv+=3;
		break;
		
	default:
		printf("Usage: %s [-t T meeting_temporal_gran(sec)] [-gc meeting_counting_gran(times)] [-o fromTime interval periods] [-w ict_nmi ict_nmia] pair_list .cont ...\n", argv[0]);
		exit(1);
	}
  }

	
  int numSlices = T/tGran;

  X = (unsigned long*)malloc(sizeof(unsigned long)*(numSlices)+1);
  X_n = (unsigned long*)malloc(sizeof(unsigned long)*(numSlices)+1);
  X_a = (unsigned long*)malloc(sizeof(unsigned long)*(numSlices)+1);

  fromTime = strtot(fromDay);

  hashtable_init(&globalTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_result_has_names);
  if((fsource=fopen(argv[1], "r"))!=NULL) {
	  while(fgets(buf, 1024, fsource)) {
		  strp = strtok(buf, ",");
		  strp1 = strtok(NULL, ",");
		  sprintf(key, "%s,%s", strp, strp1);
		  aPairResult = (struct Pair_Result*)malloc(sizeof(struct Pair_Result));
		  pair_result_init_func(aPairResult);
		  aPairResult->redundency=(double*)malloc(sizeof(double)*periods);
		  memset(aPairResult->redundency, 0,sizeof(double)* periods);

		  aPairResult->redundency_a=(double*)malloc(sizeof(double)*periods);
		  memset(aPairResult->redundency_a, 0, sizeof(double)*periods);

		  aPairResult->X_a=(unsigned long*)malloc(sizeof(unsigned long)*(numSlices)+1);
		  memset(aPairResult->X_a, 0, sizeof(unsigned long)*(numSlices)+1);

		  strncpy(aPairResult->vName1, strp, strlen(strp)+1);
		  strncpy(aPairResult->vName2, strp1, strlen(strp1)+1);
		  aPairResult->numContacts = 0;

		  hashtable_add(&globalTable, key, aPairResult);
	  }
	  fclose(fsource);
	  argv++;
	  argc--;
  }

  while(argc>1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fscanf(fsource, "%d\n", &magicNumber);
		printf("Loading %s file ...\n", argv[1]);
		if(magicNumber == FILE_CONTACT) {
			hashtable_init(&pairTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
			load_contacts_with_hashtable(fsource, NULL, &pairTable, PAIRWISE_TABLE, NULL, NULL);

			for (k=0;k<pairTable.size;k++) {
				aPairItem = pairTable.head[k];
				while (aPairItem != NULL) {
					aPair = (struct Pair*)aPairItem->datap;

					/* set up the coresponding pair in the global pair table */
					sprintf(key, "%s,%s", aPair->vName1, aPair->vName2);
					aPairResultItem = hashtable_find(&globalTable, key);
					if(aPairResultItem == NULL) {
						aPairItem = aPairItem->next;
						continue;
					} else {
						aPairResult = (struct Pair_Result*)aPairResultItem->datap;
						aPairResult->numContacts += aPair->contents.nItems;
					}

					aContactItem = aPair->contents.head;
					if(aPairResult->last1EndAt != 0) {
						aContact = (struct Contact*)aContactItem->datap;
						aIct = (struct ICT*)malloc(sizeof(struct ICT));
						aIct->timestamp = aContact->startAt;
						aIct->ict = aContact->startAt-aPairResult->last1EndAt;
						duallist_add_to_tail(&aPairResult->icts, aIct);
					}
					while(aContactItem!=NULL && aContactItem->next!=NULL) {
						aContact = (struct Contact*)aContactItem->datap;
						bContact = (struct Contact*)aContactItem->next->datap;
						aIct = (struct ICT*)malloc(sizeof(struct ICT));
						aIct->timestamp = bContact->startAt;
						aIct->ict = bContact->startAt-aContact->endAt;
						duallist_add_to_tail(&aPairResult->icts, aIct);
						aContactItem = aContactItem->next;
					}

					aContact = (struct Contact*)aPair->contents.head->prev->datap;
					aPairResult->last1StartAt = aContact->startAt;
					aPairResult->last1EndAt = aContact->endAt;


					aPairItem = aPairItem->next;
				}
			}

		} else {
			printf("Wrong file type! File %s has been ignored.\n", argv[1]);
		}
		fclose(fsource);
		hashtable_destroy(&pairTable, (void(*)(void*))pair_free_func);
	}
	argc--;
	argv++;
  }


  if(ictevolving != NULL) {


	for (k=0;k<globalTable.size;k++) {
		aPairResultItem = globalTable.head[k];
		while (aPairResultItem != NULL) {
			aPairResult = (struct Pair_Result*)aPairResultItem->datap;
			sprintf(key, "%s,%s", aPairResult->vName1, aPairResult->vName2);

			/* set up X */
			memset(X, 0, sizeof(unsigned long)*(numSlices)+1);
			if(aPairResult->icts.head !=NULL) {
				aIctItem = aPairResult->icts.head->prev;
				aIct = (struct ICT*)aIctItem->datap;
				while(aIctItem!=aPairResult->icts.head ) {
					if( aIct->timestamp >= fromTime && aIct->timestamp < fromTime+interval) {
						t1 = aIct->ict/tGran;
						if(t1<numSlices)
							X[t1] ++;
						else
							X[numSlices] ++;
					}
					if(aIct->timestamp < fromTime)
						break;
					aIctItem = aIctItem->prev;
					aIct = (struct ICT*)aIctItem->datap;
				}
			}
			entropy1 = vector_entropy(X, numSlices+1);
			if(entropy1 == 0) { 
				tempItem = aPairResultItem->next;
			 	pair_result_free_func(hashtable_pick(&globalTable, key));
				aPairResultItem = tempItem;
				continue;
			}
			previousTime = fromTime;
			for(i=0;i<periods;i++) {
				/* set up X-n */
				previousTime = previousTime - interval;
				memset(X_n, 0, sizeof(unsigned long)*(numSlices)+1);
				while(aIctItem != aPairResult->icts.head && aIct->timestamp >= previousTime ) {
					t1 = aIct->ict/tGran;
					if(t1<numSlices)
						X_n[t1] ++;
					else
						X_n[numSlices] ++;
					aIctItem = aIctItem->prev;
					aIct = (struct ICT*)aIctItem->datap;
				}

				/* set up sum X-1:X-n */
				for(jj=0;jj<(numSlices+1);jj++) {
					aPairResult->X_a[jj] += X_n[jj];
					X_a[jj] = aPairResult->X_a[jj];
				}
				for(jj=0;jj<(numSlices+1);jj++) {
					X[jj] = ceil(X[jj]*1.0/cGran);
					X_n[jj] = ceil(X_n[jj]*1.0/cGran);
					X_a[jj] = ceil(X_a[jj]*1.0/cGran);
				}
				entropy2 = vector_entropy(X_n, numSlices+1);
				jointEntropy = vectors_joint_entropy(X, X_n, numSlices+1);
				aPairResult->redundency[i] = (entropy1+entropy2-jointEntropy)/entropy1;

				entropy2 = vector_entropy(X_a, numSlices+1);
				jointEntropy = vectors_joint_entropy(X, X_a, numSlices+1);
				aPairResult->redundency_a[i] = (entropy1+entropy2-jointEntropy)/(entropy1+entropy2);
					
			}

			aPairResultItem = aPairResultItem->next;
		}
	}


	fdump = fopen(ictevolving, "w");
	fdump1 = fopen(ictevolving_a, "w");
	for (k=0;k<globalTable.size;k++) {
	      aPairResultItem = globalTable.head[k];
	      while (aPairResultItem != NULL) {
		      	aPairResult = (struct Pair_Result*)aPairResultItem->datap;
			for (ii=0;ii<periods;ii++) {
				fprintf(fdump, "%.4lf ", aPairResult->redundency[ii]);
				fprintf(fdump1, "%.4lf ", aPairResult->redundency_a[ii]);
			}
			fprintf(fdump, "\n");
			fprintf(fdump1, "\n");
		     	aPairResultItem = aPairResultItem->next;
	      }
	}
	fclose(fdump);
	fclose(fdump1);

  }

  hashtable_destroy(&globalTable, (void(*)(void*))pair_result_free_func);
  free(X);
  free(X_n);
  free(X_a);
  return 0;
}
