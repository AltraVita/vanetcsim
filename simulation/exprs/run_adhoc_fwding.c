#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include"common.h"
#include"simulator.h"
#include"oracle.h"
#include"node.h"
#include"pkg.h"
#include"cntEvent.h"
#include"traffic.h"
#include"contact.h"
#include"files.h"

void shrink_cntTable(struct Hashtable *cntTable, struct Hashtable *nodes);
void setup_adhoc_fwding_traffic_events(struct Simulator *aSim);

int main( int   argc,
          char *argv[] )
{
  struct Simulator *aSim;
  time_t trainingStart=0, trainingEnd = 0, starttime = 0, endtime = 0;
  int oracle_type = TYPE_ORACLE_MARKOV, fwdMethod = NO_REPLICA_FWD; 

  int poissonMean = 10, numPkgs = 100, bufSize = -1, pkgSize = 1, pkgTTL = -1, randseed = 1111;
  int selectPolicy = SELECT_RANDOM;
  time_t trafficStartAt = 0;

  int magicNumber;
  time_t startAt = 0, endAt = 0;

  time_t T = 86400, tGran = 3600;
  int mkvOrder = 1;
  int useDefault = 1;

  int neighborThreshold = 1;
  double alfar = 0.5, beta = 0.5;

  double lastT=24;
  double deltaT=-1;
  int onlySocial = 0;

  struct Item *aItem;
  struct Pkg *aPkg;
  struct Node *aNode;

  char buf[1024], *strp, *strp1;
  time_t dumpInterval = 0, dumpClock;
  char *pkgdumpfile = NULL, *dlvdumpfile = NULL, *trffdumpfile = NULL, *memdumpfile = NULL;
  char *cntdump=NULL, *neighbordump=NULL, *pairlistfile=NULL;
  FILE *fsource, *fpkgdump, *fdlvdump, *ftrffdump, *fmemdump, *fcntdump, *fneighbordump;

  unsigned long i, j;

  if(argc < 2) {
	printf("Usage: %s [-time trainingStartAt trainingEndAt exprStartAt exprEndAt] [-oracle type{epidemic, markov, avgdly, avgprb, bubble, simbet, social}] [-forward style{no_replica, better, everbest}] [-genarator startAt PoissonMean(sec) numPkgs selectPolicy(friends, strangers, random)] [-node buffersize] [-packet size TTL] [-r randseed] [-v1 mkvOrder T meeting_temporal_gran(sec) useDefault] [-v2 neighbor_threshold(times)] [-v3 alfar beta([0,1])] [-v4 lastT deltaT(hours) onlySocial] [-w0 dump_interval(hours)] [-w1 pkgdump] [-w2 deliverydump] [-w3 trafficdump] [-w4 memdump] [-w5 cnt_dump] [-w6 neighbor_dump] [-l pair_list_file] [.cont ...]\n", argv[0]);
	exit(1);
  }
  while(argv[1]!=NULL && argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		trainingStart = strtot(argv[2]);
		trainingEnd = strtot(argv[3]);
		starttime = strtot(argv[4]);
		endtime = strtot(argv[5]);
		argc-=5;
		argv+=5;
		break;

	case 'o':
		if(strcmp("markov",argv[2])==0) {
			oracle_type = TYPE_ORACLE_MARKOV;
		} else if (strcmp("avgdly", argv[2])==0) {
			oracle_type = TYPE_ORACLE_AVGDLY;
		} else if (strcmp("avgprb", argv[2])==0) {
			oracle_type = TYPE_ORACLE_AVGPRB;
		} else if (strcmp("epidemic", argv[2])==0) {
			oracle_type = TYPE_ORACLE_EPIDEMIC;
		} else if (strcmp("bubble", argv[2])==0) {
			oracle_type = TYPE_ORACLE_BUBBLE;
		} else if (strcmp("simbet", argv[2])==0) {
			oracle_type = TYPE_ORACLE_SIMBET;
		} else if (strcmp("social", argv[2])==0) {
			oracle_type = TYPE_ORACLE_SOCIAL;
		}
		argc-=2;
		argv+=2;
		break;

	case 'f':
		if(strcmp("no_replica",argv[2])==0) {
			fwdMethod = NO_REPLICA_FWD;
		} else if (strcmp("better", argv[2])==0) {
			fwdMethod = BETTER_ESTIMATE_FWD;
		} else if (strcmp("everbest", argv[2])==0) {
			fwdMethod = EVERBEST_ESTIMATE_FWD;
		}
		argc-=2;
		argv+=2;
		break;

	case 'g':
		trafficStartAt = strtot(argv[2]);
		poissonMean = atoi(argv[3]);
		numPkgs = atoi(argv[4]);
		if(strcmp("friends",argv[5])==0) 
			selectPolicy = SELECT_FRIENDS;
		else if(strcmp("strangers",argv[5])==0) 
			selectPolicy = SELECT_STRANGERS;
		else if(strcmp("random",argv[5])==0) 
			selectPolicy = SELECT_RANDOM;
		argc-=5;
		argv+=5;
		break;

	case 'n':
		bufSize = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'p':
		pkgSize = atoi(argv[2]);
		pkgTTL = atoi(argv[3]);
		argc-=3;
		argv+=3;
		break;

	case 'r':
		randseed = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'v':
		if(argv[1][2] == '1') {
			mkvOrder = atoi(argv[2]);
			T = atoi(argv[3]);
			tGran = atoi(argv[4]);
			useDefault = atoi(argv[5]);
			argc-=5;
			argv+=5;

		} else if(argv[1][2] == '2') {
			neighborThreshold = atoi(argv[2]);
			argc-=2;
			argv+=2;

		} else if(argv[1][2] == '3') {
			alfar = atof(argv[2]);
			beta = atof(argv[3]);
			argc-=3;
			argv+=3;
		} else if(argv[1][2] == '4') {
			lastT = atoi(argv[2]);
			deltaT = atof(argv[3]);
			onlySocial = atoi(argv[4]);
			argc-=4;
			argv+=4;
		}
		break;

	case 'l':
		pairlistfile = argv[2];
		argc-=2;
		argv+=2;
		break;

	case 'w':
		if(argv[1][2] == '0')
			dumpInterval = atoi(argv[2])*3600;
		else if(argv[1][2] == '1')
			pkgdumpfile = argv[2];
		else if(argv[1][2] == '2')
			dlvdumpfile = argv[2];
		else if(argv[1][2] == '3')
			trffdumpfile = argv[2];
		else if(argv[1][2] == '4')
			memdumpfile = argv[2];
		else if(argv[1][2] == '5')
			cntdump = argv[2];
		else if(argv[1][2] == '6')
			neighbordump = argv[2];

		argc-=2;
		argv+=2;
		break;
	default:
		printf("Usage: %s [-time trainingStartAt trainingEndAt exprStartAt exprEndAt] [-oracle type{epidemic, markov, avgdly, avgprb, bubble, simbet, social}] [-forward style{no_replica, better, everbest}] [-genarator startAt PoissonMean(sec) numPkgs selectPolicy(friends, strangers, random)] [-node buffersize] [-packet size TTL] [-r randseed] [-v1 mkvOrder T meeting_temporal_gran(sec) useDefault] [-v2 neighbor_threshold(times)] [-v3 alfar beta([0,1])] [-v4 lastT deltaT(hours) onlySocial] [-w0 dump_interval(hours)] [-w1 pkgdump] [-w2 deliverydump] [-w3 trafficdump] [-w4 memdump] [-w5 cnt_dump] [-w6 neighbor_dump] [-l pair_list_file] [.cont ...]\n", argv[0]);
	}
  }

  srand(randseed);

  aSim = (struct Simulator*)malloc(sizeof(struct Simulator));
  simulator_init_func(aSim, starttime, endtime, fwdMethod, bufSize, pkgSize, pkgTTL, 0);

  /* setting up contacts */
  while(argc>1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fscanf(fsource, "%d\n", &magicNumber);
		printf("Loading %s file ...\n", argv[1]);
		if(magicNumber == FILE_CONTACT) {
			startAt = 0;
			endAt = 0;
			load_contacts_with_hashtable(fsource, NULL, &aSim->cntTable, PAIRWISE_TABLE, &startAt, &endAt);
		}
		fclose(fsource);
	}
	argc--;
	argv++;
  }
  /* setting up nodes */
  if (pairlistfile && (fsource=fopen(pairlistfile, "r"))!=NULL) {
	  while(fgets(buf, 1024, fsource)) {
		  strp = strtok(buf, ",");
		  strp1 = strtok(NULL, ",");
		  aNode=lookup_node(&aSim->vnodes, strp);
		  if(aNode == NULL) {
			  aNode = (struct Node*)malloc(sizeof(struct Node));
			  node_init_func(aNode, strp, NULL, aSim->bufSize);
			  hashtable_add(&aSim->vnodes, strp, aNode);
		  }
		  aNode=lookup_node(&aSim->vnodes, strp1);
		  if(aNode == NULL) {
			  aNode = (struct Node*)malloc(sizeof(struct Node));
			  node_init_func(aNode, strp1, NULL, aSim->bufSize);
			  hashtable_add(&aSim->vnodes, strp1, aNode);
		  }
	  }
	  fclose(fsource);
	  /* remove extra contacts between other nodes not included in vnodes */
	  shrink_cntTable(&aSim->cntTable, &aSim->vnodes);
  } else
	  setup_vehicular_nodes_by_pairs(&aSim->cntTable, &aSim->vnodes, aSim->bufSize);
  printf("There %ld vehicular nodes.\n", aSim->vnodes.count);

  if(cntdump && (fcntdump=fopen(cntdump, "w"))!=NULL) {
	dump_contacts(&aSim->cntTable, fsource);
	fclose(fsource);
  }
  /* setting up oracle */
  aSim->oracle = (struct Oracle*)malloc(sizeof(struct Oracle));
  oracle_init_func(aSim->oracle, oracle_type, aSim, trainingStart, trainingEnd);
  if(oracle_type == TYPE_ORACLE_MARKOV) {
	aSim->oracle->order = mkvOrder;
	aSim->oracle->T = T;
	aSim->oracle->tGran = tGran;
	aSim->oracle->useDefault = useDefault;
	aSim->oracle->AB = 0;
	aSim->oracle->A_B = 0;
	aSim->oracle->A_B_ = 0;
	aSim->oracle->AB_ = 0;
  } else if (oracle_type == TYPE_ORACLE_SIMBET) {
	aSim->oracle->alfar = alfar;
	aSim->oracle->beta = beta;
  } else if (oracle_type == TYPE_ORACLE_SOCIAL) {
	aSim->oracle->order = mkvOrder;
	aSim->oracle->T = T;
	aSim->oracle->tGran = tGran;
	aSim->oracle->useDefault = useDefault;
	aSim->oracle->lastT = 3600*lastT;
	aSim->oracle->deltaT = 3600*deltaT;
	aSim->oracle->onlySocial = onlySocial;
	aSim->oracle->numAllEstimations = 0;
	aSim->oracle->numBingoEstimations = 0;
  }
  aSim->oracle->neighborThreshold = neighborThreshold;

  if(aSim->oracle->setup_oracle)
	aSim->oracle->setup_oracle(aSim->oracle);

  // setup traffic generator
  if(trafficStartAt == 0)
	trafficStartAt = aSim->exprStartAt;
  aSim->trafficGenerator = (struct TrafficGenerator*)malloc(sizeof(struct TrafficGenerator));
  trafficGenerator_init_func(aSim->trafficGenerator, trafficStartAt, numPkgs, poissonMean, selectPolicy);
  generate_v2v_poisson_traffic(aSim);
  
  // setup events
  setup_adhoc_fwding_traffic_events(aSim);
  setup_cnt_events(aSim, &aSim->cntTable); 

  /* dump pkgs*/ 
  fpkgdump = fopen(pkgdumpfile, "w");
  /* dump delivery ratio */
  fdlvdump = fopen(dlvdumpfile, "w");
  /* dump network traffic */
  ftrffdump = fopen(trffdumpfile, "w");
  /* dump the number of neighbors*/ 
  fneighbordump = fopen(neighbordump, "w");

  dumpClock = aSim->exprStartAt+dumpInterval;
  // process events
  while(consume_an_event(aSim)) {
	if(dumpInterval && aSim->clock >= dumpClock) {
		if(fpkgdump) {
			i = 0, j = 0;
			aItem = aSim->pkgs.head;
			while(aItem != NULL) {
			      aPkg = (struct Pkg*)aItem->datap;
			      if(aPkg->startAt < aSim->clock ) {
				      j ++;
			      }
			      if(aPkg->endAt != 0) {
				      fprintf(fpkgdump, "%ld ", aPkg->endAt-aPkg->startAt);
				      i ++;
			      } else {
				      fprintf(fpkgdump, "-1 ");
			      }
			      aItem = aItem->next;
			}
			fprintf(fpkgdump, "\n");
		}
		if(fdlvdump) {
		        fprintf(fdlvdump, "%.2lf ", i*1.0/aSim->trafficGenerator->numPkgs);
		}

		if(ftrffdump) {
		        fprintf(ftrffdump, "%.2lf ", j==0?0:(aSim->trafficCount-j)*1.0/j);
		}
		
		dumpClock += dumpInterval;
	}
  }

  if(fpkgdump) {
	  i = 0;
	  aItem = aSim->pkgs.head;
	  while(aItem != NULL) {
		aPkg = (struct Pkg*)aItem->datap;
		if(aPkg->endAt != 0) {
			fprintf(fpkgdump, "%ld ", aPkg->endAt-aPkg->startAt);
			i ++;
		} else {
			fprintf(fpkgdump, "-1 ");
		}
		aItem = aItem->next;
	  }
	  fprintf(fpkgdump, "\n");
  	  fclose(fpkgdump);
  }
  if(fdlvdump) {
	fprintf(fdlvdump, "%.2lf ", i*1.0/aSim->trafficGenerator->numPkgs);
  	fclose(fdlvdump);
  }

  if(ftrffdump) {
	fprintf(ftrffdump, "%.2lf ", (aSim->trafficCount-aSim->trafficGenerator->numPkgs)*1.0/aSim->trafficGenerator->numPkgs);
  	fclose(ftrffdump);
  }

  if(fneighbordump) {
	for(i=0;i<aSim->vnodes.size;i++) {
		aItem = aSim->vnodes.head[i];
		while(aItem) {
			fprintf(fneighbordump, "%ld ", ((struct Node*)aItem->datap)->neighbors.count);
			aItem = aItem->next;
		}
	}
  	fclose(fneighbordump);
  }
  /* dump oracle size per pair */ 
  fmemdump = fopen(memdumpfile, "a");
  if(fmemdump) {
	fprintf(fmemdump, "%.2lf ", (aSim->oracle->size)*1.0/aSim->oracle->pairwises.count);
	fclose(fmemdump);
  }
  if (oracle_type == TYPE_ORACLE_MARKOV || oracle_type == TYPE_ORACLE_AVGDLY || oracle_type == TYPE_ORACLE_AVGPRB) 
	printf("AB:%ld, AB_:%ld, A_B:%ld, A_B_:%ld\n", aSim->oracle->AB, aSim->oracle->AB_, aSim->oracle->A_B, aSim->oracle->A_B_);
  if (oracle_type == TYPE_ORACLE_SOCIAL) 
	printf("Markov estimation accuracy:%.2f%%\n", aSim->oracle->numAllEstimations>0? aSim->oracle->numBingoEstimations*100.0/aSim->oracle->numAllEstimations: 0);
  // destroy simulator
  simulator_free_func(aSim);

  return 0;
}

void shrink_cntTable(struct Hashtable *cntTable, struct Hashtable *nodes)
{
  unsigned long i;
  struct Item *aItem, *temp;
  struct Pair *aPair;
  struct Node *aNode, *bNode;
  char key[128];

  for(i=0;i<cntTable->size;i++) {
	aItem = cntTable->head[i];
	while(aItem!=NULL) {
		aPair = (struct Pair*)aItem->datap;
		aNode = lookup_node(nodes, aPair->vName1);
		bNode = lookup_node(nodes, aPair->vName2);
		if(aNode==NULL || bNode==NULL) {
			temp = aItem->next;
			sprintf(key, "%s,%s", aPair->vName1, aPair->vName2);
			pair_free_func((struct Pair*)hashtable_pick(cntTable, key));
			aItem = temp;
			
		} else
			aItem = aItem->next;
	}
  }
	
}

void setup_adhoc_fwding_traffic_events(struct Simulator *aSim)
{
	struct Item *aItem;
	struct Event *aEvent;
	struct Pkg *aPkg, *newPkg;
	struct Node *srcNode=NULL;	
	struct Pairwise *aPairwise;

	if(aSim->pkgs.nItems) {
		aItem = aSim->pkgs.head;
		while(aItem) {
			aPkg = (struct Pkg*)aItem->datap;
			newPkg = pkg_copy_func(aPkg);
			newPkg->value = -1;
			srcNode = lookup_node(&aSim->vnodes, aPkg->src);
			if(aSim->oracle->type== TYPE_ORACLE_MARKOV || aSim->oracle->type== TYPE_ORACLE_AVGDLY || aSim->oracle->type== TYPE_ORACLE_AVGPRB) {
				aPairwise = lookup_pairwise_in_oracle(aSim->oracle, aPkg->src, aPkg->dst);
				if(aPairwise)
					newPkg->value = aPairwise->estimation;
			}
			aEvent = (struct Event*)malloc(sizeof(struct Event));
			event_init_func(aEvent, newPkg->startAt, srcNode, newPkg, (int(*)(struct Simulator*, void*,void*))node_recv);
			add_event(aSim, aEvent);
			aItem = aItem->next;
		}
	}
}
