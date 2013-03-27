#include<stdlib.h>
#include<string.h>
#include"simulator.h"
#include"contact.h"
#include"traffic.h"
#include"busroute.h"
#include"node.h"

void simulator_init_func(struct Simulator *aSim, time_t exprStartAt, time_t exprEndAt, int fwdMethod, unsigned int bufSize, unsigned int pkgSize, int pkgTTL, int pkgRcdRoute)
{
	int i; 

	if(aSim == NULL)
		return;

	aSim->region = NULL;
	aSim->oracle = NULL;
	aSim->trafficGenerator = NULL;
	duallist_init(&aSim->pkgs);
	aSim->eventSlots = (struct Duallist*)malloc(sizeof(struct Duallist)*EVENT_SLOTS);
	for(i=0;i<EVENT_SLOTS;i++)
		duallist_init(aSim->eventSlots+i);
	aSim->slotSize=(exprEndAt-exprStartAt)/EVENT_SLOTS;
	aSim->eventNums = 0;
	aSim->fwdMethod = fwdMethod;
	aSim->exprStartAt = exprStartAt;
	aSim->exprEndAt = exprEndAt;
	aSim->clock = exprStartAt;

	aSim->bufSize = bufSize;
	aSim->pkgSize = pkgSize;
	aSim->pkgTTL = pkgTTL;
	aSim->pkgRcdRoute = pkgRcdRoute;

	aSim->trafficCount = 0;

	hashtable_init(&aSim->routes, 100, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);
	hashtable_init(&aSim->vnodes, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))node_has_name);
	hashtable_init(&aSim->snodes, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))node_has_name);
  	hashtable_init(&aSim->cntTable, 100000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  	hashtable_init(&aSim->ictTable, 100000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
	hashtable_init(&aSim->destinations, 200, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))cell_has_nos);
}


void simulator_free_func(struct Simulator *aSim)
{
	int i;

	if(aSim == NULL)
		return;
	if(aSim->region)
		region_free_func(aSim->region);
	if(aSim->oracle)
	  	oracle_free_func(aSim->oracle); 
	if(aSim->trafficGenerator)
		trafficGenerator_free_func(aSim->trafficGenerator);
	for(i=0;i<EVENT_SLOTS;i++)
		duallist_destroy(aSim->eventSlots+i, free);
	free(aSim->eventSlots);
	duallist_destroy(&aSim->pkgs,(void(*)(void*))pkg_free_func);
	hashtable_destroy(&aSim->routes, (void(*)(void*))route_free_func);
	hashtable_destroy(&aSim->vnodes, (void(*)(void*))node_free_func);
	hashtable_destroy(&aSim->snodes, (void(*)(void*))node_free_func);
	hashtable_destroy(&aSim->cntTable, (void(*)(void*))pair_free_func);
  	hashtable_destroy(&aSim->ictTable, (void(*)(void*))pair_free_func);
	hashtable_destroy(&aSim->destinations, NULL);
	free(aSim);
}

