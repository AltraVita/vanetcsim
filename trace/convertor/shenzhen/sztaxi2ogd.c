/* A data format convertor, translating the GPS data collected from Shenzhen taxies into .ogd format */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"
#include"files.h"

struct 
{
	float x;
	float y;
	time_t timestamp;
	char state;
	char warning;
	short speed;
	short angle;
	int miles;
	short temp[6];
	short oil;
} aRep;

union EightBytes
{
	char bytes[8];
	char aChar;
	short aShort;
	int anInt;
	float aFloat;
	long long aLongLong;
};

int main(int argc, char **argv)
{
	FILE *fsource, *fdest;
	char buf[128], name[128], vname[128], *strp;
	int delete = 0;
	union EightBytes item;
	long long aTime;
	int i;

	if(argc < 2) {
		printf("Usage: %s [-d] .LOG \n", argv[0]);
		exit(1);
	}

	if(strcmp(argv[1], "-d") == 0)  {
		delete = 1;
		argc --;
		argv ++;
	}

	if( strstr(argv[1], ".LOG") == NULL) {
//		printf("%s is not leagle .LOG file.\n", argv[1]);
		exit(-1);
	}

	if( (fsource = fopen(argv[1], "r")) == NULL) {
		printf("Cannot open %s.\n", argv[1]);
		exit(-1);
	}

	strncpy(buf, argv[1], strlen(argv[1])+1);
	strp = strtok(buf, ".");
	sprintf(name, "t%s", strp);
	strncpy(vname, name, strlen(name)+1);
	strncat(name, ".ogd", 4);
	if( (fdest = fopen(name, "w")) == NULL) {
		printf("Cannot open %s to write.", name);
		exit(-1);
	}

	fprintf(fdest, "%d\n", FILE_ORIGINAL_GPS_TAXI);
	while (fgetc(fsource)!=EOF)
	{
		fseek(fsource, -1, SEEK_CUR);

		fread(&item.bytes, sizeof(char), 4, fsource);	
		big2little_(&item.bytes, 4);
		aRep.x = item.aFloat;

		fread(&item.bytes, sizeof(char), 4, fsource);	
		big2little_(&item.bytes, 4);
		aRep.y = item.aFloat;

		fread(&item.bytes, sizeof(char), 8, fsource);	
		big2little_(&item.bytes, 8);
		aTime = item.aLongLong;
		aTime /= 1000;
		aRep.timestamp = (time_t)aTime;

		fread(&item.bytes, sizeof(char), 1, fsource);	
		aRep.state = item.aChar & 0x7;
		aRep.warning = item.aChar >> 3;
		aRep.warning = aRep.warning & 0x1f;

		fread(&item.bytes, sizeof(char), 2, fsource);	
		big2little_(&item.bytes, 2);
		aRep.speed = item.aShort;

		fread(&item.bytes, sizeof(char), 2, fsource);	
		big2little_(&item.bytes, 2);
		aRep.angle = item.aShort;

		fread(&item.bytes, sizeof(char), 4, fsource);	
		big2little_(&item.bytes, 4);
		aRep.miles = item.anInt;

		for (i=0; i<6; i++) {
			fread(&item.bytes, sizeof(char), 2, fsource);	
			big2little_(&item.bytes, 2);
			aRep.temp[i] = item.aShort;
		}

		fread(&item.bytes, sizeof(char), 2, fsource);	
		big2little_(&item.bytes, 2);
		aRep.oil = item.aShort;

		ttostr(aRep.timestamp, buf);
		fprintf(fdest, "%s,%s,%.5f,%.5f,%3d,%3d,%3d,%3d\n",
			        vname,		/* vehicle name */ 
				buf, 		/* report timestamp */
				aRep.x,		/* lattitude */
				aRep.y, 	/* longitude */
				aRep.speed, 	/* speed */
				aRep.angle*45,/* heading */
				aRep.state, 	/* heavy or light */
				aRep.warning	/* warnings */);
	}

	fclose(fsource);
	fflush(fdest);
	fclose(fdest);
	

	if(delete) 
		remove(argv[1]);
	return 0;
}
