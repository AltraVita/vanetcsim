/* Notice :
*
* The set of Vanet programs take .ogd (original GPS data) files as input and may
* convert to .mgd (modified GPS data) files according to map matching
* algorithms. A specific data format convertor program should be provided to
* convert any other data format to .ogd before these data can be further
* processed in Vanet programs. 
*/

/* .ogd file format specification
*   -------------------------------
*   file_format_indicator
*   ogd_record (bus, taxi)
*   ogd_record (bus, taxi)
*   .....
*   
*
*   file_format_indicator
*   -------------------------------
*   FILE_ORIGINAL_GPS_TAXI  
*   FILE_ORIGINAL_GPS_BUS   
*
*   ogd_record taxi:
*   -------------------------------
*   taxiID, timestamp, longitude, lattitude, speed, heading, status
*
*   ogd_record bus:
*   busID, timestamp, longitude, lattitude, speed, heading, routeId, msyType, status, errorcode, routeLeng, gasVol 
*   
*/ 

/* .mgd file format specification
*   -------------------------------
*   file_format_indicator
*   mgd_record (bus, taxi)
*   mgd_record (bus, taxi)
*   .....
*   
*
*   file_format_indicator
*   -------------------------------
*   FILE_MODIFIED_GPS_TAXI  
*   FILE_MODIFIED_GPS_BUS   
*
*   mgd_record taxi:
*   -------------------------------
*   taxiID, timestamp, longitude, lattitude, speed, heading, status, onRoadID
*
*   mgd_record bus:
*   busID, timestamp, longitude, lattitude, speed, heading, routeId, msyType, status, errorcode, routeLeng, gasVol, onRoadID 
*   
*/ 

#ifndef TRACE_H
#define TRACE_H

#include <time.h>
#include "common.h"
#include "geometry.h"
#include "color.h"

#define TAXI_MAX_SPEED 120
#define BUS_MAX_SPEED 80

#define MIXED_TRACE_THRESHOLD 1000

#define TRACE_STATE_LIGHT 0

#define MAX_DISTANCE_ERROR 100
#define MAX_ANGLE_ERROR 90

#define HEADING_SPEED_THRESHOLD 10.8

#define INSERT_MODE_AVGSPEED 0
#define INSERT_MODE_TRAFFIC 1
#define OUTPUT_MODE_CELL 0
#define OUTPUT_MODE_INTERVAL 1 

/* the miminum time unit in seconds */
#define MINIMUM_TIME_UNIT 1

struct Report 
{
  time_t timestamp;
  struct Point gPoint;
  short speed;
  short angle;
  char state;

  /* for bus start->*/
  char msgType;
  double routeLeng;
  double gasVol;
  char errorInfo;
  /* <-end */

  char shown; 

  struct Duallist candRoads;
  struct Item *onRoad;
  int onRoadId;
  /* on the path determined by amend_algorithm */
  struct Path *onPath;

  struct Trace* fromTrace;
};


struct Trace
{
  char vName[NAME_LENGTH];
  char type;
  struct Duallist reports;
  struct Item *at;

  /*the duration of the trace*/
  time_t startAt;
  time_t endAt;

  struct Box box; 

  /* for bus start-> */
  char onRoute[NAME_LENGTH];
  /* <-end */
 
  double maxSpeed;
  double countdown;
  char isHeadingValid;

  union Int_RGBO color;
  /* for multiple purposes */
  double var1;
  double var2;
  double var3;

	//  add by fwu, for Mapmathing, 
	//  which means that if every ogd in this trace is added by 
	//  (transVec.x, transVec.y), it will be right on the road.
	struct Duallist transVecHistory;
};

void report_init_func(struct Report *aRep);
void report_free_func(struct Report *aRep);
int report_has_earlier_timestamp_than(struct Report* aRep, struct Report *bRep);
int report_has_later_timestamp_than(struct Report* aRep, struct Report *bRep);
int are_two_reports_duplicated(struct Report *aRep, struct Report *bRep);
void dump_report(FILE *fp, struct Report* aRep, int type);
int remove_dull_reports(struct Trace *aTrace);
int is_report_in_upway(struct Report *aRep);

void trace_init_func(struct Trace *aTrace);
void trace_free_func(struct Trace *aTrace);
int trace_has_name(char *name, struct Trace* aTrace);
int is_trace_mixed(struct Trace *aTrace);
void set_trace_table_time(struct Hashtable *traces, time_t atClock);
void set_selected_traces_time(struct Duallist *selectedTraces, time_t atClock);
void trace_dump_func(FILE *fp, struct Trace *aTrace);
struct Trace* load_trace_with_hashtable(int magicNumber, FILE *ftrace, struct Region *aRegion, struct Hashtable *traces, time_t *startAt, time_t *endAt, struct Box *box);

struct Trace* insert_reports(struct Region *aRegion, struct Trace *aTrace, int insertMode, int outputMode, int interval);
struct Duallist *insert_reports_in_path(struct Path *aPath, struct Report *aRep, struct Report* nextRep, int insertMode);

//added by fwu, set and get the transVec of the trace.
void set_trace_transVec(struct Trace* aTrace, double x, double y);
struct Point*get_trace_transVec(struct Trace*);

//added by fwu, dump the translation vector history
void trace_dump_transVecHistory(FILE*, struct Trace* );
void dump_transVec(FILE*, void*);

struct CandRoad
{
  struct Road *aRoad;
  double distance;
  double angle;
  double weight;
  struct Point gPoint;
  struct Segment onSegment;
};

int candroad_has_road(struct Road *aRoad, struct CandRoad *aCandRoad);
void add_candidate_roads(struct Cell *aCell, struct Report *aRep, int angleValid);
void add_candidate_roads_from_modified_point(struct Cell *aCell, struct Report *aRep, int angleValid, struct Point* modifiedPoint);
int candroad_has_smaller_weight_than(struct CandRoad *aCandRoad, struct CandRoad* bCandRoad);

struct Path *get_path_between_two_reports(struct Region *aRegion, struct Report *aRep, struct Report *bRep, double lengthreshold);


#endif
