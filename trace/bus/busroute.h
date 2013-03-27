#ifndef BUSROUTE_H
#define BUSROUTE_H

#include <stdio.h>
#include <glib.h>
#include "common.h"
#include "geometry.h"
#include "trace.h"
#include "color.h"

#define STOP_LENGTH 200
#define MAX_DEST_RANGE 500

struct Vote
{
  void *votee;
  unsigned long count;
};

struct CandidatePath
{
  struct Path *path;
  unsigned long count;
};


struct Votetree
{
  GNode *root;
  struct Duallist votes;
  struct Point srcPoint;
  struct Point dstPoint;
  time_t travelTime;
  struct CandidatePath bestCandPath;
};
void votetree_init_func(struct Votetree *aVotetree); 
void votetree_free_func(struct Votetree *aVotetree); 
int vote_has_road(struct Road* aRoad, struct Vote *aVote);
void vote_for_route(struct Trace *aTrace, struct Region *region, struct Votetree *upVotetree, struct Votetree *downVotetree);
void determine_route_path(struct Votetree *aVotetree);

gboolean get_best_path_on_votetree(GNode *aNode, struct Votetree *aVotetree);

struct Stop
{
  int id;
  struct Point gPoint;
  int isDest;
  struct Road* onRoad;
  struct Duallist routes;
};
int stop_has_id(int *id, struct Stop *aStop);
void stop_init_func(struct Stop *aStop);
void stop_free_func(struct Stop *aStop);
void dump_stop(FILE *fstop, struct Stop *aStop);
void dump_stop_with_hashtable(FILE *fstop, struct Hashtable *stops);
struct Stop* load_stop(FILE *fstop, struct Region *aRegion);
void load_stop_with_hashtable(FILE *fstop, struct Region *aRegion, struct Hashtable *stops);

/* bus route */
struct Busroute
{
  char name[NAME_LENGTH];
  struct Box box;
  union Int_RGBO color;
  struct Path *path;
  struct Duallist stops;
};
int route_has_name(char *name, struct Busroute* aRoute);
void route_init_func(struct Busroute *aRoute);
void route_free_func(struct Busroute *aRoute);
void dump_route(FILE *fOutput, struct Busroute *aRoute);
struct Busroute *load_route(FILE *fInput, struct Region *aRegion);
void load_route_with_hashtable(FILE *froute, struct Region *aRegion, struct Hashtable *routes);

struct Duallist* get_route_coverage(struct Region *region, struct Busroute *aRoute);

void setup_routes_and_stops(struct Hashtable *routes, struct Hashtable *stops);
void setup_cells_with_routes(struct Region *region, struct Hashtable *routes);

#endif
