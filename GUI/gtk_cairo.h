#ifndef GTK_CAIRO_H
#define GTK_CAIRO_H

#include <gtk/gtk.h>
#include <time.h>
#include "geometry.h"
#include "contact.h"
#include "common.h"
#include "color.h"
#include "busroute.h"
#include "node.h"

#define NUM_OF_ROUTES 200
#define NUM_OF_STORAGE 2000
#define NUM_OF_TRACES 2000
#define NUM_OF_CONTACTPAIRS 500000
/* one meter on the ground */
#define MARGIN 10 

#define MARK_LENGTH 12 
#define CONTACT_RADIUS 2 
#define FONT_SIZE 12

struct WindowSize
{
  /* coordinate window size*/
  struct Box cBox;

  /* screen window size*/
  struct Box sBox;

  double scale;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// screen display >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct ScreenContext
{
  struct Box box;


  struct Region *region;
  struct Duallist roads;
  struct Duallist crosses;	
  struct Duallist districts;
  struct Duallist rivers;
  struct Duallist surroundings;

  double scale;
  double meterperpixel;
  
  /*RSU topology*/
  struct Region *vRegion;
  struct Duallist rsus;
  struct Duallist links;

  struct WindowSize awin;

  /*debug-mode display*/
  int debug;
 
  /* which area the mouse is locating in */
  int areaOnScreen;
  /* the mouse is currently on which road or on empty place (NULL)*/
  struct Road * mouseAtRoad;
  /* the mouse is currently on which cross or on empty place (NULL)*/
  struct Cross * mouseAtCross;
  /* the mouse is currently on which cell or on empty place (NULL)*/
  struct Cell * mouseAtCell;
  /* the mouse is currently on which log or on empty place (NULL) */
  struct Report * mouseAtReport;

  /* place to draw everything on */
  GdkPixmap *canvas;
  int firstShown;
  /* cairo draw context */
  cairo_t * cr_on_canvas;
  cairo_t * cr_on_screen;

  GtkWidget *drawingArea;
  GtkWidget *backwardButton;
  GtkWidget *forwardButton;
  GtkWidget *linkmanListSW;

  /* gtk context */
  GdkGC *gc;
  /* where the screen's origin */
  int scr_x;
  int scr_y;
  int scr_width;
  int scr_height;

  /* for play data */
  int playspeed;
  int firstplay;
  guint timeout;
  time_t startAt;
  time_t endAt;
  time_t atClock;

  struct Colormap *colormap;

  /* bus route table */
  struct Hashtable routeTable;
  struct Duallist selectedRoutes;

  /* static storage node */
  struct Hashtable nodeTable;

  /* play GPS reports */
  struct Hashtable traceTable;
  struct Duallist selectedTraces;
  struct Trace *focusTrace;

  /* play contacts */
  struct Hashtable contactTable;
  int contactTableMode;
  struct Duallist selected;

  /*play cell dynamic */
  struct Hashtable cellTable;

  /* input point*/
  struct Point iPoint;
};

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  

enum 
{
  COLUMN_TODRAW,
  COLUMN_NAME,
  NUM_COLUMNS
};

enum
{
  COLUMN_RD_NAME,
  NUM_RD_COLUMNS
};


/*
*
* Drawing operations
*
********************************************************************/
void set_background_color(GdkGC *gc, int red, int green, int blue);
void set_foreground_color(GdkGC *gc, int red, int green, int blue);
void rub_bar(GdkDrawable *drawable, GdkGC *gc, int barWidth);
void brush_bar(GdkDrawable *drawable, GdkGC *gc, int barWidth, int no);

void init_screen_context(struct ScreenContext *screenContext);
void setup_drawing_context(struct ScreenContext *screenContext);
void setup_drawing_content(struct ScreenContext *screenContext, int scr_x, int scr_y, int scr_width, int scr_height, struct WindowSize *awin);
void destroy_screen_context( struct ScreenContext *screenContext);

void draw_canvas(struct ScreenContext *screenContext);
void draw_dynamic_on_screen(struct ScreenContext *screenContext);
void draw_districts(struct ScreenContext *screenContext);
void draw_rivers(struct ScreenContext *screenContext);
void rubber_cells(struct ScreenContext *screenContext);
void draw_cells(struct ScreenContext *screenContext, cairo_t *cr);
void draw_cell_display(struct ScreenContext *screenContext, cairo_t *cr, struct Cell *aCell);
void draw_roads(struct ScreenContext *screenContext);
void draw_links(struct ScreenContext *screenContext);
void draw_crosses(struct ScreenContext *screenContext);
void draw_rsus(struct ScreenContext *screenContext);
void draw_scale(struct ScreenContext *screenContext);
void draw_selected_routes(struct ScreenContext *screenContext, cairo_t *cr);

void draw_displayed_traces(struct ScreenContext *screenContext, cairo_t*cr);
void draw_displayed_reports(struct ScreenContext *screenContext, cairo_t *cr, struct Trace *aTrace);

void draw_displayed_contacts(struct ScreenContext *screenContext, cairo_t *cr);
void draw_displayed_contacts_of_a_pair(struct ScreenContext *screenContext, cairo_t *cr, struct Pair *aPair);
void draw_displayed_contacts_of_an_ego(struct ScreenContext *screenContext, cairo_t *cr, struct Ego *anEgo);

void draw_single_road(struct ScreenContext *screenContext, cairo_t *cr, struct Road *aRoad, struct RGBO *fillColor);
void draw_single_cross(struct ScreenContext *screenContext, cairo_t *cr, struct Cross *aCroass, struct RGBO *color);
void draw_single_cell(struct ScreenContext *screenContext, cairo_t *cr, struct Cell *aCell, struct RGBO *color);
void draw_single_route(struct ScreenContext *screenContext, cairo_t *cr, struct Busroute *aRoute);
void draw_single_report(struct ScreenContext *screenContext, cairo_t *cr, struct Report *aReport, struct RGBO *color);
void draw_single_contact(struct ScreenContext *screenContext, cairo_t *cr, struct Contact *aContact, struct RGBO *color);
void draw_single_storage_node(struct ScreenContext *screenContext, cairo_t *cr, struct Node *aNode, struct RGBO *color);
void rubber_single_report(struct ScreenContext *screenContext, cairo_t *cr, struct Report *aReport);
void show_district(struct ScreenContext *screenContext, struct District *aDistrict, cairo_t *cr);

void gps_to_canvas(struct WindowSize* awnd, double x, double y, double *rx, double *ry);
double canvas_to_gps(struct WindowSize* awnd, double x, double y, double *rx, double *ry);
void show_text(struct ScreenContext *screenContext, char* text, int x, int y, int size);
void assign_window_size(struct WindowSize *target, struct WindowSize *source);


struct Colormap* load_colormap(char *filename);
void free_colormap(struct Colormap *cmap);
void get_color(struct Colormap *cmap, struct Color *rtColor, double value, double lower, double upper);

#endif
