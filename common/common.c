#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include"common.h"



/*
 *  Dual list 
 */

struct Duallist* duallist_copy_by_reference(struct Duallist *destDuallist, struct Duallist *aDuallist)
{
	struct Item *aItem;

	if(destDuallist == NULL) {
		destDuallist = (struct Duallist*)malloc(sizeof(struct Duallist));
	}
	duallist_init(destDuallist);

	if(aDuallist!=NULL) {
		aItem = aDuallist->head;
		while(aItem!=NULL) {
			duallist_add_to_tail(destDuallist, aItem->datap);
			aItem=aItem->next;
		}
	} 
	return destDuallist; 
}


struct Duallist* duallist_copy(struct Duallist *destDuallist, struct Duallist *aDuallist, void*(*copy_func)(void*))
{
	struct Item *aItem;

	if(destDuallist == NULL) {
		destDuallist = (struct Duallist*)malloc(sizeof(struct Duallist));
	}
	duallist_init(destDuallist);
	if(aDuallist!=NULL) {
		aItem = aDuallist->head;
		while(aItem!=NULL) {
			duallist_add_to_tail(destDuallist, copy_func(aItem->datap));
			aItem=aItem->next;
		}
	}	
	return destDuallist; 
}


struct Duallist* duallist_reverse_copy(struct Duallist *destDuallist, struct Duallist *aDuallist, void*(*copy_func)(void*))
{
	struct Item *aItem;

	if(destDuallist == NULL) {
		destDuallist = (struct Duallist*)malloc(sizeof(struct Duallist));
	}
	duallist_init(destDuallist);
	if(aDuallist!=NULL) {
		aItem = aDuallist->head;
		while(aItem!=NULL) {
			duallist_add_to_head(destDuallist, copy_func(aItem->datap));
			aItem=aItem->next;
		}
	}	
	return destDuallist; 
}


void duallist_init(struct Duallist *duallist)
{
	if(duallist == NULL) {
		return;
	}
	duallist->head = NULL;
	duallist->nItems = 0;
}

struct Item* duallist_add_to_head(struct Duallist* duallist, void *data)
{
	struct Item *newp;

	if(duallist == NULL) {
		return NULL;
	}
	newp = (struct Item*)malloc(sizeof(struct Item));
	newp->datap = data;
	if(duallist->head != NULL) {
		newp->next = duallist->head;
		newp->prev = duallist->head->prev;
		duallist->head->prev = newp;
		duallist->head = newp;
	} else {
		newp->next = NULL;
		newp->prev = newp;
		duallist->head = newp;
	}		
	duallist->nItems ++;
	return newp;
}

struct Item* duallist_add_to_tail(struct Duallist *duallist, void *data)
{
	struct Item *newp;

	if(duallist == NULL) {
		return NULL;
	}
	newp = (struct Item*)malloc(sizeof(struct Item));
	newp->datap = data;
	if(duallist->head != NULL) {
		newp->next = NULL;
		newp->prev = duallist->head->prev;
		duallist->head->prev->next = newp;
		duallist->head->prev = newp;
	} else {
		newp->next = NULL;
		newp->prev = newp;
		duallist->head = newp;
	}		
	duallist->nItems ++;
	return newp;
}


struct Item* duallist_add_in_sequence_from_head(struct Duallist *duallist, void* data, int(*sort_func)(void*, void*))
{
	struct Item *newp, *aItem;

	if(duallist == NULL) {
		return NULL;
	}
	newp = (struct Item*)malloc(sizeof(struct Item));
	newp->datap = data;
	if(duallist->head != NULL) {
		aItem = duallist->head;
		while(aItem->next != NULL && sort_func(data, aItem->datap)){
			aItem = aItem->next;
		}
		if(!sort_func(data, aItem->datap)) {
			if (aItem == duallist->head) {
				newp->next = aItem;
				newp->prev = aItem->prev;
				aItem->prev = newp;
				duallist->head = newp;
			} else {
				newp->next = aItem;
				newp->prev = aItem->prev;
				aItem->prev = newp;
				newp->prev->next = newp;
			}
		} else {
			newp->next = NULL;
			aItem->next = newp;
			newp->prev = aItem;
			duallist->head->prev = newp;
		}
	} else {
		newp->next = NULL;
		newp->prev = newp;
		duallist->head = newp;
	}		
	duallist->nItems ++;
	return newp;
}

struct Item* duallist_add_in_sequence_from_tail(struct Duallist *duallist, void* data, int(*sort_func)(void*, void*))
{
	struct Item *newp, *aItem;

	if(duallist == NULL) {
		return NULL;
	}
	newp = (struct Item*)malloc(sizeof(struct Item));
	newp->datap = data;
	if(duallist->head != NULL) {
		aItem = duallist->head->prev;
		while(aItem != duallist->head && sort_func(data, aItem->datap)){
			aItem = aItem->prev;
		}
		if(!sort_func(data, aItem->datap)) {
			if (aItem->next == NULL) {
				newp->next = NULL;
				newp->prev = aItem;
				aItem->next = newp;
				duallist->head->prev = newp;
			} else {
				newp->next = aItem->next;
				newp->prev = aItem;
				aItem->next->prev = newp;
				aItem->next = newp;
			}
		} else {
			newp->next = aItem;
			newp->prev = aItem->prev;
			aItem->prev = newp;
			duallist->head = newp;
		}
	} else {
		newp->next = NULL;
		newp->prev = newp;
		duallist->head = newp;
	}		
	duallist->nItems ++;
	return newp;
}

/*
 * Find the location of an item in a duallist
 */
struct Item* duallist_find(struct Duallist *duallist, void *key, int(*judge_func)(void*, void*))
{
	struct Item *aItem;

	if(duallist == NULL) {
		return NULL;
	}
	aItem = duallist->head;
	while(aItem != NULL) {
		if(judge_func(key, aItem->datap))
			return aItem;
		aItem = aItem->next;
	}
	return NULL;
}

/*
 * Pick up an item from the duallist and return the 
 * wrapped data.
 */
void* duallist_pick(struct Duallist *duallist, void *key, int(*judge_func)(void*, void*))
{
	struct Item *aItem;
	void *rt = NULL;

	if(duallist == NULL) {
		return NULL;
	}
	aItem = duallist->head;
	while(aItem != NULL) {
		if(judge_func(key, aItem->datap)) {
			if(aItem == duallist->head) {
				duallist->head = aItem->next;
				if(aItem->next != NULL) 
					duallist->head->prev = aItem->prev;
			} else {
				aItem->prev->next = aItem->next;
				if(aItem->next != NULL) 
					aItem->next->prev = aItem->prev;
				else
					duallist->head->prev = aItem->prev;	
			}
			rt = aItem->datap;
			free(aItem);
			duallist->nItems --;
			return rt;
		}
		aItem = aItem->next;
	}
	return NULL;
}

void* duallist_pick_item(struct Duallist *duallist, struct Item *theItem)
{
	void *rt;

	if(duallist == NULL) {
		return NULL;
	}
	if(theItem == duallist->head) {
		duallist->head = theItem->next;
		if(theItem->next != NULL) 
			duallist->head->prev = theItem->prev;
	} else {
		theItem->prev->next = theItem->next;
		if(theItem->next != NULL) 
			theItem->next->prev = theItem->prev;
		else
			duallist->head->prev = theItem->prev;	
	}
	rt = theItem->datap;
	free(theItem);
	duallist->nItems --;
	return rt;
}

/*
 * Pick up the head item from the duallist and return the 
 * wrapped data.
 */
void * duallist_pick_head(struct Duallist *duallist)
{
	struct Item *aItem;
	void *rt;

	if(duallist == NULL) {
		return NULL;
	}
	if(duallist->head != NULL) {
		aItem = duallist->head;
		duallist->head = aItem->next;
		if(aItem->next != NULL) 
			duallist->head->prev = aItem->prev;
		rt = aItem->datap;
		free(aItem);
		duallist->nItems --;
		return rt;
	} 
	return NULL;
}

/*
 * Pick up the tail item from the duallist and return the 
 * wrapped data.
 */
void * duallist_pick_tail(struct Duallist *duallist)
{
	struct Item *aItem;
	void *rt;

	if(duallist == NULL) {
		return NULL;
	}
	if(duallist->head != NULL) {
		aItem = duallist->head->prev;
		if(aItem->prev == aItem) 
			duallist->head = NULL;
		else {
			aItem->prev->next = NULL;
			duallist->head->prev = aItem->prev;
		}
		rt = aItem->datap;
		free(aItem);
		duallist->nItems --;
		return rt;
	} 
	return NULL;
}

int is_duallist_empty(struct Duallist *duallist)
{
	if(duallist == NULL) {
		return -1;
	}
	if(duallist->head != NULL)
		return 0;
	return 1;
}

void duallist_destroy(struct Duallist *duallist, void (*free_func)(void*))
{
	void *datap;

	if(duallist == NULL) {
		return ;
	}
	while (!is_duallist_empty(duallist)) {
		datap = duallist_pick_head(duallist);
		if(free_func)
			free_func(datap);
	}
}

void curtain_destroy(struct Duallist *duallist, void(*free_func)(void*))
{
	struct Duallist *aDuallist;

	if(duallist == NULL) {
		return ;
	}
	while (!is_duallist_empty(duallist)) {
		aDuallist = (struct Duallist*)duallist_pick_head(duallist);
		duallist_destroy(aDuallist, free_func);
		free(aDuallist);
	}
}


void duallist_dump(FILE *fOutput, struct Duallist *duallist, void(*dump_func)(FILE*, void*))
{
	unsigned long i;
	struct Item *aItem;

	if(duallist == NULL) {
		return ;
	}
	fwrite(&duallist->nItems, sizeof(unsigned long), 1, fOutput);
	aItem = duallist->head;
	for(i=0;i<duallist->nItems;i++) {
		dump_func(fOutput, &aItem->datap);
		aItem = aItem->next;
	}
}


void duallist_load(FILE *fInput, struct Duallist *duallist, void*(*load_func)(FILE*))
{
	unsigned long i, nItems;
	
	if(duallist == NULL) {
		return ;
	}
	duallist_init(duallist);	
	fread(&nItems, sizeof(unsigned long), 1, fInput);
	for (i = 0; i<nItems; i++) 
		duallist_add_to_tail(duallist, load_func(fInput));
}

unsigned long distance_to_tail(struct Item *aItem)
{
	unsigned long count = 0;

	while(aItem) {
		count ++;
		aItem = aItem->next;
	}
	return count;
}


unsigned long distance_to_head(struct Duallist *aDuallist, struct Item *aItem)
{
	unsigned long count = 0;

	while(aItem && aItem!=aDuallist->head) {
		count ++;
		aItem = aItem->prev;
	}
	return count;
}

int is_sublist(struct Duallist *aList, struct Duallist *bList, int(*equal_func)(void*, void*))
{
	struct Item *aItem, *bItem;
	int foundItem; 

	if(aList && bList) {
		aItem = aList->head;
		while(aItem) {
			bItem = bList->head;
			foundItem = 0;
			while(bItem) {
				if(equal_func(aItem->datap, bItem->datap))
					foundItem = 1;
				bItem = bItem->next;
			}
			if(!foundItem)
				return 0;
			aItem = aItem->next;
		}	
		return 1;
	}
	return 0;
}

void duallist_remove_loops(struct Duallist *aList, int(*equal_func)(void*,void*), void(*free_func)(void*))
{
	struct Item *aItem, *bItem, *temp;
	void *datap;

	if(aList) {
		aItem = aList->head;
		while(aItem) {
			bItem = aItem->next;
			while(bItem) {
				if(equal_func && equal_func(aItem->datap, bItem->datap)) {
					temp = aItem->next;
					while(temp!=bItem->next) {
						datap = duallist_pick_item(aList, aItem);
						if(free_func)
							free_func(datap);
						aItem = temp;
						temp = aItem->next;	
					}
				}
				bItem = bItem->next;
			}
			aItem = aItem->next;
		}
	}
}



/*
 * Stack implementation
 */
void stack_init(struct Duallist *stack)
{
	if(stack == NULL) {
		return ;
	}
	stack->head = NULL;
}

struct Item* stack_push(struct Duallist* stack, void *data)
{
	if(stack == NULL) {
		return NULL ;
	}
	return duallist_add_to_head(stack, data);	
}

void* stack_pop(struct Duallist* stack)
{
	if(stack == NULL) {
		return NULL;
	}
	return duallist_pick_head(stack);
}

int is_stack_empty(struct Duallist* stack)
{
	if(stack == NULL) {
		return -1;
	}
	return is_duallist_empty(stack);
}

/*
 * Queue implementation
 */
void queue_init(struct Duallist* queue)
{
	if(queue == NULL) {
		return ;
	}
	queue->head = NULL;
}

struct Item* queue_add(struct Duallist*queue, void* data)
{
	if(queue == NULL) {
		return NULL;
	}
	return duallist_add_to_tail(queue, data);	
}

void* queue_pick(struct Duallist *queue)
{
	if(queue == NULL) {
		return NULL;
	}
	return duallist_pick_head(queue);
}

int is_queue_empty(struct Duallist *queue)
{
	if(queue == NULL) {
		return -1;
	}
	return is_duallist_empty(queue);
}

/*
 *  Hash table
 */
void hashtable_init(struct Hashtable *table, unsigned long size, unsigned long (*hash_func)(void *), int(*equal_func)(void*, void*))
{
	unsigned long i;
	if(table == NULL) {
		return ;
	}
	table->head = (struct Item **)malloc(sizeof(struct Item*)*size);
	if(table->head == NULL) {
		printf("No more memory for malloc()!\n");
		return;
	}
	table->size = size;
	for(i = 0; i < size; i++)
		(table->head)[i] = NULL;
	table->count = 0;
	table->entries = 0;
	table->hash_func = hash_func;
	table->equal_func = equal_func;
}

void hashtable_destroy(struct Hashtable *table, void(*free_func)(void*))
{
	struct Item *aItem, *bItem;
	unsigned long i;

	if(table == NULL) {
		return; 
	}
	for(i = 0; i < table->size; i++) {
		if((table->head)[i] == NULL)
			continue;
		else {
			aItem = (table->head)[i];
			while(aItem != NULL) {
				bItem = aItem->next;
				if(free_func)
					free_func(aItem->datap);
			      	free(aItem);
			      	aItem = bItem;
	      		}
      		}
	}
	free(table->head);
}

/* allow duplicated entries to be added */
struct Item* hashtable_add(struct Hashtable *table, void*key, void*data)
{
	unsigned long index;
	struct Item *newp;

	if(table == NULL) { 
		return NULL;
	}
	newp = (struct Item*)malloc(sizeof(struct Item));
	newp->datap = data;

	index = table->hash_func(key)%table->size;
	if((table->head)[index] == NULL) {
		(table->head)[index] = newp;
		newp->next = NULL;
		newp->prev = newp;
		table->entries ++;
	} else {
		newp->next = (table->head)[index];
		newp->prev = (table->head)[index]->prev;
		(table->head)[index]->prev = newp;
		(table->head)[index] = newp;
	}
	table->count ++;
	return newp;
}

struct Item* hashtable_add_unique(struct Hashtable *table, void *key, void *data)
{
	struct Item *pItem;
	
	pItem = hashtable_find(table, key);
	if(pItem == NULL) 
		return hashtable_add(table, key, data);
	else
		return NULL;
}

struct Item* hashtable_next_item(struct Hashtable *table, struct Item *aItem)
{
	unsigned long i;
	char flag = 0;
	struct Item *bItem;

	for(i = 0; i < table->size; i++) {
		bItem = table->head[i];
		while (bItem){
			if(flag)
				return bItem;

			if(bItem == aItem){
				flag = 1;
			}
			bItem = bItem->next;
		}
	}
	return NULL;
}

/* return the satisfied item's address */
struct Item* hashtable_find(struct Hashtable *table, void*key)
{
	unsigned long index;
	struct Item *pItem;

	if(table == NULL) {
		return NULL;
	}
	index = table->hash_func(key)%table->size;
	if((table->head)[index] == NULL) {
	    	return NULL;
	} else {
		pItem = (table->head)[index];
		while(pItem != NULL) {
			if(table->equal_func(key, pItem->datap))
				return pItem;
			pItem = pItem->next;
		}
	}
	return NULL;
}

void* hashtable_pick(struct Hashtable *table, void*key)
{
	unsigned long index;
	struct Item *pItem;
	void *rt;

	if(table == NULL) {
		return NULL;
	}
	index = table->hash_func(key)%table->size;
	if((table->head)[index] == NULL) {
	    	return NULL;
	} else {
		pItem = (table->head)[index];
		while(pItem != NULL) {
			if(table->equal_func(key, pItem->datap)) {
				if(pItem == table->head[index]) {
					table->head[index] = pItem->next;
					if(pItem->next != NULL) {
						table->head[index]->prev = pItem->prev;
					} else
						table->entries --;
				} else {
					pItem->prev->next = pItem->next;
					if(pItem->next != NULL) 
						pItem->next->prev = pItem->prev;
					else
						table->head[index]->prev = pItem->prev;	
				}
				rt = pItem->datap;
				free(pItem);
				table->count --;
				return rt;
			}
			pItem = pItem->next;
		}
	}
	return NULL;
}

void hashtable_dump(FILE *fOutput, struct Hashtable *table, void(*dump_func)(FILE*, void*))
{
	struct Item *aItem;
	unsigned long i, flag;

	if(table == NULL) {
		return;
	}

	fwrite(&table->size, sizeof(unsigned long), 1, fOutput);
	for(i = 0; i < table->size; i++) {
		aItem = table->head[i];
		while (aItem != NULL){
			flag = 1;
			fwrite(&flag, sizeof(int), 1, fOutput);
			dump_func(fOutput, aItem->datap);
			aItem = aItem->next;
		}
		flag = 0;
		fwrite(&flag, sizeof(int), 1, fOutput);
	}
}


void hashtable_load(FILE *fInput, struct Hashtable *table, void*(*load_func)(FILE*), unsigned long (*hash_func)(void *), int(*equal_func)(void*, void*))
{
	unsigned long size, i;
	int flag;
	struct Item *newItem;

	fread(&size, sizeof(unsigned long), 1, fInput);
	table->head = (struct Item **)malloc(sizeof(struct Item*)*size);
	table->size = size;
	for(i = 0; i < size; i++)
		(table->head)[i] = NULL;
	table->count = 0;
	table->entries = 0;
	table->hash_func = hash_func;
	table->equal_func = equal_func;

	for(i = 0; i < size; i++) {
		fread(&flag, sizeof(int), 1, fInput);
		while(flag) {
			newItem = (struct Item*)malloc(sizeof(struct Item));
			newItem->datap = load_func(fInput);
			table->count ++;
			if(table->head[i] == NULL) {
				table->head[i] = newItem;
				newItem->prev = newItem;
				newItem->next = NULL;
				table->entries ++;
			} else {
				table->head[i]->prev->next = newItem;
				newItem->prev = table->head[i]->prev;
				table->head[i]->prev = newItem;
				newItem->next = NULL;
			}
			fread(&flag, sizeof(int), 1, fInput);
		}
	}
}


int is_hashtable_empty(struct Hashtable *table)
{
	return !table->count;
}

/*
 * Binary Heap
 */
int position_has_name(char *name, struct Position *aPosition)
{
	return are_strings_equal(name, aPosition->name);
}


void binaryHeap_init(struct BinaryHeap *heap, int size, int(*compare_func)(void*, void*), void(*get_key_from_entry)(char*, void*))
{
	unsigned long i;

	if(heap == NULL) 
		return;
	heap->head = (void **)malloc(sizeof(void*)*size);
	if(heap->head == NULL) {
		printf("No more memory to alloc memory for Binary Heap!\n");
		return;
	}
	heap->size = size;
	for(i = 0; i < size; i++)
		(heap->head)[i] = NULL;
	heap->nEntries = 0;
	heap->compare_func = compare_func;
	heap->get_key_from_entry = get_key_from_entry;

	hashtable_init(&heap->posTable, size, (unsigned long(*)(void*))sdbm, (int(*)(void*,void*))position_has_name);
}

void binaryHeap_destroy(struct BinaryHeap *heap, void(*free_func)(void *))
{
	unsigned long i;

	if(heap == NULL) {
		return; 
	}
	for(i = 0; i < heap->size; i++) {
		if(heap->head[i] == NULL)
			continue;
		else {
			if(free_func)
				free_func(heap->head[i]);
      		}
	}
	hashtable_destroy(&heap->posTable, free);
	free(heap->head);
}

void binaryHeap_add(struct BinaryHeap *heap, void *entry)
{
	unsigned long m;
	struct Position *newPos;
	struct Item *aItem;
	void *temp;

	if(heap->nEntries + 1 > heap->size) {
		printf("Binary heap overfloats!\n");
		return;
	}

	heap->nEntries ++;
	heap->head[heap->nEntries-1] = entry;
	heap->get_key_from_entry(heap->key, entry);
	newPos = (struct Position*)malloc(sizeof(struct Position));
	newPos->index = heap->nEntries-1;
	sprintf(newPos->name, "%s", heap->key);
	hashtable_add(&heap->posTable, heap->key, newPos);

	m = heap->nEntries;
	while(m != 1) {
		if (heap->compare_func(heap->head[m - 1], heap->head[m/2 - 1])) {
			temp = heap->head[m - 1];
			heap->head[m - 1] = heap->head[m/2 - 1];
			heap->head[m/2 - 1] = temp;
			heap->get_key_from_entry(heap->key, heap->head[m-1]);
			aItem = hashtable_find(&heap->posTable, heap->key);
			((struct Position*)aItem->datap)->index = m - 1;
			heap->get_key_from_entry(heap->key, heap->head[m/2-1]);
			aItem = hashtable_find(&heap->posTable, heap->key);
			((struct Position*)aItem->datap)->index = m/2 - 1;
			m = m/2;
		} else
			break;
	}
}

void* binaryHeap_pick(struct BinaryHeap *heap)
{
	void *rtItem, *temp;
	unsigned long u, v;
	struct Item *aItem;

	rtItem = heap->head[0];
	heap->get_key_from_entry(heap->key, rtItem);
	free(hashtable_pick(&heap->posTable, heap->key));

	heap->head[0] = heap->head[heap->nEntries-1];
	heap->head[heap->nEntries-1] = NULL;
	heap->nEntries --;

	if(heap->nEntries > 0) {
		heap->get_key_from_entry(heap->key, heap->head[0]);
		aItem = hashtable_find(&heap->posTable, heap->key);
		((struct Position*)aItem->datap)->index = 0;
	} else
		return rtItem;

	v = 1;
	while(1) {
		u = v;
		if(2*u+1 <= heap->nEntries) {
			if(heap->compare_func(heap->head[2*u-1], heap->head[u-1]))
				v = 2*u;
			if(heap->compare_func(heap->head[2*u], heap->head[v-1]))
				v = 2*u+1;
		} else if(2*u <= heap->nEntries) {
			if(heap->compare_func(heap->head[2*u-1], heap->head[u-1]))
				v = 2*u;
		}
		if (u != v) {
			temp = heap->head[u-1];
			heap->head[u-1] = heap->head[v-1];
			heap->head[v-1] = temp;
			heap->get_key_from_entry(heap->key, heap->head[u-1]);
			aItem = hashtable_find(&heap->posTable, heap->key);
			((struct Position*)aItem->datap)->index = u - 1;
			heap->get_key_from_entry(heap->key, heap->head[v-1]);
			aItem = hashtable_find(&heap->posTable, heap->key);
			((struct Position*)aItem->datap)->index = v - 1;
		} else
			break;
	}
	return rtItem;
}

int is_entry_in_binaryHeap(struct BinaryHeap *heap, void *entry)
{
	heap->get_key_from_entry(heap->key, entry);
	if(hashtable_find(&heap->posTable, heap->key) == NULL)
		return 0;
	else
		return 1;
}

void binaryHeap_resort(struct BinaryHeap *heap, void *entry)
{
	unsigned long m;
	struct Item *aItem;
	void *temp;

	heap->get_key_from_entry(heap->key, entry);
	aItem = hashtable_find(&heap->posTable, heap->key);
	m = ((struct Position*)aItem->datap)->index + 1;
	while(m != 1) {
		if (heap->compare_func(heap->head[m - 1], heap->head[m/2 - 1])) {
			temp = heap->head[m - 1];
			heap->head[m - 1] = heap->head[m/2 - 1];
			heap->head[m/2 - 1] = temp;
			heap->get_key_from_entry(heap->key, heap->head[m-1]);
			aItem = hashtable_find(&heap->posTable, heap->key);
			((struct Position*)aItem->datap)->index = m - 1;
			heap->get_key_from_entry(heap->key, heap->head[m/2-1]);
			aItem = hashtable_find(&heap->posTable, heap->key);
			((struct Position*)aItem->datap)->index = m/2 - 1;
			m = m/2;
		} else
			break;
	}
}


int is_binaryHeap_empty(struct BinaryHeap *heap)
{
	return ! heap->nEntries;
}


/*
 * General tool functions
 */
int big2little(int number)
{
  union Int_byte aint;
  char abyte;

  aint.integer = number;
  abyte = aint.byte[0];
  aint.byte[0] = aint.byte[3];
  aint.byte[3] = abyte;
  abyte = aint.byte[1];
  aint.byte[1] = aint.byte[2];
  aint.byte[2] = abyte;
  return aint.integer;
}


void big2little_(void *number, int size)
{
	char *H, *T, byte;
	int i;
	
	H = (char*)number;
	T = H + size - 1;
	for (i = 0; i < size/2; i++) {
		byte = *H;
		*H = *T;
		*T = byte;
		H ++;
		T --;
	}
}
		

int equald(double a, double b, double delta)
{
  if ( ABS(a-b) <= delta ) 
    return 1;
  else 
    return 0;
}

int greaterd(double a, double b, double delta)
{
  if (a > b-delta)
	return 1;
  else 
	return 0;
}

int smallerd(double a, double b, double delta)
{
  if ( a< b+delta)
	return 1;
  else
	return 0;
}

int are_strings_equal(char* string1, char* string2)
{
	return !strcmp(string1, string2);
}

/* 
 * convert a "YYYY-MM-DD hh:mm:ss" time string to time_t format 
 * and vice versa
 */
time_t strtot(const char *timestr)
{
   char *p, buf[128];
   struct tm atm;
  
   if(timestr == NULL) 
	return 0; 
   strncpy(buf, timestr, 128);

   p = strtok(buf, "-");
   atm.tm_year = atoi(p) - 1900;
   p = strtok(NULL, "-");
   atm.tm_mon = atoi(p) - 1;
   p = strtok(NULL, " ");
   atm.tm_mday = atoi(p);
   p = strtok(NULL, ":");
   atm.tm_hour = atoi(p);
   p = strtok(NULL, ":");
   atm.tm_min = atoi(p);
   p = strtok(NULL, "\n\0");
   atm.tm_sec = atoi(p);
   atm.tm_isdst = 0;

   return mktime(&atm);
}


void ttostr(time_t timestamp, char *time)
{
	strftime(time, 20, "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
}

/*
 * General hash function, sdbm algorithm
 */
unsigned long sdbm(unsigned char *str)
{
  unsigned long hash = 0;
  int c;
  c = *str++;
  while (c) {
	hash = c + (hash << 6) + (hash << 16) - hash;
	c = *str++;
  }
  return hash;
}

int addr_equal_func(void *a, void*b)
{
	if(a == b) return 1;
	return 0;
}



