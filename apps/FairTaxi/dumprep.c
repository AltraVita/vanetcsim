#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"common.h"

struct TaxiReps
{
	char vname[32];
	double maxvalue;
	double minvalue;
	double avgvalue;
	struct Duallist reps;
};
	
int value_is_smaller_than(double *aValue, double *bValue)
{
	return *aValue < *bValue;
} 


void load_a_repfile(FILE *fp, struct Hashtable *taxiTable)
{
	struct TaxiReps *aTaxi;
	double  *newvalue;
	char buf[128], *vName, *strp;
	struct Item *aItem;

	if(taxiTable == NULL)
		return ;
	while(fgets(buf, 128, fp)) {
		newvalue = (double *)malloc(sizeof(double));

		vName = strtok(buf, ",");
		strp = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		strp = strtok(NULL, ",");
		*newvalue = atof(strp);

		aItem = hashtable_find(taxiTable, vName);
		if(aItem == NULL) {
			aTaxi = (struct TaxiReps*)malloc(sizeof(struct TaxiReps));
			strncpy(aTaxi->vname, vName, strlen(vName)+1);
			duallist_init(&aTaxi->reps);
			hashtable_add(taxiTable, vName, aTaxi);
			aTaxi->maxvalue = *newvalue;
			aTaxi->minvalue = *newvalue;
		} else {
			aTaxi = (struct TaxiReps*)aItem->datap;
			if(*newvalue < aTaxi->minvalue)
				aTaxi->minvalue = *newvalue;
			if(*newvalue > aTaxi->maxvalue)
				aTaxi->maxvalue = *newvalue;
		}

		duallist_add_in_sequence_from_head(&aTaxi->reps, newvalue, (int(*)(void*, void*))value_is_smaller_than);

	}
}

void compute_a_taxi(struct TaxiReps *ataxi)
{
	struct Item *aItem;
	double sum = 0;

	if(ataxi == NULL) return;

	aItem = ataxi->reps.head->next;
	while(aItem != NULL) {
		sum += *((double*)aItem->datap);
		aItem = aItem->next;
	}
	if(ataxi->reps.nItems - 1 > 0)
		ataxi->avgvalue = sum/(ataxi->reps.nItems - 1);
	else
		ataxi->avgvalue = -1;
}

int taxi_has_name(char *name, struct TaxiReps* ataxi)
{
	return !strcmp(name, ataxi->vname);
}

void taxi_free_func(struct TaxiReps *ataxi)
{
	if(ataxi==NULL) 
		return;
	duallist_destroy(&ataxi->reps, free);
	free(ataxi);
}

int main(int argc, char**argv)
{
	double threshold = 0;
	FILE *flist, *fp;
	char buf[128], filename[128];
	struct Hashtable taxiTable;
	struct Item *aItem;
	int at;
	
	if(argc < 2) {
		printf("Usage: %s [-v threshold] taxi_repfile_list ...\n", argv[0]);
		exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'v':
			threshold = atof(argv[2]);
			argc-=2;
			argv+=2;
			break;
		}
	}

  	hashtable_init(&taxiTable, 15000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))taxi_has_name);

	while(argc>1) {
		flist = fopen(argv[1], "r");
		while (fgets(buf, 127, flist)) {
			sscanf(buf, "%s\n", filename);
			if( (fp=fopen(filename, "r"))!=NULL) {
				load_a_repfile(fp, &taxiTable);
				fclose(fp);
			}
		}
		fclose(flist);
		argc --;
		argv ++;
	}
	

	for(at = 0; at<taxiTable.size; at++) {
		aItem = taxiTable.head[at];
		while (aItem!=NULL)
		{
			compute_a_taxi((struct TaxiReps*)aItem->datap);
			if(((struct TaxiReps*)aItem->datap)->avgvalue >= threshold)
				printf("%s,%.1f,%.1f,%.1f\n",	((struct TaxiReps*)aItem->datap)->vname,
								((struct TaxiReps*)aItem->datap)->maxvalue,
								((struct TaxiReps*)aItem->datap)->avgvalue,
								((struct TaxiReps*)aItem->datap)->minvalue);
			aItem = aItem->next;
		}
	}
	
  	hashtable_destroy(&taxiTable, (void(*)(void*))taxi_free_func);
	return 0;
}
