#ifndef ORACLE_H
#define ORACLE_H

#include"common.h"
#include"node.h"
#include"simulator.h"

#define TYPE_ORACLE_MARKOV 0
#define TYPE_ORACLE_AVGDLY 1
#define TYPE_ORACLE_AVGPRB 2
#define TYPE_ORACLE_EPIDEMIC 3
#define TYPE_ORACLE_BUBBLE 4 
#define TYPE_ORACLE_SIMBET 5
#define TYPE_ORACLE_SOCIAL 6

/* Markov ICT predection */
struct Prob
{
  int state;
  unsigned prob;
};
int prob_has_state(int *state, struct Prob *aProb);

struct Transition
{
  char history[64];
  struct Duallist probs;
  unsigned count;
};
void transition_init_func(struct Transition *aTrans, char *history);
void transition_free_func(struct Transition *aTrans);
int transition_has_history(char *history, struct Transition *aTrans);

/* pairwise knowledge */
struct Pairwise
{
  char name1[NAME_LENGTH];
  char name2[NAME_LENGTH];
  int *preStates;
  double total;
  double estimation;
  double defaultEstimate;
  struct Hashtable transitions;
};
void pairwise_init_func(struct Pairwise *aPairwise, char *name1, char *name2, int k);
void pairwise_free_func(struct Pairwise *aPairwise);
int pairwise_has_names(char *names, struct Pairwise *aPairwise);

struct Oracle
{
  int type;
  /* global knowledge of all pairs */
  struct Hashtable pairwises;

  struct Simulator *aSim;
  time_t trainingStartAt;
  time_t trainingEndAt;

  /* build up the knowledge oracle */
  void(*setup_oracle)(struct Oracle *oracle); 

  //for social oracle
  time_t lastT;
  time_t deltaT; 
  unsigned long numAllEstimations;
  unsigned long numBingoEstimations;
  int onlySocial;

  // for markov oracle 
  int order;
  time_t T;
  time_t tGran;
  int useDefault;

  unsigned long A_B;
  unsigned long AB_;
  unsigned long A_B_;
  unsigned long AB;

  // for SimBet
  double alfar;
  double beta;

  int neighborThreshold;

  // statistic on how much memory used for this oracle
  unsigned long size;
};
void oracle_init_func(struct Oracle *oracle, int type, struct Simulator *aSim, time_t trainingStartAt, time_t trainingEndAt);
void oracle_free_func(struct Oracle *oracle);

void setup_oracle_mkv(struct Oracle *oracle);
void setup_oracle_avg(struct Oracle *oracle);
void setup_oracle_prob(struct Oracle *oracle);
void setup_oracle_bubble(struct Oracle *oracle);
void setup_oracle_simbet(struct Oracle *oracle);
void setup_oracle_social(struct Oracle *oracle);

void setup_neighborhood(struct Oracle *oracle);

/* for ict-related oracle */
struct Pairwise* lookup_pairwise_in_oracle(struct Oracle *oracle, char *vname1, char*vname2);
/* for Markov oracle */
double calculate_estimation(struct Transition *aTrans);
void update_previous_states(int *preStates, int k, int value);
double estimate_next_delay(struct Pairwise *aPairwise, int *preStates, int k, int useDefault);
/* for SimBet and social oracles */
double calculate_betweenness(struct Node *aNode);
double calculate_similarity(struct Node *aNode, struct Node *dstNode);
void assess_neighbor(struct NeighborNode *aNeighborNode, struct Oracle *oracle, time_t current_time, int ictEstimation);
#endif
