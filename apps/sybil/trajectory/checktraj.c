#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"files.h"
#include"traj.h"
#include"cliquer.h"


struct Node
{
   char name[64];
   int id;
   int nParts;
   int visited;
   struct Duallist edges;
};

struct Edge
{
   struct Node *aNode;
   double similarity;
};


int edge_has_node(struct Node *aNode, struct Edge* anEdge)
{
	return aNode == anEdge->aNode;
}


int node_has_name(char *name, struct Node* aNode)
{
	return !strcmp(name, aNode->name);
}

void node_free_func(struct Node* aNode)
{
	struct Item *anEdgeItem;
	struct Edge *anEdge;
	struct Node *bNode;

	if(aNode) {
		anEdgeItem = aNode->edges.head;
		while(anEdgeItem) {
			anEdge = (struct Edge*)anEdgeItem->datap;
			bNode = anEdge->aNode;
			free(duallist_pick(&aNode->edges, bNode, (int(*)(void*,void*))edge_has_node));
			free(duallist_pick(&bNode->edges, aNode, (int(*)(void*,void*))edge_has_node));
			anEdgeItem = anEdgeItem->next;
		}
		duallist_destroy(&aNode->edges, free);
		free(aNode);
	}
}



int main(int argc, char **argv)
{
	FILE *fsource, *fsuspicious, *ftrustworthy, *fmalicious;

	struct Duallist trajs;
	struct Item *aItem, *bItem, *aNodeItem;
	struct Node *aNode, *bNode;
	struct Edge *newEdge, *anEdge;
	struct Trajectory *aTraj, *bTraj;

	struct Hashtable nodes;
	time_t checkwindow;
	double similarity;

	unsigned long edgeCount, i;
	int K = 2;
	time_t T;

	int numTrustworthy = 0, numMalicious = 0;
	int *(*reorder)(graph_t *, boolean)=reorder_by_default;

	if(argc < 5) {
	      printf("%s is used to check the similarity of any two trajectories.\n", argv[0]);
	      printf("Usage: %s [-r reorder_func(none, reverse, default, unweighted-coloring, weighted-coloring, degree, random)] checkwindow T K (.trj | .lst ...)\n", argv[0]);
	      exit(1);
	}

	while(argv[1][0] =='-') {
	  switch ( argv[1][1]) {
		case 'r':
			if (strcmp(argv[2],"none")==0)
				reorder=NULL;
			else if (strcmp(argv[2],"reverse")==0)
				reorder=reorder_by_reverse;
			else if (strcmp(argv[2],"default")==0)
				reorder=reorder_by_default;
			else if (strcmp(argv[2],"unweighted-coloring")==0)
				reorder=reorder_by_unweighted_greedy_coloring;
			else if (strcmp(argv[2],"weighted-coloring")==0)
				reorder=reorder_by_weighted_greedy_coloring;
			else if (strcmp(argv[2],"degree")==0)
				reorder=reorder_by_degree;
			else if (strcmp(argv[2],"random")==0)
				reorder=reorder_by_random;
			else {
				fprintf(stderr,"Bad reordering type: %s\n",
					argv[2]);
				exit(1);
			}

			argc-=2;
			argv+=2;
			break;
	  }
	}

	checkwindow=atoi(argv[1]);
	T = atoi(argv[2]);
	K = atoi(argv[3]);

	argc-=3;
	argv+=3;

	if((ftrustworthy=fopen("trustworthy.rpt", "w"))== NULL) {
		printf("Cannot open file to write!\n");
		exit(1);
	}
	fprintf(ftrustworthy, "%d\n", FILE_SYBIL_HONEST_TRAJ);
	if((fmalicious=fopen("malicious.rpt", "w"))== NULL) {
		printf("Cannot open file to write!\n");
		exit(1);
	}
	fprintf(fmalicious, "%d\n", FILE_SYBIL_MALICIOUS_TRAJ);
	if((fsuspicious=fopen("suspicious.grh", "w"))== NULL) {
		printf("Cannot open file to write!\n");
		exit(1);
	}

	duallist_init(&trajs);
	while(argc > 1) {
		if((fsource=fopen(argv[1], "r"))!=NULL) {
			load_source_file(fsource, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, &trajs, (void*(*)(FILE*, void*))load_traj_with_duallist, NULL, NULL, NULL,NULL,NULL);
			fclose(fsource);
		}

		argc--;
		argv++;
	}

	edgeCount = 0;
	hashtable_init(&nodes, trajs.nItems, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))node_has_name);
	aItem = trajs.head;
	while(aItem != NULL ) {
		aTraj = (struct Trajectory*)aItem->datap;
		bItem = aItem->next;
		while (bItem != NULL) {
			bTraj = (struct Trajectory*)bItem->datap;
			similarity = similarity_between_two_trajs(aTraj, bTraj, checkwindow, T, K);
			if(similarity != -1) {
				aTraj->trustworth = TRAJ_SUSPICIOUS;
				bTraj->trustworth = TRAJ_SUSPICIOUS;
				aNodeItem = hashtable_find(&nodes, aTraj->vName);
				if(aNodeItem == NULL) {
					aNode = (struct Node*)malloc(sizeof(struct Node));
					strncpy(aNode->name, aTraj->vName, strlen(aTraj->vName)+1);
					hashtable_add(&nodes, aTraj->vName, aNode);
					aNode->id = nodes.count;
					aNode->visited = 0;
					aNode->nParts = aTraj->nParts;
					duallist_init(&aNode->edges);
				} else 
					aNode = (struct Node*)aNodeItem->datap;

				aNodeItem = hashtable_find(&nodes, bTraj->vName);
				if(aNodeItem == NULL) {
					bNode = (struct Node*)malloc(sizeof(struct Node));
					strncpy(bNode->name, bTraj->vName, strlen(bTraj->vName)+1);
					hashtable_add(&nodes, bTraj->vName, bNode);
					bNode->id = nodes.count;
					bNode->visited = 0;
					bNode->nParts = bTraj->nParts;
					duallist_init(&bNode->edges);
				} else 
					bNode = (struct Node*)aNodeItem->datap;
				newEdge = (struct Edge*)malloc(sizeof(struct Edge));
				newEdge->aNode = bNode;
				newEdge->similarity = similarity;
				duallist_add_to_tail(&aNode->edges, newEdge);
				newEdge = (struct Edge*)malloc(sizeof(struct Edge));
				newEdge->aNode = aNode;
				newEdge->similarity = similarity;
				duallist_add_to_tail(&bNode->edges, newEdge);
				edgeCount ++;
			}
			bItem = bItem->next;
		}

		if(aTraj->trustworth == TRAJ_TRUSTWORTHY) {
			numTrustworthy ++;
			fprintf(ftrustworthy, "%s,\n", aTraj->vName);
		}

		aItem = aItem->next;
	}
	duallist_destroy(&trajs, (void(*)(void*))traj_free_func);

	if(nodes.count) {
		int j, first;
		struct Duallist aClique;
		int maxTrajLeng;
		struct Node *leader;
		/* setup graph for cliquer */
		graph_t * aGraph;
		set_t s;
		aGraph = graph_new(nodes.count);

		/* Initialize out clique_options */
		clique_options *opts;
		opts=malloc(sizeof(clique_options));
		opts->time_function=NULL;
		opts->output=stderr;
		opts->reorder_function=reorder;
		opts->reorder_map=NULL;
		opts->user_function=NULL;
		opts->user_data=NULL;
		opts->clique_list=NULL;
		opts->clique_list_length=0;

		fprintf(fsuspicious, "p edge %ld %ld\n", nodes.count, edgeCount);
		for (i = 0; i<nodes.size; i++) {
			aItem = nodes.head[i];
			while(aItem != NULL ) {
				  aNode = (struct Node*)aItem->datap;
				  fprintf(fsuspicious, "n %d 1\n", aNode->id);
				  aItem = aItem->next;
			}
		}
		for (i = 0; i<nodes.size; i++) {
			aItem = nodes.head[i];
			while(aItem != NULL ) {
				  aNode = (struct Node*)aItem->datap;
				  bItem = aNode->edges.head;
				  while(bItem!=NULL) {
					  anEdge = (struct Edge*)bItem->datap;
					  bNode = anEdge->aNode;
					  if(aNode->id < bNode->id) {
						  fprintf(fsuspicious, "e %d %d\n", aNode->id, bNode->id);
						  GRAPH_ADD_EDGE(aGraph, aNode->id-1, bNode->id-1);
					  }
					  bItem = bItem->next;
				  }
				  aItem = aItem->next;
			}
		}

		/* now find the maximum clique once a time in the left graph 
 		   untile there are no more edges */
		while(graph_edge_count(aGraph)) {
			s=clique_unweighted_find_single(aGraph, 0, 0, FALSE, opts);
			first = 1;
			duallist_init(&aClique);
			for (i=0; i<SET_MAX_SIZE(s); i++) {
				if (SET_CONTAINS(s,i)) {
					/* remove all edges connected to i in graph_t */
					for (j=0;j<aGraph->n;j++) {
						if(i!=j && GRAPH_IS_EDGE_FAST(aGraph, i, j)) {
							GRAPH_DEL_EDGE(aGraph, i, j);
						}
					}
					/* remove all edges connected to node i+1 in our internal graph */
					
					for (j = 0; j<nodes.size; j++) {
						aItem = nodes.head[j];
						while(aItem != NULL ) {
							aNode = (struct Node*)aItem->datap;
							if(aNode->id == i+1) {
								duallist_add_to_tail(&aClique, aNode);
								if(first) {
									first = 0;
									maxTrajLeng = aNode->nParts;
									leader = aNode;
								} else {
									if (maxTrajLeng < aNode->nParts) {
										maxTrajLeng = aNode->nParts;
										leader = aNode;
									}
								}
								aNode->visited = 1;

								bItem = aNode->edges.head;
								while(bItem!=NULL) {
									anEdge = (struct Edge*)bItem->datap;
									bNode = anEdge->aNode;
									free(duallist_pick(&aNode->edges, bNode, (int(*)(void*,void*))edge_has_node));
									free(duallist_pick(&bNode->edges, aNode, (int(*)(void*,void*))edge_has_node));
									bItem = bItem->next;
								}
							}
							aItem = aItem->next;
						}
					}
				}
			}
			aItem = aClique.head;
			while(aItem) {
				aNode = (struct Node*)aItem->datap;
				if(aNode == leader) {
					numTrustworthy ++;
					fprintf(ftrustworthy, "%s, clique leader\n",aNode->name);
				} else {
					numMalicious ++;
					fprintf(fmalicious, "%s, clique sacrifice\n",aNode->name);
				}
				aItem = aItem->next;
			}
			set_free(s);
			duallist_destroy(&aClique, NULL);

		}

		/* the standing alone nodes are trustworthy */
		for (i = 0; i<nodes.size; i++) {
			aItem = nodes.head[i];
			while(aItem != NULL ) {
				aNode = (struct Node*)aItem->datap;
				if(aNode->visited == 0) {
					numTrustworthy ++;
					fprintf(ftrustworthy, "%s, standing alone nodes\n",aNode->name);
				}
				aItem = aItem->next;
			}
		}

		graph_free(aGraph);
	}
	hashtable_destroy(&nodes, (void(*)(void*))node_free_func);
	fclose(ftrustworthy);
	fclose(fmalicious);
	fclose(fsuspicious);
	return 0;
}




