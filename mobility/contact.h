#ifndef CONTACT_H
#define CONTACT_H

#include<time.h>
#include"geometry.h"
#include"common.h"
#include"trace.h"

#define DEFAULT_NUMBER_OF_SLIDES 10
/* How to determine a contact in terms of spatial and temporal spacing */
#define DEFAULT_MEETING_SPATIAL_GRAN 100
#define DEFAULT_MEETING_TEMPORAL_GRAN 60
#define DEFAULT_MEETING_COUNTING_GRAN 1
#define DEFAULT_DAILY_MEETING_GRAN 1

#define DEFAULT_CROSS_RANGE 50

#define PAIRWISE_TABLE 0
#define EGO_TABLE 1


struct Single
{
  int t1;

  long count;
};

struct Couple
{
  int t1;
  int t2;

  long count;
};

struct Triple
{
  int t1;
  int t2;
  int t3;

  long count;
};

struct ICT
{
  time_t timestamp;
  unsigned long ict;
};

double vector_entropy(unsigned long *vector, int nItems);
double vectors_joint_entropy(unsigned long *vector1, unsigned long *vector2, int nItems);
int single_has_name(char *names, struct Single *aSingle);
int couple_has_names(char *names, struct Couple *aCouple);
int triple_has_names(char *names, struct Triple *aTriple);

struct ContactSample
{
  time_t timestamp;
  struct Point gPoint1;
  struct Point gPoint2;
  double distance;
  int rAngle;
  int rSpeed;
  struct Pair *fromPair;
};
int is_earlier_than_contact_sample(time_t *timestamp, struct ContactSample *aContactSample);
int are_contact_samples_duplicated(struct ContactSample *aContactSample, struct ContactSample *bContactSample);
int are_contact_samples_continuous(struct ContactSample *aContactSample, struct ContactSample *bContactSample, int tGran, int sGran);

struct Contact
{
  time_t startAt;
  time_t endAt;
  struct Point gPoint;

  /* which cell*/
  int xNumber;
  int yNumber;

  char shown;
  struct Pair *fromPair;
};
int is_earlier_than_contact(time_t *timestamp, struct Contact *aContact);
struct Contact *contact_copy_func(struct Contact *aContact);


struct Pair 
{
  char vName1[2*NAME_LENGTH];
  char vName2[2*NAME_LENGTH];

  struct Duallist contents;
  struct Item *at;

  time_t startAt;
  time_t endAt;

  double countdown; 
  union Int_RGBO color;
};

void pair_init_func(struct Pair *aPair);
void pair_free_func(struct Pair *aPair);
int pair_has_names(char *names, struct Pair *aPair);
void dump_contact_sample_pair(struct Pair *aPair, FILE *fdump);
void dump_contact_pair(struct Pair *aPair, FILE *fdump);

struct Ego
{
  char vName[2*NAME_LENGTH];
  struct Duallist linkmen;
  time_t startAt;
  time_t endAt;
};
void ego_init_func(struct Ego *anEgo);
void ego_free_func(struct Ego *anEgo);
int ego_has_name(char* name, struct Ego *anEgo);

struct Linkman
{
  char vName[2*NAME_LENGTH];
  union Int_RGBO color;
  struct Duallist contacts;
  struct Item *at;
  double countdown; 
};
void linkman_init_func(struct Linkman *aLinkman);
void linkman_free_func(struct Linkman *aLinkman);
int linkman_has_name(char* name, struct Linkman *aLinkman);


struct ContactContext
{
  time_t startAt;
  time_t endAt;
  time_t clock; 

  int type1;
  int type2;
 
  int tGran;
  int sGran;

  int numSlots;
  int crossRange;

  int currentPos;
  int newestPos;
};

void get_cells_ready_for_trace(struct Region *region, int slotNumber);
void clear_cells_from_trace(struct Region *region, int slotNumber);

void retrieve_ict(struct Hashtable *cntTable, struct Hashtable *ictTable);
void dump_icts(struct Hashtable *pairIctTable, FILE *fdump);
void dump_ict_pair(struct Pair *aPair, FILE *fdump);
void load_icts_with_hashtable(FILE *fsource, struct Hashtable *table, time_t *startAt, time_t *endAt);

void mount_traces(struct Region *region, struct Hashtable *traces, struct ContactContext *context);
void mount_traces_with_seperate_slots(struct Hashtable *traces, struct ContactContext *context, struct Duallist *slots);
void free_used_reports(struct Hashtable *traces);

void mount_traces_at_slot(struct Region *region, struct Hashtable *traces, struct ContactContext *context, int position);
void mount_traces_at_seperate_slot(struct Hashtable *traces, struct ContactContext *context, struct Duallist *slots, int position);

int are_two_reps_meeting(struct Report *thisRep, struct Report *thatRep, int tGran, int sGran);


void find_contact_samples(struct Region *region, struct Hashtable *pairSmpTable, struct ContactContext *context);
void find_contact_samples_with_seperate_slots(struct Hashtable *pairSmpTable, struct ContactContext *context, struct Duallist *slots);
void find_contact_samples_between_two_slots(struct Duallist *thisSlot, struct Duallist *thatSlot, struct Hashtable *pairSmpTable, time_t clock, struct ContactContext *context);

void dump_contact_samples(struct Hashtable *pairSmpTable, FILE *fdump);
void dump_contacts(struct Hashtable *pairContTable, FILE *fdump);

void load_contacts_with_hashtable(FILE *fsource, struct Region *aRegion, struct Hashtable *table, int mode, time_t *startAt, time_t *endAt);
void load_contact_samples_with_hashtable(FILE *fsource, struct Region *aRegion, struct Hashtable *table, time_t *startAt, time_t *endAt);

void set_traces_at_clock(struct Hashtable *traces, time_t atClock);
void set_pair_table_time(struct Hashtable *pairs, time_t atClock);
void set_selected_contacts_time(struct Duallist *selectedPairs, int mode, time_t atClock);

#endif
