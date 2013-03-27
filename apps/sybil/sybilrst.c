#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"
#include"files.h"


#define HONEST_TRAJ 0
#define MALICIOUS_TRAJ 1

struct Honest
{
  char name[64];
  int state;
};

struct Attacker
{
  char name[64];
  struct Duallist spoofs;
};

int honest_has_name(char *name, struct Honest* aHonest)
{
	return !strcmp(name, aHonest->name);
}

int attacker_has_name(char *name, struct Honest* aHonest)
{
	return !strcmp(name, aHonest->name);
}

int is_name_honest(char *name)
{
	char buf[64], *p;

	strncpy(buf, name, strlen(name)+1);
	p = strtok(buf, "_");
	p = strtok(NULL, "_");
	p = strtok(NULL, "_");
	p = strtok(NULL, "_");
	if(p) return 0;
	return 1;
}


char * get_attacker_name(char *name)
{
	char buf[64], *p;

	memset(buf, 0, 64);
	p = strtok(name, "_");
	strncat(buf, p, strlen(p));
	strncat(buf, "_", 1);
	p = strtok(NULL, "_");
	strncat(buf, p, strlen(p));
	strncat(buf, "_", 1);
	p = strtok(NULL, "_");
	strncat(buf, p, strlen(p));
	strncpy(name, buf, strlen(buf)+1);

	return name;
}


int main(int argc, char**argv)
{
	FILE *fp;
	int i, *newInt, magicNumber;
	double numSpoofs, missedSpoofs, temp;
	struct Item *aItem, *bItem;
	struct Honest *anHonest;
	struct Attacker *anAttacker;
	char buf[256], name[64], *strp;
	struct Duallist honests, attackers;
	double falsePositive, falseNegative;

	if(argc < 3) {
	      printf("%s is used to report the false positive and false negative error of SHIELD scheme.\n", argv[0]);
	      printf("Usage: %s trustworthy.rpt malicious.rpt\n", argv[0]);
	      exit(1);
	}

	duallist_init(&honests);
	duallist_init(&attackers);

	for(i=0;i<2;i++) {
		fp = fopen(argv[1], "r");
		fscanf(fp, "%d\n", &magicNumber);
		if(magicNumber == FILE_SYBIL_HONEST_TRAJ) {
			memset(buf, '\0', 256);
			while (fgets(buf, 256, fp)) {
				strp = strtok(buf, ",");
				strncpy(name, strp, strlen(strp)+1);
				if(is_name_honest(name)) {
					anHonest = (struct Honest*)malloc(sizeof(struct Honest));
					strncpy(anHonest->name, name, strlen(name)+1);
					anHonest->state = HONEST_TRAJ;	
					duallist_add_to_tail(&honests, anHonest);
				} else {
					strp = get_attacker_name(name);
					aItem = duallist_find(&attackers, strp, (int(*)(void*,void*))attacker_has_name);
					if(aItem) {
						anAttacker = (struct Attacker*)aItem->datap;
						newInt =(int*)malloc(sizeof(int));
						*newInt = HONEST_TRAJ;
						duallist_add_to_tail(&anAttacker->spoofs, newInt);
					} else {
						anAttacker = (struct Attacker*)malloc(sizeof(struct Attacker));
						strncpy(anAttacker->name, strp, strlen(strp)+1);
						duallist_init(&anAttacker->spoofs);
						newInt =(int*)malloc(sizeof(int));
						*newInt = HONEST_TRAJ;
						duallist_add_to_tail(&anAttacker->spoofs, newInt);
						duallist_add_to_tail(&attackers, anAttacker);
					}
				}	
			}
		} else if(magicNumber == FILE_SYBIL_MALICIOUS_TRAJ) {
			memset(buf, '\0', 256);
			while (fgets(buf, 256, fp)) {
				strp = strtok(buf, ",");
				strncpy(name, strp, strlen(strp)+1);
				if(is_name_honest(name)) {
					anHonest = (struct Honest*)malloc(sizeof(struct Honest));
					strncpy(anHonest->name, name, strlen(name)+1);
					anHonest->state = MALICIOUS_TRAJ;	
					duallist_add_to_tail(&honests, anHonest);
				} else {
					strp = get_attacker_name(name);
					aItem = duallist_find(&attackers, strp, (int(*)(void*,void*))attacker_has_name);
					if(aItem) {
						anAttacker = (struct Attacker*)aItem->datap;
						newInt =(int*)malloc(sizeof(int));
						*newInt = MALICIOUS_TRAJ;
						duallist_add_to_tail(&anAttacker->spoofs, newInt);
					} else {
						anAttacker = (struct Attacker*)malloc(sizeof(struct Attacker));
						strncpy(anAttacker->name, strp, strlen(strp)+1);
						duallist_init(&anAttacker->spoofs);
						newInt =(int*)malloc(sizeof(int));
						*newInt = MALICIOUS_TRAJ;
						duallist_add_to_tail(&anAttacker->spoofs, newInt);
						duallist_add_to_tail(&attackers, anAttacker);
					}
				}	
			
			}
		}
		fclose(fp);
		argc--;
		argv++;
	}

	falsePositive = 0;
	aItem = honests.head;
	while(aItem) {
		anHonest = (struct Honest*)aItem->datap;
		if(anHonest->state == MALICIOUS_TRAJ)
			falsePositive += 1;
		aItem = aItem->next;
	}
	if(honests.nItems)
		falsePositive /= honests.nItems;

	falseNegative = 0;
	numSpoofs = 0;
	missedSpoofs = 0;
	aItem = attackers.head;
	while(aItem) {
		anAttacker = (struct Attacker*)aItem->datap;
		numSpoofs += anAttacker->spoofs.nItems;
		temp = 0;
		bItem = anAttacker->spoofs.head;
		while(bItem) {
			if(*(int*)bItem->datap == HONEST_TRAJ)
				temp += 1;
			bItem = bItem->next;
		}
		if (temp>1)
			missedSpoofs += temp-1;
		aItem = aItem->next;
	}

	if(numSpoofs - attackers.nItems)
		falseNegative = missedSpoofs/(numSpoofs - attackers.nItems);

	printf("%.2lf %.2lf\n", falsePositive, falseNegative);

	duallist_destroy(&honests, free);
	aItem = attackers.head;
	while(aItem) {
		anAttacker = (struct Attacker*)aItem->datap;
		duallist_destroy(&anAttacker->spoofs, free);
		aItem = aItem->next;
	}
	duallist_destroy(&attackers, free);
	return 0;
}
