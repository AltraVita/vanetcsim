#ifndef NODE_H
#define NODE_H

#include "common.h"
#include "geometry.h"
#include "busroute.h"
#include "storage.h"

// a node may refers to a vehicle or a static device installed on road side
struct Node
{
  char name[2*NAME_LENGTH];
  // static storage node's location
  struct Point gPoint;
  
  struct Storage *storage;

  struct Hashtable metNodes;
  struct Hashtable neighbors;

  double betweenness;
};

void node_init_func(struct Node *aNode, char *vName, char *onRoute, unsigned long bufSize);
void node_free_func(struct Node *aNode);
int node_has_name(char *name, struct Node* aNode);


struct MetNode
{
  char name[2*NAME_LENGTH];
  double value;
};
int metNode_has_name(char *name, struct MetNode *aMetNode);

struct NeighborNode
{
  struct Node *node;
  struct Duallist lastEstimations;
  double strength;
};
void neighborNode_init_func(struct NeighborNode *aNeighborNode, struct Node *aNode);
void neighborNode_free_func(struct NeighborNode *aNeighborNode);
int neighborNode_has_name(char *name, struct NeighborNode *aNeighbor);

struct Estimation
{
  time_t timestamp;
  int estimatedTime;
  int bingo;
};

char *node_on_route(struct Node *aNode);
struct Node* randomly_pick_a_node(struct Hashtable *nodes, int type);

struct Node* lookup_node(struct Hashtable *nodes, char *name);

void setup_vehicular_nodes_by_pairs(struct Hashtable *pairTable, struct Hashtable *nodeTable, unsigned long bufSize);
void deploy_static_nodes_at_most_routes_cells(struct Region *region, struct Hashtable *nodeTable, unsigned long bufSize);


/* return a neighbor node if this meeting makes bNode a neighbor of aNode */
struct NeighborNode* node_met_a_node(struct Node *aNode, struct Node *bNode, int threshold);

#endif
