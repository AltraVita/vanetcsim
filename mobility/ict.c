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

  struct Hashtable *singleTable;
  struct Hashtable *coupleTable;
  struct Hashtable *tripleTable;
 
  time_t last1StartAt;
  time_t last1EndAt;
  time_t last2StartAt;
  time_t last2EndAt;

  int filtered;
  unsigned long numContacts;
  unsigned long total;

  double entropy;
  double entropy1;
  double entropy2;

};

void pair_result_init_func(struct Pair_Result *aPairResult)
{
	if(aPairResult == NULL) 
		return;
	memset(aPairResult->vName1, 0, NAME_LENGTH);
	memset(aPairResult->vName2, 0, NAME_LENGTH);
	aPairResult->singleTable = NULL;
	aPairResult->coupleTable = NULL;
	aPairResult->tripleTable = NULL;
	aPairResult->last1StartAt = 0;
	aPairResult->last1EndAt = 0;
	aPairResult->last2StartAt = 0;
	aPairResult->last2EndAt = 0;
	aPairResult->entropy = 0;
	aPairResult->entropy1 = 0;
	aPairResult->entropy2 = 0;
	aPairResult->filtered = 0;
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
	if (aPairResult->singleTable != NULL)
		hashtable_destroy(aPairResult->singleTable, free);
	if (aPairResult->coupleTable != NULL)
		hashtable_destroy(aPairResult->coupleTable, free);
	if (aPairResult->tripleTable != NULL)
		hashtable_destroy(aPairResult->tripleTable, free);
	free(aPairResult->singleTable);
	free(aPairResult->coupleTable);
	free(aPairResult->tripleTable);
	aPairResult->singleTable = NULL;
	aPairResult->coupleTable = NULL;
	aPairResult->tripleTable = NULL;
	free(aPairResult);
}


int main( int   argc,
          char *argv[] )
{
  char *ictdumpfile = NULL, *cnttemperal=NULL, *cntentropy=NULL, *ictentropy=NULL;
  char *pair_list=NULL;
  char key[128];
  char buf[1024];
  char *strp, *strp1;
  FILE *fsource, *fdump;
  
  struct Item *aContactItem;
  struct Contact *aContact, *bContact;

  struct Hashtable pairTable;
  struct Hashtable globalTable;

  struct Item *aPairItem;
  struct Pair *aPair;
  struct Item *aPairResultItem;
  struct Pair_Result *aPairResult;

  struct Item *aSingleItem;
  struct Single *aSingle;
  struct Item *aCoupleItem;
  struct Couple *aCouple;
  struct Item *aTripleItem;
  struct Triple *aTriple;
  int magicNumber;

  unsigned long pairTableSize = 10e6;
  int tGran = DEFAULT_MEETING_TEMPORAL_GRAN;
  int cGran = DEFAULT_MEETING_COUNTING_GRAN;
  int mGran = 1;
  unsigned long i, at, j, k;

  long *ict_table;
  unsigned long ictTableSize;
  long duration = 1;
  unsigned long seconds;

  struct tm *timestamp;
  unsigned long sliceTableSize;
  long *slices_in_day;
  long *contact_temperal_table;

  int t1, t2, t3;
  double pi;


  if(argc < 2) {
	printf("Usage: %s [-sp number_of_pairs] [-gt meeting_temporal_gran(sec)] [-gm meeting_at_least(times)] [-gc meeting_counting_gran(times)] [-l pair_list] [-w1 ict_length_distribution] [-w2 contact_temporal_distribution] [-w3 contact_temporal_entropy] [-w4 ict_temporal_entropy] .cont ...\n", argv[0]);
	printf("****** Note that for limited memory reason, contact and ict entropies should be calculated seperately! ******\n");
	exit(1);
  }
  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 's':
		if(argv[1][2] == 'p')
			pairTableSize = atol(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'l':
		pair_list = argv[2];
		argc-=2;
		argv+=2;
		break;

	case 'g':
		if(argv[1][2] == 't')
			tGran = atoi(argv[2]);
		if(argv[1][2] == 'c')
			cGran = atoi(argv[2]);
		if(argv[1][2] == 'm')
			mGran = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;


	case 'w':
		if(argv[1][2]=='1')
			ictdumpfile = argv[2];
		if(argv[1][2]=='2')
			cnttemperal = argv[2];
		if(argv[1][2]=='3')
			cntentropy = argv[2];
		if(argv[1][2]=='4')
			ictentropy = argv[2];
		argc-=2;
		argv+=2;
		break;
		
	default:
		printf("Usage: %s [-sp number_of_pairs] [-gt meeting_temporal_gran(sec)] [-gm meeting_at_least(times)] [-gc meeting_counting_gran(times)] [-l pair_list] [-w1 ict_length_distribution] [-w2 contact_temporal_distribution] [-w3 contact_temporal_entropy] [-w4 ict_temporal_entropy] .cont ...\n", argv[0]);
		exit(1);
	}
  }

  if(cntentropy != NULL && ictentropy !=NULL) {
	printf("****** Note that for limited memory reason, contact and ict entropies should be calculated seperately!  ******\n");
	exit(1);
  }

  duration = argc-1;
  seconds = duration * 24 * 3600;

  hashtable_init(&globalTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_result_has_names);
  if(pair_list != NULL) {
	if((fsource=fopen(pair_list, "r"))!=NULL) {
		while(fgets(buf, 1024, fsource)) {
			strp = strtok(buf, ",");
			strp1 = strtok(NULL, ",");
			sprintf(key, "%s,%s", strp, strp1);
			aPairResult = (struct Pair_Result*)malloc(sizeof(struct Pair_Result));
			pair_result_init_func(aPairResult);
			strncpy(aPairResult->vName1, strp, strlen(strp)+1);
			strncpy(aPairResult->vName2, strp1, strlen(strp1)+1);
			aPairResult->numContacts = 0;

			hashtable_add(&globalTable, key, aPairResult);
		}
		fclose(fsource);
	}
  }

  if(ictdumpfile!= NULL || cnttemperal != NULL || cntentropy != NULL || ictentropy != NULL ) {

	  if(ictdumpfile != NULL) {
		ictTableSize = seconds/tGran;
		ict_table = (long*)malloc(sizeof(long)*ictTableSize);
		for (i = 0; i<ictTableSize; i++) 
			ict_table[i] = 0;
	  }

	  if(cnttemperal != NULL) {
		sliceTableSize = 24*3600/tGran;
		slices_in_day = (long*)malloc(sizeof(long)*sliceTableSize);
		contact_temperal_table = (long*)malloc(sizeof(long)*sliceTableSize);
		for (i = 0; i<sliceTableSize; i++) 
			contact_temperal_table[i] = 0;
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
						if(pair_list==NULL && aPairResultItem == NULL) {
							aPairResult = (struct Pair_Result*)malloc(sizeof(struct Pair_Result));
							pair_result_init_func(aPairResult);
							strncpy(aPairResult->vName1, aPair->vName1, strlen(aPair->vName1)+1);
							strncpy(aPairResult->vName2, aPair->vName2, strlen(aPair->vName2)+1);
							aPairResult->numContacts = aPair->contents.nItems;

							hashtable_add(&globalTable, key, aPairResult);
						} else if(aPairResultItem != NULL){
							aPairResult = (struct Pair_Result*)aPairResultItem->datap;
							aPairResult->numContacts += aPair->contents.nItems;
						} else {
							aPairItem = aPairItem->next;
							continue;
						}

						/* contact temperal distribution */
						if(cnttemperal != NULL) {
							aContactItem = aPair->contents.head;
							for(i=0;i<sliceTableSize;i++)
								slices_in_day[i] = 0;
							while(aContactItem!=NULL) {
								aContact = (struct Contact*)aContactItem->datap;
								timestamp = localtime(&aContact->startAt);
								slices_in_day[(timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran]++;
								aContactItem = aContactItem->next;
							}
							j = 0;
							for(i=0;i<sliceTableSize;i++)
								if(slices_in_day[i])
									j ++;
							contact_temperal_table[j-1] ++;
						}

						/* contact temperal entropy*/
						if(cntentropy != NULL && !aPairResult->filtered) {
							/* entropy */
							if(aPairResult->singleTable == NULL) {
								aPairResult->singleTable = (struct Hashtable*)malloc(sizeof(struct Hashtable));
								hashtable_init(aPairResult->singleTable, 5, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))single_has_name);
							}
							/* conditional entropy knowing last contact */
							if(aPairResult->coupleTable == NULL) {
								aPairResult->coupleTable = (struct Hashtable*)malloc(sizeof(struct Hashtable));
								hashtable_init(aPairResult->coupleTable, 5, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))couple_has_names);
							}
							/* conditional entropy knowing last two contact */
							if(aPairResult->tripleTable == NULL) {
								aPairResult->tripleTable = (struct Hashtable*)malloc(sizeof(struct Hashtable));
								hashtable_init(aPairResult->tripleTable, 5, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))triple_has_names);
							}

							aContactItem = aPair->contents.head;
							if(aPairResult->last1StartAt != 0) {
								timestamp = localtime(&aPairResult->last1StartAt);
								t1 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
								aContact = (struct Contact*)aContactItem->datap;
								timestamp = localtime(&aContact->startAt);
								t2 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
								sprintf(key, "%d,%d", t1, t2);
								aCoupleItem = hashtable_find(aPairResult->coupleTable, key);
								if(aCoupleItem == NULL) {
									aCouple = (struct Couple*)malloc(sizeof(struct Couple));
									aCouple->t1 = t1;
									aCouple->t2 = t2;
									aCouple->count = 1;
									hashtable_add(aPairResult->coupleTable, key, aCouple);
								} else {
									aCouple = (struct Couple*)aCoupleItem->datap;
									aCouple->count ++;
								}
								if(aPairResult->last2StartAt != 0) {
									timestamp = localtime(&aPairResult->last2StartAt);
									t1 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
									timestamp = localtime(&aPairResult->last1StartAt);
									t2 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
									timestamp = localtime(&aContact->startAt);
									t3 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
									sprintf(key, "%d,%d,%d", t1, t2, t3);
									aTripleItem = hashtable_find(aPairResult->tripleTable, key);
									if(aTripleItem == NULL) {
										aTriple = (struct Triple*)malloc(sizeof(struct Triple));
										aTriple->t1 = t1;
										aTriple->t2 = t2;
										aTriple->t3 = t3;
										aTriple->count = 1;
										hashtable_add(aPairResult->tripleTable, key, aTriple);
									} else {
										aTriple = (struct Triple*)aTripleItem->datap;
										aTriple->count ++;
									}
								}
								if( aContactItem->next != NULL) {
									timestamp = localtime(&aPairResult->last1StartAt);
									t1 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
									timestamp = localtime(&aContact->startAt);
									t2 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
									aContact = (struct Contact*)aContactItem->next->datap;
									timestamp = localtime(&aContact->startAt);
									t3 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;
									sprintf(key, "%d,%d,%d", t1, t2, t3);
									aTripleItem = hashtable_find(aPairResult->tripleTable, key);
									if(aTripleItem == NULL) {
										aTriple = (struct Triple*)malloc(sizeof(struct Triple));
										aTriple->t1 = t1;
										aTriple->t2 = t2;
										aTriple->t3 = t3;
										aTriple->count = 1;
										hashtable_add(aPairResult->tripleTable, key, aTriple);
									} else {
										aTriple = (struct Triple*)aTripleItem->datap;
										aTriple->count ++;
									}
								}
							}

							while(aContactItem!=NULL) {
								aContact = (struct Contact*)aContactItem->datap;
								timestamp = localtime(&aContact->startAt);
								t1 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;

								sprintf(key, "%d", t1);
								aSingleItem = hashtable_find(aPairResult->singleTable, key);
								if(aSingleItem == NULL) {
									aSingle = (struct Single*)malloc(sizeof(struct Single));
									aSingle->t1 = t1;
									aSingle->count = 1;
									hashtable_add(aPairResult->singleTable, key, aSingle);
								} else {
									aSingle = (struct Single*)aSingleItem->datap;
									aSingle->count ++;
								}

								if(aContactItem->next!=NULL) {
									aContact = (struct Contact*)aContactItem->next->datap;
									timestamp = localtime(&aContact->startAt);
									t2 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;

									sprintf(key, "%d,%d", t1, t2);
									aCoupleItem = hashtable_find(aPairResult->coupleTable, key);
									if(aCoupleItem == NULL) {
										aCouple = (struct Couple*)malloc(sizeof(struct Couple));
										aCouple->t1 = t1;
										aCouple->t2 = t2;
										aCouple->count = 1;
										hashtable_add(aPairResult->coupleTable, key, aCouple);
									} else {
										aCouple = (struct Couple*)aCoupleItem->datap;
										aCouple->count ++;
									}

									if (aContactItem->next->next!=NULL) {
										aContact = (struct Contact*)aContactItem->next->next->datap;
										timestamp = localtime(&aContact->startAt);
										t3 = (timestamp->tm_hour*3600+timestamp->tm_min*60+timestamp->tm_sec)/tGran;

										sprintf(key, "%d,%d,%d", t1, t2, t3);
										aTripleItem = hashtable_find(aPairResult->tripleTable, key);
										if(aTripleItem == NULL) {
											aTriple = (struct Triple*)malloc(sizeof(struct Triple));
											aTriple->t1 = t1;
											aTriple->t2 = t2;
											aTriple->t3 = t3;
											aTriple->count = 1;
											hashtable_add(aPairResult->tripleTable, key, aTriple);
										} else {
											aTriple = (struct Triple*)aTripleItem->datap;
											aTriple->count ++;
										}
									
									}
								}

								aContactItem = aContactItem->next;
							}
						}
						
						/* ict length distribution */
						if(ictdumpfile != NULL) {
							aContactItem = aPair->contents.head;
							if(aPairResult->last1EndAt != 0) {
								aContact = (struct Contact*)aContactItem->datap;
								at = (aContact->startAt-aPairResult->last1EndAt)/tGran;
								if(at>=0 && at<ictTableSize)
									ict_table[at] ++;
							}
							while(aContactItem!=NULL && aContactItem->next!=NULL) {
								aContact = (struct Contact*)aContactItem->datap;
								bContact = (struct Contact*)aContactItem->next->datap;
								at = (bContact->startAt-aContact->endAt)/tGran;
								if(at>=0 && at<ictTableSize)
									ict_table[at] ++;
								aContactItem = aContactItem->next;
							}
						}

						/* ict temperal entropy*/
						if(ictentropy != NULL && !aPairResult->filtered) {
							/* entropy */
							if(aPairResult->singleTable == NULL) {
								aPairResult->singleTable = (struct Hashtable*)malloc(sizeof(struct Hashtable));
								hashtable_init(aPairResult->singleTable, 5, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))single_has_name);
							}
							/* ict temperal conditional entropy 1*/
							if(aPairResult->coupleTable == NULL) {
								aPairResult->coupleTable = (struct Hashtable*)malloc(sizeof(struct Hashtable));
								hashtable_init(aPairResult->coupleTable, 5, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))couple_has_names);
							}
							/* ict temperal conditional entropy 2*/
							if(aPairResult->tripleTable == NULL) {
								aPairResult->tripleTable = (struct Hashtable*)malloc(sizeof(struct Hashtable));
								hashtable_init(aPairResult->tripleTable, 5, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))triple_has_names);
							}

							aContactItem = aPair->contents.head;
							if(aPairResult->last1EndAt != 0) {
								aContact = (struct Contact*)aContactItem->datap;
								t1 = (aContact->startAt-aPairResult->last1EndAt)/tGran;
								sprintf(key, "%d", t1);
								aSingleItem = hashtable_find(aPairResult->singleTable, key);
								if(aSingleItem == NULL) {
									aSingle = (struct Single*)malloc(sizeof(struct Single));
									aSingle->t1 = t1;
									aSingle->count = 1;
									hashtable_add(aPairResult->singleTable, key, aSingle);
								} else {
									aSingle = (struct Single*)aSingleItem->datap;
									aSingle->count ++;
								}

								if(aContactItem->next != NULL) {
									bContact = (struct Contact*)aContactItem->next->datap;
									t2 = (bContact->startAt-aContact->endAt)/tGran;
									sprintf(key, "%d,%d", t1, t2);
									aCoupleItem = hashtable_find(aPairResult->coupleTable, key);
									if(aCoupleItem == NULL) {
										aCouple = (struct Couple*)malloc(sizeof(struct Couple));
										aCouple->t1 = t1;
										aCouple->t2 = t2;
										aCouple->count = 1;
										hashtable_add(aPairResult->coupleTable, key, aCouple);
									} else {
										aCouple = (struct Couple*)aCoupleItem->datap;
										aCouple->count ++;
									}
									if(aPairResult->last2StartAt != 0) {
										t1 = (aPairResult->last1StartAt - aPairResult->last2EndAt)/tGran;
										t2 = (aContact->startAt - aPairResult->last1EndAt)/tGran;
										t3 = (bContact->startAt - aContact->endAt)/tGran;
										sprintf(key, "%d,%d,%d", t1, t2, t3);
										aTripleItem = hashtable_find(aPairResult->tripleTable, key);
										if(aTripleItem == NULL) {
											aTriple = (struct Triple*)malloc(sizeof(struct Triple));
											aTriple->t1 = t1;
											aTriple->t2 = t2;
											aTriple->t3 = t3;
											aTriple->count = 1;
											hashtable_add(aPairResult->tripleTable, key, aTriple);
										} else {
											aTriple = (struct Triple*)aTripleItem->datap;
											aTriple->count ++;
										}
									}
									if(aContactItem->next->next != NULL) {
										t1 = (aContact->startAt - aPairResult->last1EndAt)/tGran;
										t2 = (bContact->startAt - aContact->endAt)/tGran;
										aContact = (struct Contact*)aContactItem->next->next->datap;
										t3 = (aContact->startAt-bContact->endAt)/tGran;
										sprintf(key, "%d,%d,%d", t1, t2, t3);
										aTripleItem = hashtable_find(aPairResult->tripleTable, key);
										if(aTripleItem == NULL) {
											aTriple = (struct Triple*)malloc(sizeof(struct Triple));
											aTriple->t1 = t1;
											aTriple->t2 = t2;
											aTriple->t3 = t3;
											aTriple->count = 1;
											hashtable_add(aPairResult->tripleTable, key, aTriple);
										} else {
											aTriple = (struct Triple*)aTripleItem->datap;
											aTriple->count ++;
										}
									}
								}
							}

							while(aContactItem!=NULL && aContactItem->next!=NULL) {
								aContact = (struct Contact*)aContactItem->datap;
								bContact = (struct Contact*)aContactItem->next->datap;
								t1 = (bContact->startAt-aContact->endAt)/tGran;
								sprintf(key, "%d", t1);
								aSingleItem = hashtable_find(aPairResult->singleTable, key);
								if(aSingleItem == NULL) {
									aSingle = (struct Single*)malloc(sizeof(struct Single));
									aSingle->t1 = t1;
									aSingle->count = 1;
									hashtable_add(aPairResult->singleTable, key, aSingle);
								} else {
									aSingle = (struct Single*)aSingleItem->datap;
									aSingle->count ++;
								}

								if(aContactItem->next->next != NULL) {
									aContact = (struct Contact*)aContactItem->next->next->datap;
									t2 = (aContact->startAt-bContact->endAt)/tGran;

									sprintf(key, "%d,%d", t1, t2);
									aCoupleItem = hashtable_find(aPairResult->coupleTable, key);
									if(aCoupleItem == NULL) {
										aCouple = (struct Couple*)malloc(sizeof(struct Couple));
										aCouple->t1 = t1;
										aCouple->t2 = t2;
										aCouple->count = 1;
										hashtable_add(aPairResult->coupleTable, key, aCouple);
									} else {
										aCouple = (struct Couple*)aCoupleItem->datap;
										aCouple->count ++;
									}
									if(aContactItem->next->next->next != NULL) {
										bContact = (struct Contact*)aContactItem->next->next->next->datap;
										t3 = (bContact->startAt - aContact->endAt)/tGran;

										sprintf(key, "%d,%d,%d", t1, t2, t3);
										aTripleItem = hashtable_find(aPairResult->tripleTable, key);
										if(aTripleItem == NULL) {
											aTriple = (struct Triple*)malloc(sizeof(struct Triple));
											aTriple->t1 = t1;
											aTriple->t2 = t2;
											aTriple->t3 = t3;
											aTriple->count = 1;
											hashtable_add(aPairResult->tripleTable, key, aTriple);
										} else {
											aTriple = (struct Triple*)aTripleItem->datap;
											aTriple->count ++;
										}
									}

								}

								aContactItem = aContactItem->next;
							}
						}
						


						if (aPair->contents.head->prev != aPair->contents.head) {
							aContact = (struct Contact*)aPair->contents.head->prev->prev->datap;
							aPairResult->last2StartAt = aContact->startAt;
							aPairResult->last2EndAt = aContact->endAt;
						} else {
							aPairResult->last2StartAt = aPairResult->last1StartAt;
							aPairResult->last2EndAt = aPairResult->last1EndAt;
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

	  if(cnttemperal != NULL) {
		  fdump = fopen(cnttemperal, "w");
		  for (i = 0; i<sliceTableSize; i++) 
			fprintf(fdump, "%ld %ld\n", i+1, contact_temperal_table[i]);
		  fclose(fdump);
	  }

	  if(cntentropy != NULL) {
		  fdump = fopen(cntentropy, "w");
		  for (k=0;k<globalTable.size;k++) {
			aPairResultItem = globalTable.head[k];
			while (aPairResultItem != NULL) {
				aPairResult = (struct Pair_Result*)aPairResultItem->datap;
				if(aPairResult->numContacts >= mGran && !aPairResult->filtered) {
					aPairResult->total = 0;
					for (j=0;j<aPairResult->singleTable->size;j++) {
					      aSingleItem = aPairResult->singleTable->head[j];
					      while (aSingleItem != NULL) {
							aSingle = (struct Single*)aSingleItem->datap;
							aSingle->count = ceil(aSingle->count*1.0/cGran);
							aPairResult->total += aSingle->count;
							aSingleItem = aSingleItem->next;
					      }
					}
					for (j=0;j<aPairResult->singleTable->size;j++) {
					      aSingleItem = aPairResult->singleTable->head[j];
					      while (aSingleItem != NULL) {
							aSingle = (struct Single*)aSingleItem->datap;
							pi = aSingle->count*1.0/(aPairResult->total);
							aPairResult->entropy -= pi*log(pi)/log(2);	
							aSingleItem = aSingleItem->next;
					      }
					}
					aPairResult->total = 0;
					for (j=0;j<aPairResult->coupleTable->size;j++) {
					      aCoupleItem = aPairResult->coupleTable->head[j];
					      while (aCoupleItem != NULL) {
							aCouple = (struct Couple*)aCoupleItem->datap;
							aCouple->count = ceil(aCouple->count*1.0/cGran);
							aPairResult->total += aCouple->count;
							aCoupleItem = aCoupleItem->next;
					      }
					}
					for (j=0;j<aPairResult->coupleTable->size;j++) {
					      aCoupleItem = aPairResult->coupleTable->head[j];
					      while (aCoupleItem != NULL) {
							aCouple = (struct Couple*)aCoupleItem->datap;
							pi = aCouple->count*1.0/(aPairResult->total);
							aPairResult->entropy1 -= pi*log(pi)/log(2);	
							aCoupleItem = aCoupleItem->next;
					      }
					}
					aPairResult->total = 0;
					for (j=0;j<aPairResult->tripleTable->size;j++) {
					      aTripleItem = aPairResult->tripleTable->head[j];
					      while (aTripleItem != NULL) {
							aTriple = (struct Triple*)aTripleItem->datap;
							aTriple->count = ceil(aTriple->count*1.0/cGran);
							aPairResult->total += aTriple->count;
							aTripleItem = aTripleItem->next;
					      }
					}
					for (j=0;j<aPairResult->tripleTable->size;j++) {
					      aTripleItem = aPairResult->tripleTable->head[j];
					      while (aTripleItem != NULL) {
							aTriple = (struct Triple*)aTripleItem->datap;
							pi = aTriple->count*1.0/(aPairResult->total);
							aPairResult->entropy2 -= pi*log(pi)/log(2);	
							aTripleItem = aTripleItem->next;
					      }
					}
					if(aPairResult->entropy1>aPairResult->entropy && aPairResult->entropy2>aPairResult->entropy1)
						fprintf(fdump, "%.4lf %.4lf %.4lf\n", aPairResult->entropy, aPairResult->entropy1-aPairResult->entropy, aPairResult->entropy2-aPairResult->entropy1);
				}
				aPairResultItem = aPairResultItem->next;
			}
		  }
		  fclose(fdump);
	  }


	  if(ictdumpfile != NULL) {
		  fdump = fopen(ictdumpfile, "w");
		  for (i = 0; i<ictTableSize; i++) 
			fprintf(fdump, "%ld %ld\n", i, ict_table[i]);
		  fclose(fdump);
		  free(ict_table);
	  }

	 
	  if(ictentropy != NULL) {
		  fdump = fopen(ictentropy, "w");
		  for (k=0;k<globalTable.size;k++) {
			aPairResultItem = globalTable.head[k];
			while (aPairResultItem != NULL) {
				aPairResult = (struct Pair_Result*)aPairResultItem->datap;
				if(aPairResult->numContacts >= mGran && !aPairResult->filtered) {
					aPairResult->total = 0;
					for (j=0;j<aPairResult->singleTable->size;j++) {
					      aSingleItem = aPairResult->singleTable->head[j];
					      while (aSingleItem != NULL) {
							aSingle = (struct Single*)aSingleItem->datap;
							aSingle->count = ceil(aSingle->count*1.0/cGran);
							aPairResult->total += aSingle->count;
							aSingleItem = aSingleItem->next;
					      }
					}
					for (j=0;j<aPairResult->singleTable->size;j++) {
					      aSingleItem = aPairResult->singleTable->head[j];
					      while (aSingleItem != NULL) {
							aSingle = (struct Single*)aSingleItem->datap;
							pi = aSingle->count*1.0/aPairResult->total;
							aPairResult->entropy -= pi*log(pi)/log(2);	
							aSingleItem = aSingleItem->next;
					      }
					}


					aPairResult->total = 0;
					for (j=0;j<aPairResult->coupleTable->size;j++) {
					      aCoupleItem = aPairResult->coupleTable->head[j];
					      while (aCoupleItem != NULL) {
							aCouple = (struct Couple*)aCoupleItem->datap;
							aCouple->count = ceil(aCouple->count*1.0/cGran);
							aPairResult->total += aCouple->count;
							aCoupleItem = aCoupleItem->next;
					      }
					}
					for (j=0;j<aPairResult->coupleTable->size;j++) {
					      aCoupleItem = aPairResult->coupleTable->head[j];
					      while (aCoupleItem != NULL) {
							aCouple = (struct Couple*)aCoupleItem->datap;
							pi = aCouple->count*1.0/aPairResult->total;
							aPairResult->entropy1 -= pi*log(pi)/log(2);	
							aCoupleItem = aCoupleItem->next;
					      }
					}

					aPairResult->total = 0;
					for (j=0;j<aPairResult->tripleTable->size;j++) {
					      aTripleItem = aPairResult->tripleTable->head[j];
					      while (aTripleItem != NULL) {
							aTriple = (struct Triple*)aTripleItem->datap;
							aTriple->count = ceil(aTriple->count*1.0/cGran);
							aPairResult->total += aTriple->count;
							aTripleItem = aTripleItem->next;
					      }
					}
					for (j=0;j<aPairResult->tripleTable->size;j++) {
					      aTripleItem = aPairResult->tripleTable->head[j];
					      while (aTripleItem != NULL) {
							aTriple = (struct Triple*)aTripleItem->datap;
							pi = aTriple->count*1.0/aPairResult->total;
							aPairResult->entropy2 -= pi*log(pi)/log(2);	
							aTripleItem = aTripleItem->next;
					      }
					}
					if(aPairResult->entropy1>aPairResult->entropy && aPairResult->entropy2>aPairResult->entropy1)
						fprintf(fdump, "%.4lf %.4lf %.4lf\n", aPairResult->entropy, aPairResult->entropy1-aPairResult->entropy, aPairResult->entropy2-aPairResult->entropy1);
				}
				aPairResultItem = aPairResultItem->next;
			}
		  }
		  fclose(fdump);
	  }

  }

  hashtable_destroy(&globalTable, (void(*)(void*))pair_result_free_func);
  return 0;
}
