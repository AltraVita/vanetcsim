#include<stdlib.h>
#include<string.h>
#include"pkg.h"

#define ADDRESS_LENGTH 32


void pkg_init_func(struct Pkg *aPkg, unsigned id, char* src, char* dst, time_t startAt, unsigned int size, int ttl, double value)
{
	if(aPkg == NULL) 
		return;
	aPkg->id = id;
	strncpy(aPkg->src, src, strlen(src)+1);
	strncpy(aPkg->dst, dst, strlen(dst)+1);
	aPkg->startAt = startAt;
	aPkg->endAt = 0;
	aPkg->size = size;
	aPkg->ttl = ttl;
	aPkg->value = value;
	duallist_init(&aPkg->routingRecord);
}

struct Pkg *pkg_copy_func(struct Pkg *aPkg)
{
	struct Pkg *newPkg;

	if(aPkg == NULL) 
		return NULL;
	newPkg = (struct Pkg*)malloc(sizeof(struct Pkg));
	newPkg->id = aPkg->id;
	strncpy(newPkg->src, aPkg->src, NAME_LENGTH);
	strncpy(newPkg->dst, aPkg->dst, NAME_LENGTH);
	newPkg->startAt = aPkg->startAt;
	newPkg->endAt = aPkg->endAt;
	newPkg->size = aPkg->size;
	newPkg->ttl = aPkg->ttl;
	newPkg->value = aPkg->value;
	duallist_copy(&newPkg->routingRecord, &aPkg->routingRecord,(void*(*)(void*))address_copy_func);
	return newPkg;
}


char* address_copy_func(char * addr)
{
	char *rtAddr;

	if(addr) {
		rtAddr = (char*)malloc(sizeof(char)*ADDRESS_LENGTH);
		strncpy(rtAddr, addr, ADDRESS_LENGTH);
	}
	return rtAddr;
}

void pkg_free_func(struct Pkg *aPkg)
{
	if(aPkg) {
		duallist_destroy(&aPkg->routingRecord, free);
		free(aPkg);
	}
}

int pkg_has_id(int *id, struct Pkg *aPkg)
{
	return *id==aPkg->id; 
}

int pkg_has_earlier_startAt_than(struct Pkg *aPkg, struct Pkg *bPkg)
{
	return aPkg->startAt < bPkg->startAt;
}
