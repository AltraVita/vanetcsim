#ifndef STORAGE_H
#define STORAGE_H

#include"common.h"
#include"pkg.h"

struct Storage
{
  struct Duallist pkgs;
  unsigned long size;
  unsigned long usage;
};
void storage_init_func(struct Storage *storage, unsigned long size);
int storage_add_pkg(struct Storage *storage, struct Pkg *aPkg);
void storage_remove_pkg(struct Storage *storage, int pkgId);
void storage_free_func(struct Storage *storage);
struct Pkg* storage_lookup_pkg(struct Storage *storage, int pkgId);

#endif
