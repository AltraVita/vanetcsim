#include<stdlib.h>
#include<string.h>
#include "contact.h"
#include "node.h"

void node_init_func(struct Node *aNode, char *vName, char *onRoute, unsigned long bufSize)
{
	if(aNode == NULL)
		return ;
	if(onRoute)
		sprintf(aNode->name, "%s@%s", vName, onRoute);
	else
		strncpy(aNode->name, vName, NAME_LENGTH);
	aNode->gPoint.x = 0;
	aNode->gPoint.y = 0;
	aNode->betweenness = 0;
	aNode->storage = (struct Storage*)malloc(sizeof(struct Storage));
	storage_init_func(aNode->storage, bufSize);
	hashtable_init(&aNode->metNodes, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))metNode_has_name);
	hashtable_init(&aNode->neighbors, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))neighborNode_has_name);
}

int metNode_has_name(char *name, struct MetNode *aMetNode)
{
	return !strcmp(name, aMetNode->name);
}

void neighborNode_init_func(struct NeighborNode *aNeighborNode, struct Node *aNode)
{
	if(aNeighborNode) {
		aNeighborNode->node = aNode;
		aNeighborNode->strength = 1;
		duallist_init(&aNeighborNode->lastEstimations);
	}
}

void neighborNode_free_func(struct NeighborNode *aNeighborNode)
{
	if(aNeighborNode) {
		duallist_destroy(&aNeighborNode->lastEstimations, free);
		free(aNeighborNode);
	}
}

int neighborNode_has_name(char *name, struct NeighborNode *aNeighbor)
{
	return !strcmp(name, aNeighbor->node->name);
}

/* Establish a neighbor if met at least threshold times.
 * Note that the strength between aNode and bNode is set to default value 1,
 * more sofisticated method should overcast this value after calling this function.
 */
struct NeighborNode* node_met_a_node(struct Node *aNode, struct Node *bNode, int threshold)
{
	struct Item *aItem;
	struct MetNode *aMetNode;
	struct NeighborNode *aNeighborNode;

	aItem = hashtable_find(&aNode->metNodes, bNode->name);
	if(aItem != NULL) {
		aMetNode = (struct MetNode*)aItem->datap;
	} else {
		aMetNode = (struct MetNode*)malloc(sizeof(struct MetNode));
		sprintf(aMetNode->name, "%s", bNode->name);
		aMetNode->value = 0;
		hashtable_add(&aNode->metNodes, bNode->name, aMetNode);
	} 
	aMetNode->value += 1;

	if(aMetNode->value >= threshold) {
		aItem = hashtable_find(&aNode->neighbors, bNode->name);
		if(aItem==NULL) {
			aNeighborNode = (struct NeighborNode*)malloc(sizeof(struct NeighborNode));
			neighborNode_init_func(aNeighborNode, bNode);
			hashtable_add(&aNode->neighbors, bNode->name, aNeighborNode);
		} else
			aNeighborNode = (struct NeighborNode*)aItem->datap;
		return aNeighborNode;
	} else
		return NULL;
}

char *node_on_route(struct Node *aNode)
{
	char *strp;
	if(aNode == NULL)
		return NULL;
	if((strp = strstr(aNode->name, "@")))
		strp ++;
	return strp;
}

void node_free_func(struct Node *aNode)
{
	storage_free_func(aNode->storage);
	hashtable_destroy(&aNode->metNodes, free);
	hashtable_destroy(&aNode->neighbors, (void(*)(void*))neighborNode_free_func);
	free(aNode);
}


int node_has_name(char *name, struct Node* aNode)
{
	return !strcmp(name, aNode->name);
}

struct Node* randomly_pick_a_node(struct Hashtable *nodes, int type)
{
	int index;
	unsigned long i;
	struct Item *aItem;
	struct Node *aNode;

	if(nodes->count==0)
		return NULL;

	index = rand()%nodes->count+1;
	for(i=0;i<nodes->size;i++) {
		aItem = nodes->head[i];
		while(aItem != NULL) {
			aNode = (struct Node*)aItem->datap;
			if((type==VEHICLE_TYPE_BUS && aNode->name[0]=='b')
			|| (type==VEHICLE_TYPE_TAXI && aNode->name[0]=='t')
			|| type==VEHICLE_TYPE_NULL)
				index --;
			if(index == 0) {
				return aNode;
			}
			aItem = aItem->next;
		}
	}
	return NULL;
}

struct Node* lookup_node(struct Hashtable *nodes, char *name) 
{
	struct Item *aItem;
	struct Node *aNode;
	char key[64];

	sprintf(key, "%s", name);
	aItem = hashtable_find(nodes, key);
	if(aItem != NULL) {
		aNode = (struct Node*)aItem->datap;
	} else {
		aNode = NULL;
	} 
	return aNode;
}



void setup_vehicular_nodes_by_pairs(struct Hashtable *pairTable, struct Hashtable *nodeTable, unsigned long bufSize)
{
	unsigned long i;
	struct Item *aPairItem;
	struct Pair *aPair;
	struct Node *aNode;

	for(i=0;i<pairTable->size;i++) {
		aPairItem = pairTable->head[i];
		while(aPairItem != NULL) {
			aPair = (struct Pair*)aPairItem->datap;

			aNode=lookup_node(nodeTable, aPair->vName1);
			if(aNode == NULL) {
				aNode = (struct Node*)malloc(sizeof(struct Node));
				node_init_func(aNode, aPair->vName1, NULL, bufSize);
				hashtable_add(nodeTable, aPair->vName1, aNode);
			}
			aNode=lookup_node(nodeTable, aPair->vName2);
			if(aNode == NULL) {
				aNode = (struct Node*)malloc(sizeof(struct Node));
				node_init_func(aNode, aPair->vName2, NULL, bufSize);
				hashtable_add(nodeTable, aPair->vName2, aNode);
			}
			aPairItem = aPairItem->next;
		}
	}
}


// call this function after setup_cells_with_routes()
void deploy_static_nodes_at_most_routes_cells(struct Region *region, struct Hashtable *nodeTable, unsigned long bufSize)
{
	struct Item *aItem, *bItem;
	struct Cell *aCell, *bCell;
	struct Duallist aList, sList;
	struct Node *aNode;
	char buf[32];
	unsigned long i, j;
	int notsublist;

	if(region->busCoveredCells.nItems == 0)
		return;

	duallist_init(&aList);
	for(i = 0; i<region->hCells; i++)
		for(j = 0; j<region->vCells;j++) { 
			aCell = region->mesh + i*region->vCells + j;
			if(aCell->routes.nItems > 1)
				duallist_add_in_sequence_from_head(&aList, aCell, (int(*)(void*, void*))cell_has_less_routes_than);
		}	

	duallist_init(&sList);
	aItem = aList.head;
	while(aItem) {
		aCell = (struct Cell*)aItem->datap;
		bItem = sList.head;
		notsublist = 1;
		while(bItem) {
			bCell = (struct Cell*)bItem->datap;
			if(is_sublist(&aCell->routes, &bCell->routes, (int(*)(void*, void*))addr_equal_func)) {
				notsublist = 0;
				break;
			}
			bItem = bItem->next;
		}
		if(notsublist) 
			duallist_add_to_tail(&sList, aCell);
		aItem = aItem->next;
	}
	duallist_destroy(&aList, NULL);

	aItem = sList.head;
	while(aItem) {
		aCell = (struct Cell*)aItem->datap;
		sprintf(buf, "s(%d,%d)", aCell->xNumber, aCell->yNumber);
		aNode = (struct Node*)malloc(sizeof(struct Node));
		node_init_func(aNode, buf, NULL, bufSize);
		aNode->gPoint.x = (aCell->box.xmin+aCell->box.xmax)/2;
		aNode->gPoint.y = (aCell->box.ymin+aCell->box.ymax)/2;
		hashtable_add(nodeTable, buf, aNode);
		aCell->storage = aNode;
		aItem = aItem->next;
	}
}

