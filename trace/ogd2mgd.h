#ifndef OGD2MGD_H
#define OGD2MGD_H

#define WEIGHT_ANGLE_ERROR 0.4 
#define WEIGHT_TURNS 0.3 
#define WEIGHT_DISTANCE_ERROR 0.2 
#define WEIGHT_LENGTH 0.1 

#define MAX_TURNS 5
#define MAX_LENGTH 2000

#define TOP_N_CANDIDATES 5


struct CandChoice {
  struct Item *onRoad;
  double score;
};

int candchoice_has_smaller_score_than(struct CandChoice *aCandChoice, struct CandChoice *bCandChoice);
int candchoice_has_larger_score_than(struct CandChoice *aCandChoice, struct CandChoice *bCandChoice);
void candchoice_free_func(struct CandChoice *aCandChoice);
void amend_trace(struct Region *aRegion, struct Trace *aTrace, double wdist, double wangle, double wlength, double wturns, struct Trace **rtTrace, struct Trace **dTrace);
void amend_report(struct Region *aRegion, struct Trace *aTrace, double wdist, double wangle, double wlength, double wturns);
double calculate_choice_weight(double distance, double angle, double length, int turns, double wdist, double wangle, double wlength, double wturns);

#endif
