#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include"common.h"
#include"simulator.h"
#include"oracle.h"
#include"node.h"
#include"pkg.h"
#include"traffic.h"
#include"contact.h"
#include"files.h"
#include"traffic.h"
#include"busArriveCellEvent.h"
#include"busMeetBusEvent.h"
#include"busMeetStorageEvent.h"
#include"taxiMeetBusEvent.h"
#include"taxiMeetStorageEvent.h"
#include"cntEvent.h"

void setup_b2l_traffic_events(struct Simulator *aSim);

int main( int   argc,
          char *argv[] )
{
  time_t starttime = 0, endtime = 0;
  int fwdMethod = NO_REPLICA_FWD, httl = 1; 
  int traffictype = TYPE_TRAFFIC_GENERATOR_POISSON, poissonMean = 10, numPkgs = 100, bufSize = -1, pkgSize = 1, randseed = 1111;
  struct Simulator *aSim;
  struct Hashtable cntTable, traceTable;
  FILE *fsource, *ftraffic, *fpkgdump, *fdlydump, *fdlvdump, *ftrffdump;

  struct Trace *aTrace;
  unsigned long i, totalevents;
  
  struct Item *aItem;
  struct Node *aNode;
  struct Pkg *aPkg;

  double cper, lper;
  int recordRoute = 0;
  char *strp, *trafficfile = NULL, *pkgdumpfile = NULL, *dlydumpfile = NULL, *dlvdumpfile = NULL, *trffdumpfile = NULL;

  if(argc < 2) {
	printf("Usage: %s [-time exprStartAt exprEndAt] [-forward [no_replica|keep_replica]] [-helper maxrelay] [-traffic [poisson Mean(sec) numPkgs]|[load file.tffc]] [-sbuffer Size] [-spacket size] [-i randseed] [-record] [-w1 pkgdump] [-w2 delaydump] [-w3 deliverydump] [-w4 trafficdump] .map [.bus .cont .mgd ...]\n", argv[0]);
	exit(1);
  }
  while(argv[1]!=NULL && argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		if(strcmp("-time",argv[1])==0) {
			starttime = strtot(argv[2]);
			endtime = strtot(argv[3]);
			argc-=3;
			argv+=3;
		} else if (strcmp("-traffic", argv[1])==0) {
			if (strcmp("poisson", argv[2])==0) {
				traffictype = TYPE_TRAFFIC_GENERATOR_POISSON;
				poissonMean = atoi(argv[3]);
				numPkgs = atoi(argv[4]);
				argc-=4;
				argv+=4;
			} else if (strcmp("load", argv[2])==0) {
				traffictype = TYPE_TRAFFIC_GENERATOR_LOAD;
				trafficfile = argv[3];
				argc-=3;
				argv+=3;
			}
		}
		break;

	case 'f':
		if(strcmp("no_replica",argv[2])==0) {
			fwdMethod = NO_REPLICA_FWD;
		} else if (strcmp("keep_replica", argv[2])==0) {
			fwdMethod = KEEP_REPLICA_FWD;
		}
		argc-=2;
		argv+=2;
		break;

	case 'h':
		httl = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 's':
		if(argv[1][2] == 'b')
			bufSize = atoi(argv[2]);
		if(argv[1][2] == 'p')
			pkgSize = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'i':
		randseed = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'r':
		recordRoute = 1;
		argc-=1;
		argv+=1;
		break;

	case 'w':
		if(argv[1][2] == '1')
			pkgdumpfile = argv[2];
		if(argv[1][2] == '2')
			dlydumpfile = argv[2];
		if(argv[1][2] == '3')
			dlvdumpfile = argv[2];
		if(argv[1][2] == '4')
			trffdumpfile = argv[2];
		argc-=2;
		argv+=2;
		break;
	default:
		printf("Usage: %s [-time exprStartAt exprEndAt] [-forward [no_replica|keep_replica]] [-helper maxrelay] [-traffic [poisson Mean(sec) numPkgs]|[load file.tffc]] [-sbuffer Size] [-spacket size] [-i randseed] [-record] [-w1 pkgdump] [-w2 delaydump] [-w3 deliverydump] [-w4 trafficdump] .map [.bus .cont .mgd ...]\n", argv[0]);
		exit(1);
	}
  }

  srand(randseed);

  aSim = (struct Simulator*)malloc(sizeof(struct Simulator));
  simulator_init_func(aSim, starttime, endtime, fwdMethod, bufSize, pkgSize, httl, recordRoute);
  // setup experiment region
  if((fsource=fopen(argv[1], "rb"))!=NULL) {
	printf("Loading map ...\n");
  	aSim->region = region_load_func(fsource, NULL, -1, -1);
	fclose(fsource);
  }

  // load bus & taxi traces, contacts between buses and between buses and taxies
  hashtable_init(&traceTable, 4000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
  hashtable_init(&cntTable, 1000000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  while(argc>2) {
	if((fsource=fopen(argv[2], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[2]);
		load_source_file(fsource, aSim->region, &traceTable, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, &cntTable, PAIRWISE_TABLE, (void*(*)(FILE*, struct Region*, void *, int, time_t *, time_t *))load_contacts_with_hashtable, NULL, NULL, NULL, NULL, &aSim->routes, (void*(*)(FILE*, struct Region*, void *))load_route_with_hashtable, &starttime, &endtime, NULL);
		fclose(fsource);
	}
	argc--;
	argv++;
  }
  if(aSim->exprStartAt == 0) {
	aSim->exprStartAt = starttime;
	aSim->exprEndAt = endtime;
  }
  // setup vnodes
  for (i = 0; i<traceTable.size; i++) {
  	aItem = traceTable.head[i];
      	while(aItem != NULL ) {
		aTrace = (struct Trace*)aItem->datap;
		aNode=lookup_node(&aSim->vnodes, aTrace->vName);
		if(aNode == NULL) {
			if(aTrace->vName[0]=='b') {
				aNode = (struct Node*)malloc(sizeof(struct Node));
				strp = (char*)malloc(sizeof(char)*NAME_LENGTH);	
				sprintf(strp, "%s_upway", aTrace->onRoute);
				node_init_func(aNode, aTrace->vName, strp, aSim->bufSize);
				hashtable_add(&aSim->vnodes, aNode->name, aNode);

				aNode = (struct Node*)malloc(sizeof(struct Node));
				strp = (char*)malloc(sizeof(char)*NAME_LENGTH);	
				sprintf(strp, "%s_downway", aTrace->onRoute);
				node_init_func(aNode, aTrace->vName, strp, aSim->bufSize);
				hashtable_add(&aSim->vnodes, aNode->name, aNode);
			} else {
				aNode = (struct Node*)malloc(sizeof(struct Node));
				node_init_func(aNode, aTrace->vName, NULL, aSim->bufSize);
				hashtable_add(&aSim->vnodes, aNode->name, aNode);
			}
		}
		aItem = aItem->next;
	}
  }
  printf("There %ld vehicular nodes.\n", aSim->vnodes.count);

  // setup snodes
  setup_cells_with_routes(aSim->region, &aSim->routes);
  deploy_static_nodes_at_most_routes_cells(aSim->region, &aSim->snodes, aSim->bufSize);
  printf("There %ld static storage nodes.\n", aSim->snodes.count);
  
  // setup traffic generator
  if (traffictype == TYPE_TRAFFIC_GENERATOR_POISSON) {
	aSim->trafficGenerator = (struct TrafficGenerator*)malloc(sizeof(struct TrafficGenerator));
	trafficGenerator_init_func(aSim->trafficGenerator, aSim->exprStartAt, numPkgs, poissonMean, SELECT_RANDOM);
	generate_b2l_poisson_traffic(aSim);
  } else if (traffictype == TYPE_TRAFFIC_GENERATOR_LOAD) {
	if((ftraffic=fopen(trafficfile, "r"))!=NULL) {
		load_traffic(ftraffic, &aSim->pkgs, NULL, LOAD_DELIEVERED_PKGS);
		fclose(ftraffic);
	}
  }
  
  // setup events
  setup_b2l_traffic_events(aSim);
  setup_bus_meet_bus_events(aSim, &cntTable);
  setup_taxi_meet_bus_events(aSim, &cntTable);
  hashtable_destroy(&cntTable, (void(*)(void*))pair_free_func);

  setup_bus_meet_storage_events(aSim, &traceTable);
  setup_taxi_meet_storage_events(aSim, &traceTable);
  setup_bus_arrive_cell_events(aSim, &traceTable);
  hashtable_destroy(&traceTable, (void(*)(void*))trace_free_func);

  // process events
  totalevents = 0;
  for(i=0;i<EVENT_SLOTS;i++)
	if(aSim->eventSlots[i].nItems)
		totalevents += aSim->eventSlots[i].nItems;
  printf("Start to process %ld events.\n", totalevents);
  i = 0;
  lper = 0;
  while(consume_an_event(aSim)) {
	i++;
	cper = i*100.0/totalevents;
	if(cper-lper>=0.05) {
		printf("Processing events... %.0f%%\r", cper);
		lper = cper;
	}
  }
  printf("Processing events... 100%%\n");

  // dump pkgs 
  if((fpkgdump = fopen(pkgdumpfile, "a"))!=NULL) {
	dump_traffic(fpkgdump, &aSim->pkgs, aSim->trafficCount);
	fclose(fpkgdump);
  }

  // collect resutls
  if((fdlydump = fopen(dlydumpfile, "a"))!=NULL) {
	  i = 0;
	  aItem = aSim->pkgs.head;
	  while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		if(aPkg->endAt != 0) {
			if(fdlydump)
				fprintf(fdlydump, "%ld ", aPkg->endAt-aPkg->startAt);
			i ++;
		} else {
			if(fdlydump)
				fprintf(fdlydump, "-1 ");
		}
		aItem = aItem->next;
	  }
	  fprintf(fdlydump, "\n");
	  fclose(fdlydump);
  }
  /* dump delivery ratio */
  fdlvdump = fopen(dlvdumpfile, "a");
  if(fdlvdump) {
	fprintf(fdlvdump, "%.2lf ", i*1.0/aSim->pkgs.nItems);
	fclose(fdlvdump);
  }

  /* dump network traffic */
  ftrffdump = fopen(trffdumpfile, "a");
  if(ftrffdump) {
	fprintf(ftrffdump, "%.2lf ", (aSim->trafficCount-aSim->pkgs.nItems)*1.0/aSim->pkgs.nItems);
	fclose(ftrffdump);
  }

  // destroy simulator
  simulator_free_func(aSim); 

  return 0;
}


void setup_b2l_traffic_events(struct Simulator *aSim)
{
	struct Item *aItem;
	struct Event *aEvent;
	struct Pkg *aPkg, *newPkg;
	struct Node *srcNode=NULL;	

	if(aSim->pkgs.nItems) {
		aItem = aSim->pkgs.head;
		while(aItem) {
			aPkg = (struct Pkg*)aItem->datap;
			newPkg = pkg_copy_func(aPkg);
			srcNode = lookup_node(&aSim->vnodes, aPkg->src);
			aEvent = (struct Event*)malloc(sizeof(struct Event));
			event_init_func(aEvent, newPkg->startAt, srcNode, newPkg, (int(*)(struct Simulator*, void*,void*))node_recv);
			add_event(aSim, aEvent);
			aItem = aItem->next;
		}
	}
}
