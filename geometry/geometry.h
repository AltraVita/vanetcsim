#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdio.h>
#include "common.h"
#include "shp.h"
#include "color.h"
#include "pkg.h"

#define DELTA 1e-7
// precision in meters
#define PRECISION 1

#define DIRECTION_FORWARD 1
#define DIRECTION_REVERSE 2
#define DIRECTION_DUAL 0

#define ROAD_WIDTH 10
#define CROSS_WIDTH 20 

#define RADIUS_A 6378140
#define RADIUS_B 6356755

#define MAXIMUM_SEARCH_COUNT 1000
#define MAXIMUM_SELECTION 1000

#define MAX_PERCEPTION_DISTANCE 10000

#define DEFAULT_CELLSIZE 150
#define DEFAULT_SCRAPLENGTH 0

#undef M_PI
#define M_PI 3.1415926

struct Segment
{
  struct Point aPoint;
  struct Point bPoint;
};

struct Box
{
  double xmin;
  double ymin;
  double xmax;
  double ymax;
};

int point_equal_func(struct Point *aPoint, struct Point *bPoint);
void point_free_func(struct Point *aPoint);
void point_dump_func(FILE *fOutput, struct Point *aPoint);
struct Point* point_load_func(FILE *fInput);
struct Point* point_copy_func(struct Point *aPoint);

int polyline_equal_func(struct Duallist *aPolyline, struct Duallist *bPolyline);
void polyline_free_func(struct Duallist *aPolyline);
void polyline_dump_func(FILE *fOutput, struct Duallist *aPolyline);
struct Duallist* polyline_load_func(FILE *fInput);

struct Sample
{
  int speed;
};

void sample_free_func(struct Sample *aSample);

struct Slide
{
  struct Duallist samples;
  double condition;
};

void slide_free_func(struct Slide *aSlide);

struct Scrap
{
  struct Point gPoint;
  struct Duallist points;
  struct Box box;
  double value;

  struct Duallist slides;
};

void scrap_free_func(struct Scrap *aScrap);
void scrap_dump_func(FILE *fOutput, struct Scrap *aScrap);
struct Scrap* scrap_load_func(FILE *fInput);


struct Cross
{
  int number;
  struct Point gPoint;
  struct Duallist points;

  struct Box box;

  struct Duallist inRoads;
  struct Duallist outRoads;

  struct Duallist refs;
  double range;

  /* path finding */
  struct Cross *fromCross;
  int checked;
  double pastCost;
  double basicCost;

  /* chosen count */
  unsigned long count;
};

void get_cross_name(char *name, struct Cross *aCross);
int cheaper_cross(struct Cross *aCross, struct Cross *bCross);
int cross_equal_func(struct Cross *aCross, struct Cross *bCross);
void cross_init_func(struct Cross *aCross);
void cross_free_func(struct Cross *aCross);
int cross_has_number(int* id, struct Cross *aCross);
int cross_has_location(struct Point *gPoint, struct Cross *aCross);
void cross_dump_func(FILE *fOutput, struct Cross *aCross);
struct Cross* cross_load_func(FILE *fInput);
struct Cross* cross_copy_func(struct Cross* aCross);


struct Road
{
  int id;
  char name[NAME_LENGTH];
  double density;

  struct Duallist origPoints;
  struct Duallist points;

  /*geographic box*/
  struct Box box;

  double width;
  double length;

  /* may use for multiple purposes */
  int value;
  /* average error of gps data on this road*/
  double error;
  int samples;

  /*hops from center road*/
  int hops;

  int headEndAngle;
  int tailEndAngle;
  struct Cross *headEnd;
  struct Cross *tailEnd;

  struct Duallist refs;
  struct Duallist scraps;
  double nSamples;
};

int road_equal_func(struct Road *aRoad, struct Road *bRoad);
int road_has_id(int* id, struct Road *aRoad);
void road_init_func(struct Road *aRoad);
void road_free_func(struct Road *aRoad);
void road_dump_func(FILE *fOutput, struct Road *aRoad);
struct Road* road_load_func(FILE *fInput);
struct Road* road_copy_func(struct Road* aRoad);


struct Ref
{
  struct Duallist *byWho;
  struct Item *at;
};

void ref_free_func(struct Ref *aRef);


struct District
{
  struct Box box;
  struct Duallist rings;
};
void district_free_func(struct District *aDistrict);
void district_dump_func(FILE *fOutput, struct District *aDistrict);
struct District* district_load_func(FILE *fInput);

struct River
{
  struct Box box;
  struct Duallist rings;
};
void river_free_func(struct River *aRiver);
void river_dump_func(FILE *fOutput, struct River *aRiver);
struct River* river_load_func(FILE *fInput);

struct Path
{
  struct Duallist roads;
  double length;
  int turns;
};
void path_free_func(struct Path *aPath);
int is_path_within_box(struct Path *aPath, struct Box *aBox);
void path_init_func(struct Path *aPath);


struct Cell
{
  int xNumber;
  int yNumber;
  struct Box box;
  struct Duallist roads; 
  struct Duallist crosses;
  /* reports in a row */
  struct Duallist reps;
  /* cycle buffer for mounting reports*/
  struct Duallist* slots;

  /* bus routes */
  struct Duallist routes;
  /* bus stops */
  struct Duallist stops;
  /* storage node */
  struct Node *storage;

  /* count the number of object of interest */
  unsigned long n1;
  unsigned long n2;
  unsigned long n3;
  unsigned long n4;
  unsigned long n5;
  unsigned long n6;
  /* display anything dynamic */
  struct Duallist displays;
  struct Item *at;
  double countdown; 
};
int cell_equal_func(struct Cell *aCell, struct Cell *bCell);
int cell_has_nos(char *nos, struct Cell *aCell);
void cell_init_func(struct Cell *aCell);
void cell_free_func(struct Cell *aCell);
int cell_has_less_routes_than(struct Cell *aCell, struct Cell *bCell);

struct Polygon
{
  struct Box box;
  struct Duallist points;
  struct Item *currentPoint;
  struct Item *mouseAt;
  double scale;
  int state;
};

int is_legal(struct Polygon *chosen_polygon, struct Point *aPoint);
int is_polygon(struct Polygon *chosen_polygon, struct Point *aPoint);
void build_polygon(struct Polygon ** chosen_polygon, struct Point *aPoint);
void close_polygon(struct Polygon *chosen_polygon);
void polygon_free_func(struct Polygon * chosen_polygon);
void polygon_dump_func(FILE *fOutput, struct Polygon *aPolygon);
struct Polygon* polygon_load_func(FILE *fInput);
struct Polygon* polygon_copy_func(struct Polygon *aPolygon);


struct Region
{
  struct Polygon *chosen_polygon;

  struct Duallist roads;
  int roadNums;
  struct Duallist crosses;
  int crossNums;
  struct Duallist districts;
  struct Duallist rivers;

  double cellSize;
  unsigned long vCells;
  unsigned long hCells;
  struct Cell *mesh;
  char *map;

  // bus coverage
  struct Duallist busCoveredCells;

  double scrapLength;

  double commRange;

  /* maximum degree*/
  int maxdgr;

  struct Colormap cmap;
  /*min and max value of certain metric*/
  double lower;
  double upper;
};
struct Region* build_geographical_region(char *districtFile, char *riverFile, char *roadFile, char *roadAttrFile, struct Polygon *chosen_polygon, double cellSize, double scrapLength);
void region_free_func( struct Region *aRegion);
void region_dump_func(FILE *fOutput, struct Region *region);
struct Region* region_load_func(FILE *fInput, struct Polygon *chosen_polygon, double cellSize, double scrapLength);
void edit_region(FILE *fedit, struct Region *aRegion);
struct Region* build_region_with_roads(struct Duallist *roads);
struct Region* region_copy_func(struct Region *aRegion);
void setup_cells_in_region(struct Region *region);
void setup_roads_and_crosses_in_region(struct Region *region);
void check_max_degree(struct Region *region);

void read_all_road_records(FILE *froad, FILE *froadAttr, struct Region *region);
void add_road_rcd_to_region(struct Polyline_record *rcd, struct Region *region);
void setup_road (struct Road *newRoad, struct Region *region);
void link_road(struct Road *newRoad, struct Duallist *roadList);
void link_cross(struct Cross *newCross, struct Duallist *crossList);
void offset_right_road(struct Duallist *original, double width, struct Duallist *polyline);
void offset_right(struct Segment *original, double offset, struct Segment *rnSegment);
void refine_roadnetwork(struct Region *region);


void read_all_district_records(FILE *fdist, struct Region *region);
void add_district_rcd_to_region(struct Polygon_record *pgrcd, struct Region *region);

void read_all_river_records(FILE *friver, struct Region *region);
void add_river_rcd_to_region(struct PolygonM_record *pgmrcd, struct Region *region);


struct District *point_in_district(struct Region *region, struct Point *aPoint);

double polyline_length(struct Duallist *polyline);
void point_cut_polyline(const struct Duallist *aPolyline, struct Point *cutPoint, struct Duallist *aPart, struct Duallist *bPart);

void cells_on_line(struct Region *region, struct Point *point1, struct Point *point2, struct Duallist *cellList);
void load_cell_displays_with_hashtable(FILE *fsource, struct Region *region, struct Hashtable *cellTable, time_t *startAt, time_t *endAt);
struct Cell *point_in_cell(struct Region *region, struct Point *aPoint);
void set_cell_table_time(struct Hashtable *cellTable, time_t atClock);
void surroundings_from_point(struct Duallist *list, struct Region *region, struct Point *aPoint);
struct Cell* randomly_pick_a_bus_covered_cell(struct Region *region);


double angle_between(double x1, double y1, double x2, double y2);
double inter_angle(double angle1, double angle2);

void create_box(struct Point *aPoint, double length, struct Box *aBox);
void merge_boxes(struct Box *targetBox, struct Box *aBox, int firsttime);
int is_point_on_segment(struct Point *aPoint, struct Segment* aSegment);
int is_point_in_polygon(struct Point *aPoint, struct Polygon *chosen_polygon);
int is_polyline_in_polygon(struct Duallist *polyline, struct Polygon *chosen_polygon);
int is_point_in_box(struct Point *aPoint, struct Box *aBox);
int is_cell_in_polygon(struct Cell *aCell, struct Polygon *chosen_polygon);

int is_segment_cut_polyline(struct Segment *aSegment, struct Duallist *polyline, struct Point *cutPoint);
int are_segments_intersected(struct Segment *aSegment, struct Segment *bSegment);
void copy_segment(struct Segment *dest, struct Segment *source);
void segment_cut_segment(struct Segment *aSegment, struct Segment *bSegment, struct Point *aPoint, struct Point *bPoint);
int segment_equal_func(struct Segment *aSeg, struct Segment *bSeg);

int are_boxes_intersected(struct Box *aBox, struct Box *bBox);
int is_box_within_box(struct Box *aBox, struct Box *bBox);
double getRotateAngle(struct Segment *aSegment, struct Segment *bSegment);
void remove_loop(struct Duallist *polyline);

struct Path* find_shortest_path(struct Region* region, struct Cross* sCross, struct Cross* dCross);
struct Duallist* polyline_on_path(struct Path *aPath, struct Point *fromPoint, struct Point *toPoint);
int turns_on_path(struct Path *aPath);
struct Path* path_copy_func(struct Path *aPath);

double distance_in_meter(double lng1, double lat1, double lng2, double lat2);
double distance_in_pixel(double x1, double y1, double x2, double y2);
double distance_in_latitude(double dist);

double distance_point_to_segment(struct Point *aPoint, struct Segment*aSegment, struct Point *crossPoint);
double distance_point_to_polyline(struct Point *aPoint, const struct Duallist *polyline, struct Point *crossPoint, struct Segment* onSegment);

double distance_on_path(struct Path *aPath, struct Point *fromPoint, struct Point *toPoint);
double distance_to_tail_cross(struct Road *aRoad, struct Point *fromPoint);
double distance_to_head_cross(struct Road *aRoad, struct Point *fromPoint);

void remove_road(struct Region *aRegion, struct Road *aRoad);
void remove_cross(struct Region *aRegion, struct Cross *currentCross);
#endif
