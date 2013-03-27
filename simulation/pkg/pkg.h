#ifndef PKG_H
#define PKG_H

#include<time.h>
#include"common.h"

struct Pkg
{
  unsigned id;
  char src[2*NAME_LENGTH];
  char dst[2*NAME_LENGTH];
  time_t startAt;
  time_t endAt;
  unsigned size;
  // TTL
  int ttl;
  //all purpose value  
  double value;
  // routing record
  struct Duallist routingRecord;
};

void pkg_init_func(struct Pkg *aPkg, unsigned id, char* src, char* dst, time_t startAt, unsigned int size, int ttl, double value);
char* address_copy_func(char * addr);
struct Pkg *pkg_copy_func(struct Pkg *aPkg);
void pkg_free_func(struct Pkg *aPkg);
int pkg_has_id(int *id, struct Pkg *aPkg);
int pkg_has_earlier_startAt_than(struct Pkg *aPkg, struct Pkg *bPkg);
#endif
