#ifndef TRAJ_H
#define TRAJ_H

#include<time.h>
#include"common.h"
#include"geometry.h"
#include"trace.h"

#define TRAJ_TRUSTWORTHY 0
#define TRAJ_MALICIOUS 1
#define TRAJ_SUSPICIOUS 2

struct Landmark
{
  int crossId;
  time_t timestamp;
  struct Point gPoint;
}; 

struct Trajectory
{
  char vName[NAME_LENGTH];
  /* trajectory for changshan */
  struct Duallist landmarks;
  int nParts;
  int trustworth;
};

void traj_init_func(struct Trajectory *aTraj, char *name);
int traj_has_name(char *name, struct Trajectory* aTraj);
void traj_free_func(struct Trajectory *aTraj);

void traj_dump_func(FILE *fp, struct Trajectory *aTraj);
struct Trajectory* load_traj_with_hashtable(FILE *ftraj, struct Hashtable *trajs);
struct Trajectory* load_traj_with_duallist(FILE *ftraj, struct Duallist *trajs);
int log_trajectory(struct Trace *aTrace, struct Region *aRegion, struct Region *rstRegion);
void spoof_a_traj_using_stack(struct Duallist *spooftrajs, struct Trajectory *aTraj, struct Region *rsuRegion, int k, int number);
int are_two_landmarks_neighbors(struct Region *rsuRegion, struct Landmark *aLandmark, struct Landmark *bLandmark);
time_t get_parts_in_traj(struct Duallist *parts, struct Trajectory *aTraj);
double similarity_between_two_trajs(struct Trajectory *aTraj, struct Trajectory *bTraj, time_t checkwindow, time_t T, int K);
int check_two_parts(struct Duallist *aPart, struct Duallist *bPart, time_t checkwindow);
int argmax_C_n_x_larger_than_m(int n, int m);
int C_n_x(int n, int x);

#endif
