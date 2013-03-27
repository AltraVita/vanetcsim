#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include"trace.h"
#include"common.h"
#include"files.h"

int main(int argc, char **argv)
{
	FILE *fsource, *fdump;
	char dumpfile[256], *directory, buf[32], vName[32];

	struct Hashtable traces;
	struct Item *aItem;
	struct Trace *aTrace;
	char *strp, *strp1, *strp2;

	unsigned long i;


	if(argc < 2) {
	      printf("Usage: %s [-d directory] .ogd|.mgd|.lst ...\n", argv[0]);
	      exit(1);
	}

	directory = ".";
	while(argv[1][0] == '-') {
	      switch(argv[1][1]) {
	      case 'd':
		      directory = argv[2];
		      argc-=2;
		      argv+=2;
		      break;
	      default:
	      		printf("Usage: %s [-d directory] .ogd|.mgd|.lst ...\n", argv[0]);
	      }
	}

	hashtable_init(&traces, 2000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			printf("Loading %s file ...\n", argv[1]);
			load_source_file(fsource, NULL, &traces, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}


	if(traces.count != 0) {
	      for (i = 0; i<traces.size; i++) {
		      aItem = traces.head[i];
		      while(aItem != NULL ) {
				aTrace = (struct Trace*)aItem->datap;
				memset(vName, 0, 32);
				if(aTrace->type == FILE_ORIGINAL_GPS_TAXI || aTrace->type == FILE_MODIFIED_GPS_TAXI) {
					strcat(vName, "t");
				}
				if(aTrace->type == FILE_ORIGINAL_GPS_BUS || aTrace->type == FILE_MODIFIED_GPS_BUS) {
					strcat(vName, "b");
				}

				strncpy(buf, aTrace->vName, strlen(aTrace->vName)+1);
				strp = strtok(buf, "_");
				strp1 = strtok(NULL, "_");
				strp2 = strtok(NULL, "_");

				strcat(vName, strp);

				if(strp1 && strcmp(strp1,"amended")==0)
					strcat(vName, "a");
				if(strp2 && strcmp(strp2,"inserted")==0)
					strcat(vName, "i");

				strncpy(aTrace->vName, vName, strlen(vName)+1);
				mkdir(directory,0777);
				if(aTrace->type == FILE_ORIGINAL_GPS_TAXI || aTrace->type == FILE_ORIGINAL_GPS_BUS) 
					sprintf(dumpfile, "%s/%s.ogd", directory, aTrace->vName);
				if(aTrace->type == FILE_MODIFIED_GPS_TAXI || aTrace->type == FILE_MODIFIED_GPS_BUS) 
					sprintf(dumpfile, "%s/%s.mgd", directory, aTrace->vName);

				if( (fdump = fopen(dumpfile, "w"))!=NULL) {
					trace_dump_func(fdump, aTrace);
					fflush(fdump);
					fclose(fdump);
				}

			      	aItem = aItem->next;
		      }
	      }
	} 
	 
	hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
	return 0;
}
