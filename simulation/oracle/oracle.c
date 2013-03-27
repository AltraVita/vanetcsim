#include<stdlib.h>
#include<string.h>
#include"oracle.h"
#include"contact.h"

void pairwise_init_func(struct Pairwise *aPairwise, char *name1, char *name2, int k)
{
	if(aPairwise == NULL)
		return;
	strncpy(aPairwise->name1, name1, NAME_LENGTH);
	strncpy(aPairwise->name2, name2, NAME_LENGTH);

	aPairwise->total = 0;
	aPairwise->estimation = 0;
	if(k>0) {
		aPairwise->preStates = (int*)malloc(sizeof(int)*k);
	} else
		aPairwise->preStates = NULL;
	hashtable_init(&aPairwise->transitions, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))transition_has_history);
}


void pairwise_free_func(struct Pairwise *aPairwise)
{
	if(aPairwise == NULL)
		return;
	if(aPairwise->preStates != NULL)
		free(aPairwise->preStates);
	hashtable_destroy(&aPairwise->transitions, (void(*)(void*))transition_free_func);
}



int pairwise_has_names(char *names, struct Pairwise *aPairwise)
{
	char buf[128];
	sprintf(buf, "%s,%s", aPairwise->name1, aPairwise->name2);
	return !strcmp(names, buf);
}



struct Pairwise* lookup_pairwise_in_oracle(struct Oracle *oracle, char *vname1, char*vname2) 
{
	struct Item *aItem;
	struct Pairwise *aPairwise;
	char key[64], *name1, *name2;

	if (0 > strcmp(vname1, vname2)) {
		name1 = vname1;
		name2 = vname2;
	} else {
		name1 = vname2;
		name2 = vname1;
	}
	sprintf(key, "%s,%s", name1, name2);
	aItem = hashtable_find(&oracle->pairwises, key);
	if(aItem != NULL) {
		aPairwise = (struct Pairwise*)aItem->datap;
	} else {
		aPairwise = NULL;
	} 
	return aPairwise;
}


double calculate_estimation(struct Transition *aTrans)
{
	struct Item *aItem;
	struct Prob *aProb;
	double rst = 0;

	if(aTrans == NULL)
		return -1;
	aItem = aTrans->probs.head;
	while(aItem != NULL) {
		aProb = (struct Prob*)aItem->datap;
		rst = rst + aProb->state * aProb->prob;
		aItem = aItem->next;
	}
	return rst;
}


int prob_has_state(int *state, struct Prob *aProb)
{
	return *state == aProb->state;
}


void transition_init_func(struct Transition *aTrans, char *history)
{
	if(aTrans == NULL)
		return;
	strncpy(aTrans->history, history, 64);
	duallist_init(&aTrans->probs);
}

void transition_free_func(struct Transition *aTrans)
{
	if(aTrans==NULL)
		return;
	duallist_destroy(&aTrans->probs, free);
}


int transition_has_history(char *history, struct Transition *aTrans)
{
	if(aTrans == NULL)
		return 0;
	return !strcmp(history, aTrans->history);
}

void oracle_init_func(struct Oracle *oracle, int type, struct Simulator *aSim, time_t trainingStartAt, time_t trainingEndAt)
{
	if(oracle == NULL)
		return;
	oracle->type = type;
	oracle->aSim = aSim;
	oracle->trainingStartAt = trainingStartAt;
	oracle->trainingEndAt = trainingEndAt;
	oracle->size = 0;
	if(type == TYPE_ORACLE_MARKOV)
		oracle->setup_oracle = setup_oracle_mkv;
	else if (type == TYPE_ORACLE_AVGDLY)
		oracle->setup_oracle = setup_oracle_avg;
	else if (type == TYPE_ORACLE_AVGPRB)
		oracle->setup_oracle = setup_oracle_prob;
	else if (type == TYPE_ORACLE_BUBBLE)
		oracle->setup_oracle = setup_oracle_bubble;
	else if (type == TYPE_ORACLE_SIMBET)
		oracle->setup_oracle = setup_oracle_simbet;
	else if (type == TYPE_ORACLE_SOCIAL)
		oracle->setup_oracle = setup_oracle_social;
	else
		oracle->setup_oracle = NULL;

	hashtable_init(&oracle->pairwises, 10000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pairwise_has_names);
}

void oracle_free_func(struct Oracle *oracle)
{
	if(oracle) {
		hashtable_destroy(&oracle->pairwises, (void(*)(void*))pairwise_free_func);
		free(oracle);
	}
}


void setup_oracle_mkv(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem, *bCntItem, *aTransItem, *aProbItem;
	struct Pair *aCntPair;
	int aIct;
	int k, valid, value, state;
	struct Pairwise *aPairwise;
	struct Transition *aTrans;
	struct Prob *aProb;
	char key[64], history[1024], strp[32]; 

	if(oracle == NULL || oracle->order == 0)
		return;
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the pairwise table */
			aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2 );
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, aCntPair->vName1, aCntPair->vName2, oracle->order);
				sprintf(key, "%s,%s", aCntPair->vName1, aCntPair->vName2);
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
			/* setup transitions */
			aPairwise->total = 0;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next) {
				if(((struct Contact*)aCntItem->next->datap)->startAt > oracle->trainingStartAt 
				 &&((struct Contact*)aCntItem->next->datap)->startAt < oracle->trainingEndAt) {
					aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/oracle->tGran;
					aPairwise->total += aIct * aIct;
					valid = 1;
					bCntItem = aCntItem;
					memset(history, 0, 1024);
					for(k=0;k<oracle->order;k++) {
						if(bCntItem && bCntItem->next) {
							aIct = (((struct Contact*)bCntItem->next->datap)->startAt - ((struct Contact*)bCntItem->datap)->endAt)/oracle->tGran;	
							if(aIct < oracle->T/oracle->tGran)
								value = aIct;
							else
								value = oracle->T/oracle->tGran;
							memset(strp, 0, 32);
							sprintf(strp, "%d,", value);
							strcat(history, strp);
						} else {
							valid = 0;
							break;
						}	
						bCntItem = bCntItem->next;
					}
					if(valid && bCntItem && bCntItem->next) {
						aIct = (((struct Contact*)bCntItem->next->datap)->startAt - ((struct Contact*)bCntItem->datap)->endAt)/oracle->tGran;	
						if(aIct < oracle->T/oracle->tGran)
							value = aIct;
						else
							value = oracle->T/oracle->tGran;
						state = value;
					} else {
						valid = 0;
					}
					if(valid) {
						aTransItem = hashtable_find(&aPairwise->transitions, history);
						if(aTransItem == NULL) {
							aTrans = (struct Transition*)malloc(sizeof(struct Transition));
							transition_init_func(aTrans, history);
							aTrans->count = 1;
							hashtable_add(&aPairwise->transitions, history, aTrans);
						} else {
							aTrans = (struct Transition*)aTransItem->datap;
							aTrans->count ++;
						}
						aProbItem = duallist_find(&aTrans->probs, &state, (int(*)(void*,void*))prob_has_state);
						if(aProbItem == NULL) {
							aProb = (struct Prob*) malloc(sizeof(struct Prob));
							aProb->state = state;
							aProb->prob = 1;
							duallist_add_to_tail(&aTrans->probs, aProb);
							/* statistic on how much memory used for this oracle */
							oracle->size ++;
						} else {
							aProb = (struct Prob*)aProbItem->datap;
							aProb->prob += 1;
						}
					}
				} 
				aCntItem = aCntItem->next;
			}

			if(aPairwise->total)
				aPairwise->defaultEstimate = aPairwise->total/(2*(oracle->trainingEndAt-oracle->trainingStartAt)/oracle->tGran);
			else	
				aPairwise->defaultEstimate = -1;

			/* setup previous states and current estimation */
			for(k=0;k<oracle->order;k++)
				aPairwise->preStates[k] = -1;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next && ((struct Contact*)aCntItem->next->datap)->startAt<oracle->aSim->exprStartAt) {
				aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/oracle->tGran;
				if(aIct < oracle->T/oracle->tGran)
					value = aIct;
				else
					value = oracle->T/oracle->tGran;
				update_previous_states(aPairwise->preStates, oracle->order, value);
				aCntItem = aCntItem->next;
			}
			aPairwise->estimation = estimate_next_delay(aPairwise, aPairwise->preStates, oracle->order, oracle->useDefault);
			aItem = aItem->next;
		}
	}
}


void update_previous_states(int *preStates, int k, int value)
{
	int i;
	if(k>0) {
		for(i=0;i<k-1;i++)
			preStates[i] = preStates[i+1];
		preStates[k-1] = value;
	}
}


double estimate_next_delay(struct Pairwise *aPairwise, int *preStates, int k, int useDefault)
{
	int i;
	char history[1024], strp[32];
	struct Transition *aTrans;
	struct Prob *aProb;
	struct Item *aTransItem, *aProbItem;
	double rt = 0;

	memset(history, 0, 1024);
	for(i=0;i<k;i++) {
		if(preStates[i] == -1) {
			return rt = -1;
		}
		memset(strp, 0, 32);
		sprintf(strp, "%d,", preStates[i]);
		strcat(history, strp);
	}
	
	aTransItem = hashtable_find(&aPairwise->transitions, history);
	if(aTransItem == NULL) {
		if(useDefault)
			rt = aPairwise->defaultEstimate;
		else
			rt = -1;
	} else {
		aTrans = (struct Transition*)aTransItem->datap;
		aProbItem = aTrans->probs.head;
		rt = 0;
		while(aProbItem != NULL) {
			aProb = (struct Prob*)aProbItem->datap;
			rt +=  aProb->state*aProb->prob*1.0/aTrans->count;
			aProbItem = aProbItem->next;
		}
	}
	return rt;
}


void setup_oracle_avg(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem;
	struct Pair *aCntPair; 
	struct Pairwise *aPairwise;
	int aIct;
	char key[64];

	if(oracle == NULL)
		return;
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the pairwise table */
			aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2 );
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, aCntPair->vName1, aCntPair->vName2, oracle->order);
				sprintf(key, "%s,%s", aCntPair->vName1, aCntPair->vName2);
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
			/* setup average estimation */
			aPairwise->total = 0;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next) {
				aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/3600;	
				if(((struct Contact*)aCntItem->next->datap)->startAt > oracle->trainingStartAt 
				&& ((struct Contact*)aCntItem->next->datap)->startAt < oracle->trainingEndAt) {
					aPairwise->total += aIct * aIct ;
				}
				aCntItem = aCntItem->next;
			}
			if(aPairwise->total)
				aPairwise->estimation = aPairwise->total/(2*(oracle->trainingEndAt-oracle->trainingStartAt)/3600);
			else	
				aPairwise->estimation = -1;
			aItem = aItem->next;
		}
	}
}


void setup_oracle_prob(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem;
	struct Pair *aCntPair; 
	struct Pairwise *aPairwise;
	int aIct;
	char key[64];

	if(oracle == NULL)
		return;
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			/* set up the coresponding pair in the pairwise table */
			aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2 );
			if(aPairwise == NULL) {
				aPairwise = (struct Pairwise*)malloc(sizeof(struct Pairwise));
				pairwise_init_func(aPairwise, aCntPair->vName1, aCntPair->vName2, oracle->order);
				sprintf(key, "%s,%s", aCntPair->vName1, aCntPair->vName2);
				hashtable_add(&oracle->pairwises, key, aPairwise);
			}
			/* setup average link-break probability estimation */
			aPairwise->total = 0;
			aCntItem = aCntPair->contents.head;
			while(aCntItem && aCntItem->next) {
				aIct = (((struct Contact*)aCntItem->next->datap)->startAt - ((struct Contact*)aCntItem->datap)->endAt)/3600;	
				if(((struct Contact*)aCntItem->next->datap)->startAt > oracle->trainingStartAt 
				&& ((struct Contact*)aCntItem->next->datap)->startAt < oracle->trainingEndAt) {
					aPairwise->total += aIct ;
				}
				aCntItem = aCntItem->next;
			}
			if(aPairwise->total)
				aPairwise->estimation = aPairwise->total/((oracle->trainingEndAt-oracle->trainingStartAt)/3600);
			else	
				aPairwise->estimation = -1;

			aItem = aItem->next;
		}
	}
}

void setup_oracle_bubble(struct Oracle *oracle)
{
 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	setup_neighborhood(oracle);
}

void setup_oracle_simbet(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem;
	struct Node *aNode;

 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	setup_neighborhood(oracle);

	/* calculate egocentric betweenness */
	for(i=0;i<oracle->aSim->vnodes.size;i++) {
		aItem = oracle->aSim->vnodes.head[i];
		while(aItem!=NULL) {
			aNode = (struct Node*)aItem->datap;
			if(oracle->alfar) {
				aNode->betweenness = calculate_betweenness(aNode);
			}
			aItem = aItem->next;
		}
	}
}


void setup_oracle_social(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem, *bCntItem;
	struct Pair *aCntPair; 
	struct Contact *aCnt;
	struct Node *aNode, *bNode;
	int value;
	struct Pairwise *aPairwise;
	int ict, ictEstimation;
	int *preStates,k;
	struct NeighborNode *aNeighborNode, *bNeighborNode;

	/* setup Markov oracle during the training phase */
	setup_oracle_mkv(oracle);

	preStates = (int*)malloc(sizeof(int)*oracle->order);

 	/* the main purpose of this is to establishing neighbor 
 	* relationship between nodes */
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			aNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName1);
			bNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName2);
			if(aNode && bNode) {
				aCntItem = aCntPair->contents.head;
				while(aCntItem != NULL) {
					aCnt = (struct Contact*)aCntItem->datap;
					if(aCnt->endAt > oracle->trainingStartAt && aCnt->endAt < oracle->trainingEndAt) {
						ictEstimation = -1;
						aPairwise = lookup_pairwise_in_oracle(oracle, aCntPair->vName1, aCntPair->vName2);
						if(aPairwise) {
							/* setup previous states and current estimation */
							for(k=0;k<oracle->order;k++)
								preStates[k] = -1;
							k = 0;
							bCntItem = aCntItem;
							while(bCntItem!=aCntPair->contents.head && k<oracle->order) {
								ict = (((struct Contact*)bCntItem->datap)->startAt - ((struct Contact*)bCntItem->prev->datap)->endAt)/oracle->tGran;
								if(ict < oracle->T/oracle->tGran)
									value = ict;
								else
									value = oracle->T/oracle->tGran;
								preStates[oracle->order-k-1]=value;
								k++;
								bCntItem = bCntItem->prev;
							}
							ictEstimation = estimate_next_delay(aPairwise, preStates, oracle->order, oracle->useDefault);
					
						}	
						if(ictEstimation !=-1)
							ictEstimation = (ictEstimation+1) * oracle->tGran;
					
						aNeighborNode = node_met_a_node(aNode, bNode, oracle->neighborThreshold);
						bNeighborNode = node_met_a_node(bNode, aNode, oracle->neighborThreshold);
						assess_neighbor(aNeighborNode, oracle, aCnt->startAt, ictEstimation);
						assess_neighbor(bNeighborNode, oracle, aCnt->startAt, ictEstimation);
					}
					aCntItem = aCntItem->next;
				}
			}
			aItem = aItem->next;
		}
	}
	free(preStates);
}



void setup_neighborhood(struct Oracle *oracle)
{
	unsigned long i;
	struct Item *aItem, *aCntItem;
	struct Pair *aCntPair; 
	struct Contact *aCnt;
	struct Node *aNode, *bNode;


	if(oracle == NULL)
		return;
	for(i=0;i<oracle->aSim->cntTable.size;i++) {
		aItem = oracle->aSim->cntTable.head[i];
		while(aItem!=NULL) {
			aCntPair = (struct Pair*)aItem->datap;
			aNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName1);
			bNode = lookup_node(&oracle->aSim->vnodes, aCntPair->vName2);
			if(aNode && bNode) {
				aCntItem = aCntPair->contents.head;
				while(aCntItem != NULL) {
					aCnt = (struct Contact*)aCntItem->datap;
					if(aCnt->endAt > oracle->trainingStartAt && aCnt->endAt < oracle->trainingEndAt) {
						node_met_a_node(aNode, bNode, oracle->neighborThreshold);
						node_met_a_node(bNode, aNode, oracle->neighborThreshold);
					}
					aCntItem = aCntItem->next;
				}
			}
			aItem = aItem->next;
		}
	}
}

double calculate_betweenness(struct Node *aNode)
{
	double A[aNode->neighbors.count][aNode->neighbors.count];
	double A2[aNode->neighbors.count][aNode->neighbors.count];
	int i,j,l;
	unsigned long k;
	struct Item *aItem, *bItem, *cItem;
	struct NeighborNode *aNeighborNode, *bNeighborNode;
	double rt;

	i=0;
	for(k=0;k<aNode->neighbors.size;k++) {
		aItem = aNode->neighbors.head[k];
		while(aItem) {
			i++;
			j = i;
			aNeighborNode = (struct NeighborNode*)aItem->datap;
			A[0][i] = aNeighborNode->strength;
			for(l=0;l<((int)aNode->neighbors.count)-(i+1);l++) {
				j++;
				bItem = hashtable_next_item(&aNode->neighbors, aItem);
				bNeighborNode = (struct NeighborNode*)bItem->datap;
				cItem = hashtable_find(&aNeighborNode->node->neighbors, bNeighborNode->node->name);
				if(cItem)
					A[i][j] = ((struct NeighborNode*)cItem->datap)->strength;
				else
					A[i][j] = 0;
			}
			aItem = aItem->next;
		}
	}

	for(i=0;i<aNode->neighbors.count;i++)
		for(j=0;j<i+1;j++) {
			if(i==j)
				A[i][j] = 0;
			else
				A[i][j] = A[j][i];
		}
	for(i=0;i<aNode->neighbors.count;i++)
		for(j=i+1;j<aNode->neighbors.count;j++) {
			A2[i][j] = 0;
			for(k=0;k<aNode->neighbors.count;k++)
				A2[i][j] += A[i][k]*A[k][j];
		}
	rt = 0;
	for(i=0;i<aNode->neighbors.count;i++)
		for(j=i+1;j<aNode->neighbors.count;j++) 
			if(A[i][j]==0 && A2[i][j])
				rt += 1/A2[i][j];

	return rt;
}


double calculate_similarity(struct Node *aNode, struct Node *dstNode)
{
	struct Duallist commons;
	struct Item *aItem, *bItem, *cItem;
	unsigned long i;
	struct NeighborNode *aNeighborNode;
	double rt;

	duallist_init(&commons);
	/* direct common neighbors */
	aItem = hashtable_find(&aNode->neighbors, dstNode->name);
	if(aItem) {
		for(i=0;i<dstNode->neighbors.size;i++) {
			bItem = dstNode->neighbors.head[i];
			while(bItem) {
				aNeighborNode = (struct NeighborNode*)bItem->datap;
				cItem = hashtable_find(&aNode->neighbors, aNeighborNode->node->name);
				if(cItem) {
					duallist_add_to_tail(&commons, aNeighborNode);
				}
				bItem = bItem->next;
			}
		}
	}  

	/* indirect common neighbors */
	for(i=0;i<aNode->neighbors.size;i++) {
		aItem = aNode->neighbors.head[i];
		while(aItem) {
			aNeighborNode = (struct NeighborNode*)aItem->datap;
			if(aNeighborNode->node!=dstNode) {
				bItem = hashtable_find(&aNeighborNode->node->neighbors, dstNode->name);
				if(bItem) {
					cItem = duallist_find(&commons, aNeighborNode->node->name, (int(*)(void*,void*))neighborNode_has_name);
					if(!cItem)
						duallist_add_to_tail(&commons, aNeighborNode);
				}
			}
			aItem = aItem->next;
		}
	}
	rt = commons.nItems;
	duallist_destroy(&commons, NULL);
	return rt;	
}

void assess_neighbor(struct NeighborNode *aNeighborNode, struct Oracle *oracle, time_t current_time, int ictEstimation)
{
	struct Item *aItem;
	time_t error;
	struct Estimation *aEstimation;
	int count;

	if(aNeighborNode) {
		/* judge the last estimation and update the pairwise relation strength*/
		aItem = aNeighborNode->lastEstimations.head;
		if(aItem) {
			aEstimation = (struct Estimation*)aItem->datap;
			if(aEstimation->estimatedTime != -1) {
				oracle->numAllEstimations ++;
				error = (aEstimation->estimatedTime + aEstimation->timestamp) > current_time ? (aEstimation->timestamp + aEstimation->estimatedTime - current_time) : (current_time - aEstimation->timestamp - aEstimation->estimatedTime);
			} else
				error = -1;
			if(oracle->deltaT == -1 || (oracle->deltaT!=-1 && error!= -1 && error < oracle->deltaT)) {
				aEstimation->bingo = 1; 
				if(oracle->deltaT!=-1 && error!= -1 ) 
					oracle->numBingoEstimations ++;
			} else
				aEstimation->bingo = 0;
			count = 0;
			aItem = aNeighborNode->lastEstimations.head;
			while(aItem) {
				if(((struct Estimation*)aItem->datap)->bingo)
					count ++;
				aItem = aItem->next;
			}
			aNeighborNode->strength = count*1.0/aNeighborNode->lastEstimations.nItems;
		} else {
			aNeighborNode->strength = 0;
		}

		/* add new estimation */
		aEstimation = (struct Estimation*)malloc(sizeof(struct Estimation));
		aEstimation->timestamp = current_time;
		aEstimation->estimatedTime = ictEstimation;
		duallist_add_to_head(&aNeighborNode->lastEstimations, aEstimation);
	}
}

