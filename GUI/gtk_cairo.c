#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<gtk/gtk.h>
#include "trace.h"
#include "gtk_cairo.h"
#include "contact.h"

/*
*
*  Gdk drawing operations.
*
*************************************************************************
*************************************************************************/

void set_background_color(GdkGC *gc, int red, int green, int blue)
{
  GdkColormap * cmap = gdk_colormap_get_system();
  GdkColor color;
  color.red = red;
  color.green = green;
  color.blue = blue;
  if (gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE))
  { 
	gdk_gc_set_background(gc, &color);
  }
}

void set_foreground_color(GdkGC *gc, int red, int green, int blue)
{
  GdkColormap * cmap = gdk_colormap_get_system();
  GdkColor color;
  color.red = red;
  color.green = green;
  color.blue = blue;
  if (gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE))
  { 
	gdk_gc_set_foreground(gc, &color);
  }
}


void rub_bar(GdkDrawable *drawable, GdkGC *gc, int barWidth)
{
	int width, height;
	gdk_drawable_get_size(drawable, &width, &height);
	set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
	gdk_draw_rectangle(drawable, gc, TRUE, 0, 0, barWidth, height);
	gdk_draw_rectangle(drawable, gc, TRUE, 0, height-barWidth, width, barWidth);
	gdk_draw_rectangle(drawable, gc, TRUE, 0, 0, width, barWidth);
	gdk_draw_rectangle(drawable, gc, TRUE, width-barWidth,0, barWidth, height);
}



void brush_bar(GdkDrawable *drawable, GdkGC *gc, int barWidth, int no)
{
	int width, height;
	GdkPoint points[3];

	gdk_drawable_get_size(drawable, &width, &height);
	set_foreground_color(gc, 65535*222/255, 65535*217/255, 65535*202/255);
	switch (no) {
		case 1:
			gdk_draw_rectangle(drawable, gc, TRUE, 0, height/4, barWidth-1, height/2);
			points[0].x = 7, points[0].y = height/2-5;
			points[1].x = 2, points[1].y = height/2;
			points[2].x = 7, points[2].y = height/2+5;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
		case 2:
			gdk_draw_rectangle(drawable, gc, TRUE, 0, 3*height/4, barWidth-1, height/4-1);
			gdk_draw_rectangle(drawable, gc, TRUE, 0, height-barWidth+1, width/4, barWidth-1);
			points[0].x = 3, points[0].y = height-10;
			points[1].x = 3, points[1].y = height-3;
			points[2].x = 10, points[2].y = height-3;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
		case 3:
			gdk_draw_rectangle(drawable, gc, TRUE, width/4, height-barWidth+1, width/2-1, barWidth-1);
			points[0].x = width/2, points[0].y = height-2;
			points[1].x = width/2+5, points[1].y = height-7;
			points[2].x = width/2-5, points[2].y = height-7;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
		case 4:
			gdk_draw_rectangle(drawable, gc, TRUE, width*3/4, height-barWidth+1, width/4, barWidth-1);
			gdk_draw_rectangle(drawable, gc, TRUE, width-barWidth+1, height*3/4, barWidth-1, height/4);
			points[0].x = width-10, points[0].y = height-3;
			points[1].x = width-3, points[1].y = height-3;
			points[2].x = width-3, points[2].y = height-10;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
		case 5:
			gdk_draw_rectangle(drawable, gc, TRUE, width-barWidth+1, height/4, barWidth-1, height/2);
			points[0].x = width-7, points[0].y = height/2-5;
			points[1].x = width-2, points[1].y = height/2;
			points[2].x = width-7, points[2].y = height/2+5;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
		case 6:
			gdk_draw_rectangle(drawable, gc, TRUE, width*3/4, 0, width/4, barWidth-1);
			gdk_draw_rectangle(drawable, gc, TRUE, width-MARGIN+1, 0, barWidth-1, height/4);
			points[0].x = width-10, points[0].y = 3;
			points[1].x = width-3, points[1].y = 3;
			points[2].x = width-3, points[2].y = 10;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
		case 7:
			gdk_draw_rectangle(drawable, gc, TRUE, width/4, 0, width/2, barWidth-1);
			points[0].x = width/2, points[0].y = 2;
			points[1].x = width/2-5, points[1].y = 7;
			points[2].x = width/2+5, points[2].y = 7;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
		case 8:
			gdk_draw_rectangle(drawable, gc, TRUE, 0, 0, barWidth-1, height/4);
			gdk_draw_rectangle(drawable, gc, TRUE, 0, 0, width/4, barWidth-1);
			points[0].x = 3, points[0].y = 3;
			points[1].x = 3, points[1].y = 10;
			points[2].x = 10, points[2].y = 3;
			set_foreground_color(gc, 65535*255/255, 65535*255/255, 65535*255/255);
			gdk_draw_polygon(drawable, gc, TRUE, points, 3);
			break;
	}
}


void init_screen_context(struct ScreenContext *screenContext)
{
	duallist_init(&screenContext->roads);
	duallist_init(&screenContext->crosses);
	duallist_init(&screenContext->rsus);
	duallist_init(&screenContext->links);
	duallist_init(&screenContext->districts);
	duallist_init(&screenContext->rivers);
	hashtable_init(&screenContext->routeTable, NUM_OF_ROUTES, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))route_has_name);
	duallist_init(&(screenContext->selectedRoutes));
	hashtable_init(&screenContext->nodeTable, NUM_OF_STORAGE, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))node_has_name);
	hashtable_init(&screenContext->traceTable, NUM_OF_TRACES, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
	duallist_init(&(screenContext->selectedTraces));
	duallist_init(&(screenContext->selected));
	hashtable_init(&screenContext->cellTable, screenContext->region->vCells*screenContext->region->hCells, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))cell_has_nos);
	
	screenContext->debug = 0;
	screenContext->contactTableMode = PAIRWISE_TABLE;

	screenContext->colormap = NULL;
	screenContext->vRegion = NULL;

	screenContext->startAt = 0;
	screenContext->endAt = 0;
	screenContext->atClock = 0;
	screenContext->timeout = 0;
	screenContext->scale = 1;
	screenContext->firstplay = 1;
	screenContext->firstShown = 1;

	screenContext->focusTrace = NULL;

	screenContext->canvas = NULL;
	screenContext->gc = NULL;
}


void setup_drawing_context(struct ScreenContext *screenContext)
{
	if(screenContext->canvas) {
		g_object_unref(screenContext->canvas);
		cairo_destroy(screenContext->cr_on_canvas);
	}
  	if(screenContext->gc) gdk_gc_unref(screenContext->gc);
	
	screenContext->canvas = gdk_pixmap_new(screenContext->drawingArea->window, screenContext->drawingArea->allocation.width, screenContext->drawingArea->allocation.height, -1);
	screenContext->cr_on_canvas = gdk_cairo_create(screenContext->canvas);
	screenContext->cr_on_screen = gdk_cairo_create(screenContext->drawingArea->window);
  	screenContext->gc = gdk_gc_new(screenContext->drawingArea->window);
}


void setup_drawing_content(struct ScreenContext *screenContext, int scr_x, int scr_y, int scr_width, int scr_height, struct WindowSize *awin)
{
	double deltaCx, deltaCy, deltaSx, deltaSy;
	struct Item *aItem;

	duallist_destroy(&screenContext->roads, NULL);
	duallist_destroy(&screenContext->crosses, NULL);
	duallist_destroy(&screenContext->districts, NULL);
	duallist_destroy(&screenContext->rivers, NULL);
	duallist_destroy(&screenContext->surroundings, NULL);
	duallist_destroy(&screenContext->rsus, NULL);
	duallist_destroy(&screenContext->links, NULL);

	if(scr_x != NOTSET)
		screenContext->scr_x = scr_x;
	if(scr_y != NOTSET)
		screenContext->scr_y = scr_y;
	if(scr_width != NOTSET)
		screenContext->scr_width = scr_width;
	if(scr_height != NOTSET)
		screenContext->scr_height = scr_height;

	if(awin!=NULL) {
		screenContext->awin.cBox.xmin = awin->cBox.xmin;
		screenContext->awin.cBox.xmax = awin->cBox.xmax;
		screenContext->awin.cBox.ymin = awin->cBox.ymin;
		screenContext->awin.cBox.ymax = awin->cBox.ymax;
		screenContext->awin.sBox.xmin = awin->sBox.xmin;
		screenContext->awin.sBox.xmax = awin->sBox.xmax;
		screenContext->awin.sBox.ymin = awin->sBox.ymin;
		screenContext->awin.sBox.ymax = awin->sBox.ymax;
	}
	canvas_to_gps(&screenContext->awin, screenContext->scr_x+MARGIN, screenContext->scr_y+screenContext->scr_height-MARGIN, &(screenContext->box.xmin), &(screenContext->box.ymin));
	canvas_to_gps(&screenContext->awin, screenContext->scr_x+screenContext->scr_width-MARGIN, screenContext->scr_y+MARGIN, &(screenContext->box.xmax), &(screenContext->box.ymax));

	aItem = screenContext->region->roads.head;
	while(aItem != NULL) {
		if(are_boxes_intersected(&((struct Road*)aItem->datap)->box,&screenContext->box)) {
			duallist_add_to_head(&screenContext->roads, aItem->datap);	
		}
		aItem = aItem->next;
	}

	aItem = screenContext->region->crosses.head;
	while(aItem != NULL) {
		if(are_boxes_intersected(&((struct Cross*)aItem->datap)->box, &screenContext->box)) {
			duallist_add_to_head(&screenContext->crosses, aItem->datap);
		}
		aItem = aItem->next;
	}
	
	aItem = screenContext->region->districts.head;
	while(aItem!=NULL) {
		if(are_boxes_intersected(&((struct District*)aItem->datap)->box,&screenContext->box)) {
			duallist_add_to_head(&screenContext->districts, aItem->datap);
		}
		aItem = aItem->next;
	}

	aItem = screenContext->region->rivers.head;
	while(aItem!=NULL) {
		if(are_boxes_intersected(&((struct River*)aItem->datap)->box,&screenContext->box)) {
			duallist_add_to_head(&screenContext->rivers, aItem->datap);
		}
		aItem = aItem->next;
	}

	if(screenContext->vRegion != NULL) {
		aItem = screenContext->vRegion->roads.head;
		while(aItem != NULL) {
			if(are_boxes_intersected(&((struct Road*)aItem->datap)->box,&screenContext->box)) {
				duallist_add_to_head(&screenContext->links, aItem->datap);	
			}
			aItem = aItem->next;
		}

		aItem = screenContext->vRegion->crosses.head;
		while(aItem != NULL) {
			if(are_boxes_intersected(&((struct Cross*)aItem->datap)->box, &screenContext->box)) {
				duallist_add_to_head(&screenContext->rsus, aItem->datap);
			}
			aItem = aItem->next;
		}
	}
	screenContext->mouseAtRoad = NULL;
	screenContext->mouseAtCross = NULL;
	screenContext->mouseAtCell = NULL;
	screenContext->mouseAtReport = NULL;

	/* how long a pixel is in meters on the map*/
	deltaCx =screenContext->awin.cBox.xmax-screenContext->awin.cBox.xmin;
	deltaCy =screenContext->awin.cBox.ymax-screenContext->awin.cBox.ymin;
	deltaSx =screenContext->awin.sBox.xmax-screenContext->awin.sBox.xmin;
	deltaSy =screenContext->awin.sBox.ymax-screenContext->awin.sBox.ymin;
	if(deltaCy/deltaCx > deltaSy/deltaSx)
	      screenContext->meterperpixel = deltaCy/deltaSy;
	else 
	      screenContext->meterperpixel = deltaCx/deltaSx;
	screenContext->meterperpixel = distance_in_meter(0, 0, 0, screenContext->meterperpixel); 
}


void destroy_screen_context(struct ScreenContext *screenContext)
{
  	region_free_func(screenContext->region);
  	region_free_func(screenContext->vRegion);
	duallist_destroy(&screenContext->roads, NULL);
	duallist_destroy(&screenContext->crosses, NULL);
	duallist_destroy(&screenContext->links, NULL);
	duallist_destroy(&screenContext->rsus, NULL);
	duallist_destroy(&screenContext->districts, NULL);
	duallist_destroy(&screenContext->rivers, NULL);
	duallist_destroy(&screenContext->surroundings, NULL);
	hashtable_destroy(&screenContext->routeTable, (void(*)(void*))route_free_func);
	duallist_destroy(&screenContext->selectedRoutes, NULL);
	hashtable_destroy(&screenContext->nodeTable, (void(*)(void*))node_free_func);
	hashtable_destroy(&screenContext->traceTable, (void(*)(void*))trace_free_func);
	duallist_destroy(&screenContext->selectedTraces, NULL);
	if(screenContext->contactTableMode == PAIRWISE_TABLE)
		hashtable_destroy(&screenContext->contactTable, (void(*)(void*))pair_free_func);
	else
		hashtable_destroy(&screenContext->contactTable, (void(*)(void*))ego_free_func);
	duallist_destroy(&screenContext->selected, NULL);
	hashtable_destroy(&screenContext->cellTable, NULL);
	free_colormap(screenContext->colormap);

  	gdk_gc_unref(screenContext->gc);
	if(screenContext->canvas) {
		g_object_unref(screenContext->canvas);
		cairo_destroy(screenContext->cr_on_canvas);
		cairo_destroy(screenContext->cr_on_screen);
	}
  	if(screenContext->timeout != 0) 
		gtk_timeout_remove(screenContext->timeout);  
	free(screenContext);
}


void draw_canvas(struct ScreenContext *screenContext) 
{

	/* draw background */
	cairo_set_source_rgb(screenContext->cr_on_canvas, 1, 1, 1);
	cairo_paint(screenContext->cr_on_canvas);	
	cairo_rectangle(screenContext->cr_on_canvas, MARGIN, MARGIN,screenContext->scr_width-2*MARGIN, screenContext->scr_height-2*MARGIN);
	cairo_clip(screenContext->cr_on_canvas);
	cairo_rectangle(screenContext->cr_on_canvas, MARGIN, MARGIN,screenContext->scr_width-2*MARGIN, screenContext->scr_height-2*MARGIN);
	cairo_set_source_rgb(screenContext->cr_on_canvas, 176.0/255, 212.0/255, 236.0/255);
	cairo_fill(screenContext->cr_on_canvas);


	/* draw districts */
	draw_districts(screenContext);

	/* draw rivers */
	draw_rivers(screenContext);

	/* draw roads */
	draw_roads(screenContext);

	/* draw crosses */
	draw_crosses(screenContext);

	draw_dynamic_on_screen(screenContext);
}

void draw_dynamic_on_screen(struct ScreenContext *screenContext)
{
	gdk_draw_drawable(screenContext->drawingArea->window, screenContext->gc, screenContext->canvas, 0, 0, 0, 0, -1, -1);

	/* draw bus routes */
	draw_selected_routes(screenContext, screenContext->cr_on_screen);

	/* draw displayed traces */
	draw_displayed_traces(screenContext, screenContext->cr_on_screen);
	/* draw displayed contacts */
	draw_displayed_contacts(screenContext, screenContext->cr_on_screen);

	draw_scale(screenContext);
}

void draw_single_cell(struct ScreenContext *screenContext, cairo_t *cr, struct Cell *aCell, struct RGBO *color)
{

	struct Point p1;

	gps_to_canvas(&screenContext->awin, aCell->box.xmin, aCell->box.ymin, &p1.x, &p1.y);
	cairo_move_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCell->box.xmin, aCell->box.ymax, &p1.x, &p1.y);
	cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCell->box.xmax, aCell->box.ymax, &p1.x, &p1.y);
	cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCell->box.xmax, aCell->box.ymin, &p1.x, &p1.y);
	cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	cairo_close_path(cr);

	cairo_set_source_rgba(cr, color->red/255.0, color->green/255.0, color->blue/255.0, color->opacity/255.0);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, 0.5);
	cairo_stroke(cr);
}


void draw_single_route(struct ScreenContext *screenContext, cairo_t *cr, struct Busroute *aRoute)
{
	struct Item *bItem;
	struct RGBO aRgbo;

	aRgbo.red = 255;
	aRgbo.green = 0;
	aRgbo.blue = 0;
	aRgbo.opacity = 255;

	bItem = aRoute->path->roads.head;
	while(bItem!=NULL) {
//		draw_single_road(screenContext, cr, (struct Road*)bItem->datap, &aRoute->color.rgbo); 
		draw_single_road(screenContext, cr, (struct Road*)bItem->datap, &aRgbo); 
		bItem = bItem->next;
	}
	bItem = aRoute->stops.head;
	while(bItem!=NULL) {
		aRgbo.red = MAX(aRoute->color.rgbo.red - 20, 0);
		aRgbo.green = MAX(aRoute->color.rgbo.green - 20, 0);
		aRgbo.blue = MIN(aRoute->color.rgbo.blue + 20, 255);
		aRgbo.opacity = aRoute->color.rgbo.opacity;
		draw_single_cell(screenContext, cr, (struct Cell*)bItem->datap, &aRgbo); 
		bItem = bItem->next;
	}

}


void draw_selected_routes(struct ScreenContext *screenContext, cairo_t *cr)
{
	struct Item *aSelected;

	aSelected = screenContext->selectedRoutes.head;
	while(aSelected != NULL) {
		draw_single_route(screenContext, cr, (struct Busroute*)aSelected->datap);
		aSelected = aSelected->next;
	}
}



void draw_displayed_traces(struct ScreenContext *screenContext, cairo_t *cr)
{
	struct Item *aSelected;

	aSelected = screenContext->selectedTraces.head;
	while(aSelected != NULL) {
		draw_displayed_reports(screenContext, cr, (struct Trace*)aSelected->datap);
		aSelected = aSelected->next;
	}
}


void show_district(struct ScreenContext *screenContext, struct District *aDistrict, cairo_t *cr)
{
	struct Point point, point1;
	struct Item *pllp, *pp;
  	struct Color fillColor;

	fillColor.red = 235.0/255;
	fillColor.green = 210.0/255;
	fillColor.blue = 175.0/255;


	if(aDistrict != NULL) {
		pllp = aDistrict->rings.head;
		while(pllp!=NULL) {
			pp = ((struct Duallist*)pllp->datap)->head;
			point1.x = ((struct Point*)pp->datap)->x, point1.y = ((struct Point*)pp->datap)->y;
			g_print("%.4lf %.4lf ", point1.x, point1.y);
			gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y, &point.x, &point.y);
			cairo_move_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
			pp = pp->next;
			while(pp!=NULL) {
				g_print("%.4lf %.4lf ", ((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y);
				gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y,&point.x,&point.y);
				cairo_line_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
				pp = pp->next;
			}
			cairo_close_path(cr);
			cairo_set_source_rgba(cr, fillColor.red, fillColor.green, fillColor.blue, 0.3);
			cairo_fill(cr);
			pllp = pllp->next;
		}
		g_print("%.4lf %.4lf\n", point1.x, point1.y);
	}
}



void draw_districts(struct ScreenContext *screenContext)
{

	double dashes[]={5.0, 2.0, 3.0, 2.0};
	int ndash = sizeof(dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	struct Point point;
	double transparent;
  	struct Color fillColor, lineColor;
	struct Item *distp, *pllp, *pp;

/*
	fillColor.red = 230.0/255;
	fillColor.green = 225.0/255;
	fillColor.blue = 215.0/255;
*/
	fillColor.red = 255.0/255;
	fillColor.green = 251.0/255;
	fillColor.blue = 240.0/255;


	/* boundary stroke */
	distp = screenContext->districts.head;
	while(distp!=NULL) {
		pllp = ((struct District*)distp->datap)->rings.head;
		while(pllp!=NULL) {
			pp = ((struct Duallist*)pllp->datap)->head;
			gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y, &point.x, &point.y);
			cairo_move_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
			pp = pp->next;
			while(pp!=NULL) {
				gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y,&point.x,&point.y);
				cairo_line_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
				pp = pp->next;
			}
			cairo_close_path(screenContext->cr_on_canvas);
			pllp = pllp->next;
		}
		lineColor.red = 36.0/255;
		lineColor.green = 72.0/255;
		lineColor.blue = 108.0/255;

		cairo_set_line_width(screenContext->cr_on_canvas, 4);
		cairo_set_source_rgb(screenContext->cr_on_canvas, lineColor.red, lineColor.green, lineColor.blue);
		cairo_stroke(screenContext->cr_on_canvas);
		distp = distp->next;
	}

	/* fill color */
	transparent = 1;
	distp = screenContext->districts.head;
	while(distp!=NULL) {
		pllp = ((struct District*)distp->datap)->rings.head;
		while(pllp!=NULL) {
			pp = ((struct Duallist*)pllp->datap)->head;
			gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y, &point.x, &point.y);
			cairo_move_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
			pp = pp->next;
			while(pp!=NULL) {
				gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y,&point.x,&point.y);
				cairo_line_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
				pp = pp->next;
			}
			cairo_close_path(screenContext->cr_on_canvas);
			pllp = pllp->next;
		}
		cairo_set_source_rgba(screenContext->cr_on_canvas, fillColor.red, fillColor.green, fillColor.blue, transparent);
		cairo_fill(screenContext->cr_on_canvas);
		distp = distp->next;
	}

	/* inner stroke */
	distp = screenContext->districts.head;
	cairo_set_dash(screenContext->cr_on_canvas, dashes, ndash, offset);
	while(distp!=NULL) {
		pllp = ((struct District*)distp->datap)->rings.head;
		while(pllp!=NULL) {
			pp = ((struct Duallist*)pllp->datap)->head;
			gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y, &point.x, &point.y);
			cairo_move_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
			pp = pp->next;
			while(pp!=NULL) {
				gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y,&point.x,&point.y);
				cairo_line_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
				pp = pp->next;
			}
			cairo_close_path(screenContext->cr_on_canvas);
			pllp = pllp->next;
		}

		lineColor.red = 160.0/255;
		lineColor.green = 160.0/255;
		lineColor.blue = 164.0/255;

		cairo_set_line_width(screenContext->cr_on_canvas, 0.3);
		cairo_set_source_rgb(screenContext->cr_on_canvas, lineColor.red, lineColor.green, lineColor.blue);
		cairo_stroke(screenContext->cr_on_canvas);
		distp = distp->next;
	}
	cairo_set_dash(screenContext->cr_on_canvas, dashes, 0, offset);	
}


void draw_rivers(struct ScreenContext *screenContext)
{
	struct Point point;
	double transparent;
	struct Item *riverp, *pllp, *pp;
  	struct Color fillColor, lineColor;

	fillColor.red = 175.0/255;
	fillColor.green = 210.0/255;
	fillColor.blue = 235.0/255;

	lineColor.red = 140.0/255;
	lineColor.green = 145.0/255;
	lineColor.blue = 135.0/255;

	transparent = 1;

	riverp = screenContext->rivers.head;
	while(riverp!=NULL) {
		pllp = ((struct River*)riverp->datap)->rings.head;
		while(pllp!=NULL) {
			pp = ((struct Duallist*)pllp->datap)->head;
			gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y, &point.x, &point.y);
			cairo_move_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
			pp = pp->next;
			while(pp!=NULL) {
				gps_to_canvas(&screenContext->awin,((struct Point*)pp->datap)->x,((struct Point*)pp->datap)->y,&point.x,&point.y);
				cairo_line_to(screenContext->cr_on_canvas, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
				pp = pp->next;
			}
			cairo_close_path(screenContext->cr_on_canvas);
			pllp = pllp->next;
		}
		cairo_set_line_width(screenContext->cr_on_canvas, 1);
		cairo_set_source_rgba(screenContext->cr_on_canvas, lineColor.red, lineColor.green, lineColor.blue, transparent);
		cairo_stroke_preserve(screenContext->cr_on_canvas);
		cairo_set_source_rgba(screenContext->cr_on_canvas, fillColor.red, fillColor.green, fillColor.blue, transparent);
		cairo_fill(screenContext->cr_on_canvas);
		riverp = riverp->next;
	}

}

void draw_scale(struct ScreenContext *screenContext)
{
	struct Point aSPoint, bSPoint, aCPoint, bCPoint;
	cairo_text_extents_t extents;
	char buf[128], buf1[128];
	double dist;
	
	aSPoint.x = MARGIN+40;
	aSPoint.y = MARGIN+40;
	bSPoint.x = MARGIN+90;
	bSPoint.y = MARGIN+40;

	cairo_move_to(screenContext->cr_on_screen, aSPoint.x, aSPoint.y);
	cairo_line_to(screenContext->cr_on_screen, aSPoint.x, aSPoint.y+3);
	cairo_line_to(screenContext->cr_on_screen, aSPoint.x+50, aSPoint.y+3);
	cairo_line_to(screenContext->cr_on_screen, aSPoint.x+50, aSPoint.y);
	cairo_set_line_width(screenContext->cr_on_screen, 1);
	cairo_set_source_rgb(screenContext->cr_on_screen, 64.0/255, 0, 64.0/255);
	cairo_stroke(screenContext->cr_on_screen);
	
	canvas_to_gps(&screenContext->awin, screenContext->scr_x+aSPoint.x, screenContext->scr_y+aSPoint.y, &aCPoint.x, &aCPoint.y);
	canvas_to_gps(&screenContext->awin, screenContext->scr_x+bSPoint.x, screenContext->scr_y+bSPoint.y, &bCPoint.x, &bCPoint.y);
	dist = distance_in_meter(aCPoint.x, 0, bCPoint.x, 0);

	sprintf(buf, "0");
	if (dist/1000 > 1) {
		sprintf(buf1, "%.2lfkm", dist/1000);
	} else
		sprintf(buf1, "%.0lfm", dist);

	cairo_select_font_face(screenContext->cr_on_screen, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(screenContext->cr_on_screen, FONT_SIZE);
	cairo_text_extents(screenContext->cr_on_screen, buf, &extents);
	cairo_set_source_rgb(screenContext->cr_on_screen, 64.0/255, 0, 64.0/255);
	cairo_move_to(screenContext->cr_on_screen, aSPoint.x-5, aSPoint.y+15);
	cairo_show_text(screenContext->cr_on_screen, buf);
	cairo_text_extents(screenContext->cr_on_screen, buf1, &extents);
	cairo_move_to(screenContext->cr_on_screen, aSPoint.x+40, aSPoint.y+15);
	cairo_show_text(screenContext->cr_on_screen, buf1);
}

void rubber_cells(struct ScreenContext *screenContext)
{
	struct Point p1, p2;
	double minx, miny, maxx, maxy;
	struct Item *aItem;

	if(screenContext->surroundings.head == NULL) return;

	aItem = screenContext->surroundings.head;
	gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmin, ((struct Cell*)aItem->datap)->box.ymin, &minx, &maxy);
	gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmax, ((struct Cell*)aItem->datap)->box.ymax, &maxx, &miny);
	while(aItem!=NULL) {
		gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmin, ((struct Cell*)aItem->datap)->box.ymin, &p1.x, &p1.y);
		gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmax, ((struct Cell*)aItem->datap)->box.ymax, &p2.x, &p2.y);
		if (p1.x < minx) minx = p1.x;
		if (p1.y > maxy) maxy = p1.y;
		if (p2.x > maxx) maxx = p2.x;
		if (p2.y < miny) miny = p2.y;
		aItem=aItem->next;
	}
	gdk_draw_drawable(screenContext->drawingArea->window, screenContext->gc, screenContext->canvas, minx-screenContext->scr_x , miny-screenContext->scr_y, minx-screenContext->scr_x, miny-screenContext->scr_y, maxx-minx, maxy-miny);
}


void draw_cells(struct ScreenContext *screenContext, cairo_t *cr)
{
	char buf[128];
	double transparent;
	struct Item *aItem;
	struct Point p1, p2;
	double cellsize, fontSize, fsize, fontX, fontY;
	cairo_text_extents_t extents;
  	struct Color fillColor, lineColor, fontColor;

	fillColor.red = 175.0/255;
	fillColor.green = 210.0/255;
	fillColor.blue = 235.0/255;

	lineColor.red = 140.0/255;
	lineColor.green = 145.0/255;
	lineColor.blue = 135.0/255;

	fontColor.red = 0/255;
	fontColor.green = 0/255;
	fontColor.blue = 0/255;

	transparent = 0.3;

	aItem = screenContext->surroundings.head;
	gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmin, ((struct Cell*)aItem->datap)->box.ymin, &p1.x, &p1.y);
	gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmax, ((struct Cell*)aItem->datap)->box.ymax, &p2.x, &p2.y);
	cellsize = p2.x - p1.x;
	while(aItem!=NULL) {
		gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmin, ((struct Cell*)aItem->datap)->box.ymin, &p1.x, &p1.y);
		fontX = p1.x-screenContext->scr_x, fontY = p1.y-screenContext->scr_y;
		cairo_move_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
		gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmin, ((struct Cell*)aItem->datap)->box.ymax, &p1.x, &p1.y);
		cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
		gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmax, ((struct Cell*)aItem->datap)->box.ymax, &p1.x, &p1.y);
		cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
		gps_to_canvas(&screenContext->awin, ((struct Cell*)aItem->datap)->box.xmax, ((struct Cell*)aItem->datap)->box.ymin, &p1.x, &p1.y);
		cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
		cairo_close_path(cr);
		cairo_set_line_width(cr, 1);
		cairo_set_source_rgba(cr, lineColor.red, lineColor.green, lineColor.blue, transparent);
		cairo_stroke_preserve(cr);
		cairo_set_source_rgba(cr, fillColor.red, fillColor.green, fillColor.blue, transparent);
		cairo_fill(cr);

		sprintf(buf, "(%d, %d)", ((struct Cell*)aItem->datap)->xNumber, ((struct Cell*)aItem->datap)->yNumber);
		if (aItem == screenContext->surroundings.head) {
			fsize = 2*cellsize/(strlen(buf)+2);
			if (fsize > 25) fsize = 15;
			fontSize = fsize;
		} else
			fontSize = 0.75*fsize;

		if(fsize > 5) {
			fontX = fontX + fontSize*0.5;
			fontY = fontY + fontSize*0.5;
			cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
			cairo_set_font_size(cr, (int)fontSize);
			cairo_text_extents(cr, buf, &extents);
			cairo_set_source_rgba(cr, fontColor.red, fontColor.green, fontColor.blue, transparent);
			cairo_move_to(cr, fontX, fontY-extents.height);
			cairo_show_text(cr, buf);
		}
		aItem=aItem->next;
	}
}

/* draw the dynamic display at Cell.at */
void draw_cell_display(struct ScreenContext *screenContext, cairo_t *cr, struct Cell *aCell)
{

	struct Point p1;
	struct Color aColor;

	gps_to_canvas(&screenContext->awin, aCell->box.xmin, aCell->box.ymin, &p1.x, &p1.y);
	cairo_move_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCell->box.xmin, aCell->box.ymax, &p1.x, &p1.y);
	cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCell->box.xmax, aCell->box.ymax, &p1.x, &p1.y);
	cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCell->box.xmax, aCell->box.ymin, &p1.x, &p1.y);
	cairo_line_to(cr, p1.x-screenContext->scr_x, p1.y-screenContext->scr_y);
	cairo_close_path(cr);


	get_color(screenContext->colormap, &aColor, ((struct Display*)aCell->at->datap)->value, screenContext->region->lower, screenContext->region->upper);
	if(aColor.red == -1) {
		aColor.red = 230.0/255;
		aColor.green = 225.0/255;
		aColor.blue = 215.0/255;
	}
	
	cairo_set_source_rgb(cr, aColor.red, aColor.green, aColor.blue);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr, 0.5);
	cairo_stroke(cr);
}

void draw_links(struct ScreenContext *screenContext)
{
	struct Item *anEntry;
  	struct RGBO fillColor;
	double dashes[]={4.0, 4.0};
	int ndash = sizeof(dashes)/sizeof(dashes[0]);
	double offset = -10.0;

	fillColor.red = 255;
	fillColor.green = 0;
	fillColor.blue = 0;
	fillColor.opacity = 255;

	cairo_set_line_join(screenContext->cr_on_screen, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(screenContext->cr_on_screen, CAIRO_LINE_CAP_BUTT);
	cairo_set_dash(screenContext->cr_on_screen, dashes, ndash, offset);

	anEntry = screenContext->links.head;
	while(anEntry != NULL) {
	      draw_single_road(screenContext, screenContext->cr_on_screen, anEntry->datap, &fillColor);
	      anEntry = anEntry->next;
	}

	cairo_set_dash(screenContext->cr_on_screen, dashes, 0, offset);	
}


void draw_roads(struct ScreenContext *screenContext)
{
	struct Item *anEntry;
  	struct RGBO fillColor;

	fillColor.red = 255;
	fillColor.green = 255;
	fillColor.blue = 255;
	fillColor.opacity = 255;

	cairo_set_line_join(screenContext->cr_on_canvas, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(screenContext->cr_on_canvas, CAIRO_LINE_CAP_BUTT);

	anEntry = screenContext->roads.head;
	while(anEntry != NULL) {
	      draw_single_road(screenContext, screenContext->cr_on_canvas, anEntry->datap, &fillColor);
	      anEntry = anEntry->next;
	}
}



void draw_single_road(struct ScreenContext *screenContext, cairo_t *cr, struct Road *aRoad, struct RGBO *fillColor)
{
	struct Item *p;
	struct Point point, mp;
	double width, wRoad;
	struct Color lineColor;
	double dist, d, angle;

	width = aRoad->width/screenContext->meterperpixel;
	if(width < 1) 
		wRoad =1;
	else 
		wRoad = width;
	/* draw a road */
	lineColor.red = 179.0/255;
	lineColor.green = 166.0/255;
	lineColor.blue = 147.0/255;
	// for test, draw origpoints
	if (screenContext->debug) {
		p = aRoad->origPoints.head;
		gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
		cairo_move_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
		p = p->next;
		while(p!=NULL) {
			gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
			cairo_line_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
			p = p->next;
		}
		cairo_set_line_width(cr, wRoad+1);
		cairo_set_source_rgb(cr, lineColor.red, lineColor.green, lineColor.blue);
		cairo_stroke_preserve(cr);
		cairo_set_line_width(cr, wRoad);
		cairo_set_source_rgba(cr, fillColor->red/255.0, fillColor->green/255.0, fillColor->blue/255.0, fillColor->opacity/255.0);
		cairo_stroke(cr);
				
		p = aRoad->origPoints.head;
		gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
		cairo_arc(cr,  (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y, 2, 0, 2*M_PI) ;
		cairo_set_source_rgb(cr, 0.2, 0.2, 0.8);
		cairo_fill(cr);
		p = p->next;
		while(p!=NULL) {
			gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
			cairo_arc(cr,  (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y, 2, 0, 2*M_PI) ;
			cairo_set_source_rgb(cr, 0.2, 0.2, 0.8);
			cairo_fill(cr);
			p = p->next;
		}
	}
	// draw points on the road
	p = aRoad->points.head;
	gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
	cairo_move_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
	dist = 0;
	p = p->next;
	while(p!=NULL) {
		gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
		cairo_line_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
		d = distance_in_meter(((struct Point*)p->prev->datap)->x, ((struct Point*)p->prev->datap)->y, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y);
		if (dist < aRoad->length/2 && dist + d > aRoad->length/2) {
			gps_to_canvas(&screenContext->awin, (((struct Point*)p->prev->datap)->x+((struct Point*)p->datap)->x)/2, (((struct Point*)p->prev->datap)->y+((struct Point*)p->datap)->y)/2, &mp.x, &mp.y);
			angle = angle_between(((struct Point*)p->prev->datap)->x, ((struct Point*)p->prev->datap)->y, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y);
		} 
		dist += d;
		p = p->next;
	}
	cairo_set_line_width(cr, wRoad+1);
	cairo_set_source_rgb(cr, lineColor.red, lineColor.green, lineColor.blue);
	cairo_stroke_preserve(cr);
	cairo_set_line_width(cr, wRoad);
	cairo_set_source_rgba(cr, fillColor->red/255.0, fillColor->green/255.0, fillColor->blue/255.0, fillColor->opacity/255.0);
	cairo_stroke(cr);
			
	if (screenContext->debug) {
		p = aRoad->points.head;
		gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
		cairo_arc(cr,  (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y, 2, 0, 2*M_PI) ;
		cairo_set_source_rgb(cr, 0.2, 0.2, 0.8);
		cairo_fill(cr);
		p = p->next;
		while(p!=NULL) {
			gps_to_canvas(&screenContext->awin, ((struct Point*)p->datap)->x, ((struct Point*)p->datap)->y, &point.x, &point.y);
			cairo_arc(cr,  (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y, 2, 0, 2*M_PI) ;
			cairo_set_source_rgb(cr, 0.2, 0.2, 0.8);
			cairo_fill(cr);
			p = p->next;
		}
	}
        // draw arrow
        if(screenContext->meterperpixel < 2.5) {
		cairo_set_line_width(cr, 1);
		cairo_set_source_rgb(cr, lineColor.red, lineColor.green, lineColor.blue);
		cairo_move_to(cr, mp.x+cos(M_PI*(90+angle)/180)*MARK_LENGTH/4-screenContext->scr_x, mp.y-sin(M_PI*(90+angle)/180)*MARK_LENGTH/4-screenContext->scr_y);
		cairo_line_to(cr, mp.x+cos(M_PI*angle/180)*MARK_LENGTH/2-screenContext->scr_x, mp.y-sin(M_PI*angle/180)*MARK_LENGTH/2-screenContext->scr_y);
		cairo_line_to(cr, mp.x+cos(M_PI*(270+angle)/180)*MARK_LENGTH/4-screenContext->scr_x, mp.y-sin(M_PI*(270+angle)/180)*MARK_LENGTH/4-screenContext->scr_y);
		cairo_stroke(cr);
		cairo_move_to(cr, mp.x+cos(M_PI*angle/180)*MARK_LENGTH/2-screenContext->scr_x, mp.y-sin(M_PI*angle/180)*MARK_LENGTH/2-screenContext->scr_y);
		cairo_line_to(cr, mp.x-cos(M_PI*angle/180)*MARK_LENGTH/2-screenContext->scr_x, mp.y+sin(M_PI*angle/180)*MARK_LENGTH/2-screenContext->scr_y);
		cairo_stroke(cr);
	}
}


void draw_rsus(struct ScreenContext *screenContext)
{
	struct Cross *aCross;
	struct Item *anEntry;
	struct Point aPoint, bPoint, cPoint;

	anEntry = screenContext->rsus.head;
	while(anEntry != NULL) {
		aCross = (struct Cross*)anEntry->datap;
		gps_to_canvas(&screenContext->awin, aCross->gPoint.x, aCross->gPoint.y, &aPoint.x, &aPoint.y);
		gps_to_canvas(&screenContext->awin, aCross->box.xmin, aCross->box.ymin, &bPoint.x, &bPoint.y);
		gps_to_canvas(&screenContext->awin, aCross->box.xmax, aCross->box.ymin, &cPoint.x, &cPoint.y);
		cairo_arc(screenContext->cr_on_screen,  (int)aPoint.x-screenContext->scr_x, (int)aPoint.y-screenContext->scr_y, (cPoint.x-bPoint.x)*5, 0, 2*M_PI) ;
		cairo_set_source_rgb(screenContext->cr_on_screen, 0, 0, 0);
		cairo_stroke_preserve(screenContext->cr_on_screen);
		cairo_set_source_rgb(screenContext->cr_on_screen, 0.2, 0.2, 0.8);
		cairo_fill(screenContext->cr_on_screen);
		anEntry = anEntry->next;
	}

}

void draw_crosses(struct ScreenContext *screenContext)
{
	struct Cross *aCross;
	struct Item *anEntry;
  	struct RGBO color;

	color.red = 255;
	color.green = 255;
	color.blue = 255;
	color.opacity = 255;

	anEntry = screenContext->crosses.head;
	while(anEntry != NULL) {
		aCross = (struct Cross*)anEntry->datap;
		draw_single_cross(screenContext, screenContext->cr_on_canvas, aCross, &color); 
		anEntry = anEntry->next;
	}
}


void draw_single_cross(struct ScreenContext *screenContext, cairo_t *cr, struct Cross *aCross, struct RGBO *color)
{
	struct Point point;

	gps_to_canvas(&screenContext->awin, aCross->box.xmin, aCross->box.ymin, &point.x, &point.y);
	cairo_move_to(cr, (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCross->box.xmin, aCross->box.ymax, &point.x, &point.y);
	cairo_line_to(cr, (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCross->box.xmax, aCross->box.ymax, &point.x, &point.y);
	cairo_line_to(cr, (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y);
	gps_to_canvas(&screenContext->awin, aCross->box.xmax, aCross->box.ymin, &point.x, &point.y);
	cairo_line_to(cr, (int)point.x-screenContext->scr_x, (int)point.y-screenContext->scr_y);
	cairo_close_path(cr);
	cairo_set_source_rgba(cr, color->red/255.0, color->green/255.0, color->blue/255.0, color->opacity/255.0);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);
}


void draw_displayed_reports(struct ScreenContext *screenContext, cairo_t *cr, struct Trace *aTrace) 
{

  struct Point point;
  struct Report *aReport;

  if (aTrace == NULL) return;

  struct Item *aItem = aTrace->reports.head;
  while(aItem != aTrace->at) {
	aReport = (struct Report*)aItem->datap;
	gps_to_canvas(&screenContext->awin, aReport->gPoint.x, aReport->gPoint.y, &(point.x), &(point.y));
	if(aReport->shown==1 && point.x > screenContext->scr_x+MARGIN && point.x < screenContext->scr_x+screenContext->scr_width-MARGIN && point.y > screenContext->scr_y+MARGIN && point.y < screenContext->scr_y+screenContext->scr_height-MARGIN ) {
		draw_single_report(screenContext, cr, aReport, &aReport->fromTrace->color.rgbo);
	}
	aItem = aItem->next;
  }
}


void draw_single_report(struct ScreenContext *screenContext, cairo_t *cr, struct Report *aReport, struct RGBO *color)
{
	struct Point point;

	if(aReport==NULL || !is_point_in_box(&aReport->gPoint, &screenContext->region->chosen_polygon->box )) return;
	
	gps_to_canvas(&screenContext->awin, aReport->gPoint.x, aReport->gPoint.y, &(point.x), &(point.y));
	cairo_move_to(cr, point.x+cos(M_PI*aReport->angle/180)*MARK_LENGTH-screenContext->scr_x, point.y-sin(M_PI*aReport->angle/180)*MARK_LENGTH-screenContext->scr_y);
	cairo_line_to(cr, point.x+cos(M_PI*(90+aReport->angle)/180)*MARK_LENGTH/4-screenContext->scr_x, point.y-sin(M_PI*(90+aReport->angle)/180)*MARK_LENGTH/4-screenContext->scr_y);
	cairo_line_to(cr, point.x+cos(M_PI*(270+aReport->angle)/180)*MARK_LENGTH/4-screenContext->scr_x, point.y-sin(M_PI*(270+aReport->angle)/180)*MARK_LENGTH/4-screenContext->scr_y);
	cairo_set_source_rgb(cr, color->red/255.0, color->green/255.0, color->blue/255.0);
		
	cairo_fill(cr);
}


void draw_single_storage_node(struct ScreenContext *screenContext, cairo_t *cr, struct Node *aNode, struct RGBO *color)
{
	struct Point point;

	gps_to_canvas(&screenContext->awin, aNode->gPoint.x, aNode->gPoint.y, &(point.x), &(point.y));
	
	cairo_move_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
	cairo_line_to(cr, point.x-0.3*MARK_LENGTH-screenContext->scr_x, point.y+MARK_LENGTH-screenContext->scr_y);
	cairo_line_to(cr, point.x+0.3*MARK_LENGTH-screenContext->scr_x, point.y+MARK_LENGTH-screenContext->scr_y);
	cairo_line_to(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y);
	cairo_set_source_rgb(cr, color->red/255.0, color->green/255.0, color->blue/255.0);
	cairo_arc(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y, MARK_LENGTH/4, 0, 2*M_PI);
	cairo_set_source_rgb(cr, color->red/255.0, color->green/255.0, color->blue/255.0);
		
	cairo_fill(cr);
}



void draw_displayed_contacts(struct ScreenContext *screenContext, cairo_t *cr)
{
	struct Item *aSelected;

	aSelected = screenContext->selected.head;
	while(aSelected != NULL) {
		if(screenContext->contactTableMode == PAIRWISE_TABLE)
			draw_displayed_contacts_of_a_pair(screenContext, cr, (struct Pair*)aSelected->datap);
		else
			draw_displayed_contacts_of_an_ego(screenContext, cr, (struct Ego*)aSelected->datap);

		aSelected = aSelected->next;
	}
}

void draw_displayed_contacts_of_a_pair(struct ScreenContext *screenContext, cairo_t *cr, struct Pair *aPair) 
{

  struct Point point;
  struct Contact *aContact;

  if (aPair == NULL) return;

  struct Item *aItem = aPair->contents.head;
  while(aItem != aPair->at) {
	aContact = (struct Contact*)aItem->datap;
	gps_to_canvas(&screenContext->awin, aContact->gPoint.x, aContact->gPoint.y, &(point.x), &(point.y));
	if(aContact->shown==1 && point.x > screenContext->scr_x+MARGIN && point.x < screenContext->scr_x+screenContext->scr_width-MARGIN && point.y > screenContext->scr_y+MARGIN && point.y < screenContext->scr_y+screenContext->scr_height-MARGIN ) {
		draw_single_contact(screenContext, cr, aContact, &aContact->fromPair->color.rgbo);
	}
	aItem = aItem->next;
  }
}

void draw_displayed_contacts_of_an_ego(struct ScreenContext *screenContext, cairo_t *cr, struct Ego *anEgo) 
{
  struct Point point;
  struct Linkman *aLinkman;
  struct Contact *aContact;
  struct Item *aItem, *bItem;

  if (anEgo == NULL) return;

  aItem = anEgo->linkmen.head;
  while(aItem) {
	aLinkman = (struct Linkman*)aItem->datap;
	bItem = aLinkman->contacts.head;
	while(bItem != aLinkman->at) {
		aContact = (struct Contact*)bItem->datap;
		gps_to_canvas(&screenContext->awin, aContact->gPoint.x, aContact->gPoint.y, &(point.x), &(point.y));
		if(aContact->shown==1 && point.x > screenContext->scr_x+MARGIN && point.x < screenContext->scr_x+screenContext->scr_width-MARGIN && point.y > screenContext->scr_y+MARGIN && point.y < screenContext->scr_y+screenContext->scr_height-MARGIN ) {
			draw_single_contact(screenContext, cr, aContact, &aContact->fromPair->color.rgbo);
		}
		bItem = bItem->next;
	}
	aItem = aItem->next;
  }
}


void draw_single_contact(struct ScreenContext *screenContext, cairo_t *cr, struct Contact *aContact, struct RGBO *color)
{
	struct Point point;

	if(aContact==NULL) return;
	
	gps_to_canvas(&screenContext->awin, aContact->gPoint.x, aContact->gPoint.y, &(point.x), &(point.y));
	cairo_arc(cr, point.x-screenContext->scr_x, point.y-screenContext->scr_y, CONTACT_RADIUS, 0, 2*M_PI);
	cairo_set_source_rgb(cr, color->red/255.0, color->green/255.0, color->blue/255.0);
	cairo_fill(cr);
}

void rubber_single_report(struct ScreenContext *screenContext, cairo_t *cr, struct Report *aReport)
{

}


void gps_to_canvas(struct WindowSize* awnd, double x, double y, double *rx, double *ry)
{
  double dxgps, dygps;
  double dxscrn, dyscrn;

  dxgps = awnd->cBox.xmax - awnd->cBox.xmin ;
  dygps = awnd->cBox.ymax - awnd->cBox.ymin ;

  dxscrn = awnd->sBox.xmax - awnd->sBox.xmin;
  dyscrn = awnd->sBox.ymax - awnd->sBox.ymin;

  if (greaterd(dxgps * dyscrn, dygps * dxscrn, DELTA) || equald(dxgps * dyscrn, dygps * dxscrn, DELTA)) 
	awnd->scale = dxscrn/dxgps;
  else
	awnd->scale = dyscrn/dygps;

  *ry = (awnd->cBox.ymax - y) * awnd->scale;
  *rx = (x - awnd->cBox.xmin) * awnd->scale;
}

double canvas_to_gps(struct WindowSize* awnd, double x, double y, double *rx, double *ry)
{
  double dxgps, dygps;
  double dxscrn, dyscrn;

  dxgps = awnd->cBox.xmax - awnd->cBox.xmin ;
  dygps = awnd->cBox.ymax - awnd->cBox.ymin ;

  dxscrn = awnd->sBox.xmax - awnd->sBox.xmin;
  dyscrn = awnd->sBox.ymax - awnd->sBox.ymin;

  if (greaterd(dxgps * dyscrn, dygps * dxscrn, DELTA) || equald(dxgps * dyscrn, dygps * dxscrn, DELTA)) 
	awnd->scale = dxgps/dxscrn;
  else
	awnd->scale = dygps/dyscrn;

  *rx = awnd->cBox.xmin + x*awnd->scale;
  *ry = awnd->cBox.ymax - y*awnd->scale;
  return awnd->scale;
}

void assign_window_size(struct WindowSize *target, struct WindowSize *source)
{
	target->cBox.xmin = source->cBox.xmin;
	target->cBox.xmax = source->cBox.xmax;
	target->cBox.ymin = source->cBox.ymin;
	target->cBox.ymax = source->cBox.ymax;
	target->sBox.xmin = source->sBox.xmin;
	target->sBox.xmax = source->sBox.xmax;
	target->sBox.ymin = source->sBox.ymin;
	target->sBox.ymax = source->sBox.ymax;
}

void show_text(struct ScreenContext *screenContext, char* text, int x, int y, int size)
{
	cairo_text_extents_t extents;

	cairo_select_font_face(screenContext->cr_on_screen, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(screenContext->cr_on_screen, size);
	cairo_text_extents(screenContext->cr_on_screen, text, &extents);

	gdk_draw_drawable(screenContext->drawingArea->window, screenContext->gc, screenContext->canvas, 
			  x-5, y-5, 
			  x-5, y-5, 
			  extents.width+10, extents.height+10);
	cairo_set_source_rgb(screenContext->cr_on_screen, 64.0/255, 0, 64.0/255);
	cairo_move_to(screenContext->cr_on_screen, x, y+extents.height);
	cairo_show_text(screenContext->cr_on_screen, text);
}
			


struct Colormap* load_colormap(char *filename)
{
	FILE *fp;
	char buffer[64];
	int i;
	struct Colormap *rt;

	if((fp=fopen(filename, "r"))==NULL) {
		printf("Cannot open colormap file!\n");
		return NULL;
	}

	rt = (struct Colormap*)malloc(sizeof(struct Colormap));

	fgets(buffer, 64, fp);
	sscanf(buffer, "%d", &(rt->nColors));
	rt->colors = (struct Color*)malloc(sizeof(struct Color)*rt->nColors);
	for (i = 0; i< rt->nColors; i++) {
		fgets(buffer, 64, fp);
		sscanf(buffer, "%lf %lf %lf", &(rt->colors[i].red), &(rt->colors[i].green), &(rt->colors[i].blue));
	}
	fclose(fp);
	return rt;
}


void free_colormap(struct Colormap *cmap)
{
	if(cmap!=NULL) {
		if(cmap->colors!=NULL)
	  		free(cmap->colors);
		free(cmap);
	}
}


void get_color(struct Colormap *cmap, struct Color *rtColor, double value, double lower, double upper)
{
	double whichColor;
	int index;
	
	if(cmap == NULL || (cmap!=NULL&&cmap->colors==NULL) || lower==upper) {
		rtColor->red =-1, rtColor->green = -1; rtColor->blue = -1;
		return;
	}
	whichColor = (value-lower)/(upper-lower) * cmap->nColors;
	if(whichColor<0) {
		rtColor->red =-1, rtColor->green = -1; rtColor->blue = -1;
		return;
	}
	if(whichColor >= cmap->nColors)
		whichColor = cmap->nColors-1;
	index = ceil(whichColor);	
	rtColor->red = cmap->colors[index].red;
	rtColor->green = cmap->colors[index].green;
	rtColor->blue = cmap->colors[index].blue;
}

