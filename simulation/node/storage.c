#include<stdlib.h>
#include"common.h"
#include"storage.h"
#include"errors.h"

void storage_init_func(struct Storage *storage, unsigned long size)
{
	if(storage) {
		duallist_init(&storage->pkgs);
		storage->size = size;
		storage->usage = 0;
	}
}

int storage_add_pkg(struct Storage *storage, struct Pkg *aPkg)
{
	if(storage && aPkg) {
		if(storage->size == -1 || storage->usage + aPkg->size <= storage->size) {
			duallist_add_in_sequence_from_tail(&storage->pkgs, aPkg, (int(*)(void*,void*))pkg_has_earlier_startAt_than);
			storage->usage += aPkg->size;
			return 0;
		}  else {
			pkg_free_func(aPkg);
			return NO_STORAGE_SPACE_ERROR;
		}
	}
	return NULL_STORAGE_OR_PKG_ERROR;
}

void storage_remove_pkg(struct Storage *storage, int pkgId)
{
	struct Item *aItem;

	if(storage){
		aItem = duallist_find(&storage->pkgs, &pkgId,(int(*)(void*,void*))pkg_has_id);
		if(aItem) {
			storage->usage -= ((struct Pkg*)aItem->datap)->size;
			pkg_free_func( (struct Pkg*)duallist_pick_item(&storage->pkgs, aItem));
		}
	}
}


void storage_free_func(struct Storage *storage)
{
	if(storage) {
		while(storage->pkgs.head) {
			pkg_free_func((struct Pkg*)duallist_pick_head(&storage->pkgs));
		}
		free(storage);
	}
}


struct Pkg* storage_lookup_pkg(struct Storage *storage, int pkgId)
{
	struct Item *aItem;

	if(storage){
		aItem = duallist_find(&storage->pkgs, &pkgId, (int(*)(void*,void*))pkg_has_id);
		if(aItem)
			return aItem->datap;
	}
	return NULL;
}

