#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <stdio.h>

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#define NOTSET -12345

#define NAME_LENGTH 32

#define VEHICLE_TYPE_TAXI 1 
#define VEHICLE_TYPE_BUS 2
#define VEHICLE_TYPE_NULL 3 

/* data structure definition */
union Int_byte
{
  unsigned long integer;
  char byte[4];
};

struct Int2
{
  unsigned long int1;
  unsigned long int2;
};

union Int2_byte
{
  struct Int2 a2Int;
  char byte[8];
};

/* anything dynamic to draw */
struct Display 
{
  void *party;
  time_t timestamp;
  char shown;
  double value;
};

/*
 *  A general item, with two-direction pointers
 */
struct Item
{
  void *datap;
  struct Item *prev;
  struct Item *next;
};

/* dual list, the next pointer is NULL when 
 * no more items at the tail of the list; the 
 * prev pointer of the first item points to 
 * the last item in the list.
 */
struct Duallist
{
  struct Item *head;
  unsigned long nItems;
};

void duallist_init(struct Duallist *duallist);
void duallist_destroy(struct Duallist *duallist, void (*free_func)(void*));
void duallist_dump(FILE *fOutput, struct Duallist *duallist, void(*dump_func)(FILE*, void*));
void duallist_load(FILE *fInput, struct Duallist *duallist, void*(*load_func)(FILE*));
struct Item* duallist_add_to_head(struct Duallist *duallist, void *data);
struct Item* duallist_add_to_tail(struct Duallist *duallist, void *data);
struct Item* duallist_add_in_sequence_from_head(struct Duallist *duallist, void* data, int(*sort_func)(void*, void*));
struct Item* duallist_add_in_sequence_from_tail(struct Duallist *duallist, void* data, int(*sort_func)(void*, void*));
struct Item* duallist_find(struct Duallist *duallist, void *key, int(*judge_func)(void*, void*));
void* duallist_pick(struct Duallist *duallist, void *key, int(*judge_func)(void*, void*));
void* duallist_pick_item(struct Duallist *duallist, struct Item *theItem);
void* duallist_pick_head(struct Duallist *duallist);
void* duallist_pick_tail(struct Duallist *duallist);
int is_duallist_empty(struct Duallist *duallist);
struct Duallist* duallist_copy_by_reference(struct Duallist *destDuallist, struct Duallist *aDaullist);
struct Duallist* duallist_copy(struct Duallist *destDuallist, struct Duallist *aDuallist, void*(*copy_func)(void*));
struct Duallist* duallist_reverse_copy(struct Duallist *destDuallist, struct Duallist *aDuallist, void*(*copy_func)(void*));
unsigned long distance_to_tail(struct Item *aItem);
unsigned long distance_to_head(struct Duallist *aDuallist, struct Item *aItem);
void curtain_destroy(struct Duallist *duallist, void(*free_func)(void*));
int is_sublist(struct Duallist *aList, struct Duallist *bList, int(*equal_func)(void*, void*));
void duallist_remove_loops(struct Duallist *aList, int(*equal_func)(void*,void*), void(*free_func)(void*));

/*
 * Stack implementation
 */
void stack_init(struct Duallist* stack);
struct Item* stack_push(struct Duallist* stack, void *data);
void* stack_pop(struct Duallist* stack);
int is_stack_empty(struct Duallist* stack);

/*
 * Queue implementation
 */
void queue_init(struct Duallist* queue);
struct Item* queue_add(struct Duallist*queue, void* data);
void* queue_pick(struct Duallist *queue);
int is_queue_empty(struct Duallist *queue);

/*
 * Data structure and operations for hash table
 */
struct Hashtable
{
  struct Item **head;
  /* the size of the table */
  unsigned long size;
  /*how many table entries have been filled */
  unsigned long entries;
  /*how many items in the tabel in total */
  unsigned long count;

  unsigned long (*hash_func)(void *);
  int(*equal_func)(void*, void*);
};

void hashtable_init(struct Hashtable *table, unsigned long size, unsigned long (*hash_func)(void *), int(*equal_func)(void*, void*));
void hashtable_destroy(struct Hashtable *table, void(*free_func)(void*));
struct Item* hashtable_add(struct Hashtable *table, void *key, void *data);
struct Item* hashtable_add_unique(struct Hashtable *table, void *key, void *data);
struct Item* hashtable_find(struct Hashtable *table, void*key);
struct Item* hashtable_next_item(struct Hashtable *table, struct Item *aItem);
void* hashtable_pick(struct Hashtable *table, void*key);
void hashtable_dump(FILE *fOutput, struct Hashtable *table, void(*dump_func)(FILE*, void*));
void hashtable_load(FILE *fInput, struct Hashtable *table, void*(*load_func)(FILE*), unsigned long (*hash_func)(void *), int(*equal_func)(void*, void*));
int is_hashtable_empty(struct Hashtable *table);

/*
 * Binary heap
 */
struct Position
{
  char name[128];
  long index;
};
int position_has_name(char *name, struct Position *aPosition);

struct BinaryHeap
{
  void **head;
  struct Hashtable posTable;
  unsigned long size;
  unsigned long nEntries;
  char key[128];
  int(*compare_func)(void*, void*);
  void(*get_key_from_entry)(char*, void*);
};  
void binaryHeap_init(struct BinaryHeap *heap, int size, int(*compare_func)(void*, void*), void(*get_key_from_entry)(char*, void*));
void binaryHeap_destroy(struct BinaryHeap *heap, void(*free_func)(void *));
void binaryHeap_add(struct BinaryHeap *heap, void *entry);
void* binaryHeap_pick(struct BinaryHeap *heap);
void binaryHeap_resort(struct BinaryHeap *heap, void *entry);
int is_entry_in_binaryHeap(struct BinaryHeap *heap, void *entry);
int is_binaryHeap_empty(struct BinaryHeap *heap);
/*
 * General tool functions
 */
int big2little(int);
void big2little_(void *number, int size);

int equald(double a, double b, double delta);
int greaterd(double a, double b, double delta);
int smallerd(double a, double b, double delta);

int are_strings_equal(char* string1, char* string2);

/* 
 * convert a "YYYY-MM-DD hh:mm:ss" time string to time_t format 
 * and vice versa
 */
time_t strtot(const char *timestr);
void ttostr(time_t timestamp, char *time);

/*
 * General hash function, sdbm algorithm
 */
unsigned long sdbm(unsigned char *str);
int addr_equal_func(void *a, void*b);

#endif
