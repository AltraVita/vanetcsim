#ifndef TRAFFIC_H
#define TRAFFIC_H

#include<stdio.h>
#include"simulator.h"
#include"common.h"

#define TYPE_TRAFFIC_GENERATOR_POISSON 4
#define TYPE_TRAFFIC_GENERATOR_LOAD 5 

#define LOAD_DELIEVERED_PKGS 0
#define LOAD_ALL_PKGS 1

#define SELECT_RANDOM 0
#define SELECT_FRIENDS 1
#define SELECT_STRANGERS 2

struct TrafficGenerator
{
  int numPkgs;
  time_t startAt;
  int selectPolicy;
  //generator specific data
  time_t value;
};

void trafficGenerator_init_func(struct TrafficGenerator *trafficGenerator, time_t trafficStartAt, int numPkgs, time_t mean, int selectPolicy);
void trafficGenerator_free_func(struct TrafficGenerator *trafficGenerator);

void generate_v2v_poisson_traffic(struct Simulator *aSim);
void generate_b2l_poisson_traffic(struct Simulator *aSim);
time_t ExpRnd(time_t mean);

void load_traffic(FILE *fload, struct Duallist *pkgs, unsigned long *traffic, int aim);
void dump_traffic(FILE *fdump, struct Duallist *pkgs, unsigned long traffic);
#endif
