#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "color.h"
#include "common.h"
#include "trace.h"
#include "contact.h"
#include "gtk_cairo.h"
#include "events.h"
#include "files.h"
#include "busroute.h"
#include "node.h"

#define RECOGNITION_GRAN 5
#define MAXVALUE 100000

extern struct ScreenContext *screenContext;


gboolean forward(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  	if(screenContext->timeout != 0) 
		gtk_timeout_remove(screenContext->timeout);  
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(screenContext->backwardButton), FALSE);
		screenContext->playspeed = PLAY_SPEED;
		screenContext->timeout = gtk_timeout_add(1000/screenContext->playspeed, draw_forward, screenContext->forwardButton); 
	}

	return TRUE;
}

gboolean fast_forward(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(screenContext->forwardButton))) {
		if(screenContext->playspeed > 1000) return TRUE;
		screenContext->playspeed *=2;
		if(screenContext->timeout != 0) {
			g_print("playspeed: %dX\n", screenContext->playspeed/PLAY_SPEED);
			gtk_timeout_remove(screenContext->timeout);  
			screenContext->timeout = gtk_timeout_add(1000/screenContext->playspeed, draw_forward, screenContext->forwardButton); 
		}
		return TRUE;
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(screenContext->backwardButton))) {
		if(screenContext->playspeed == PLAY_SPEED) return TRUE;
		screenContext->playspeed /= 2;
		if(screenContext->timeout != 0) {
			g_print("playspeed: -%dX\n", screenContext->playspeed/PLAY_SPEED);
			gtk_timeout_remove(screenContext->timeout);  
			screenContext->timeout = gtk_timeout_add(1000/screenContext->playspeed, draw_backward, screenContext->backwardButton); 
		}
		return TRUE;
	}
	return TRUE;
}


gboolean backward(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(screenContext->timeout != 0) 
		gtk_timeout_remove(screenContext->timeout);  
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(screenContext->forwardButton), FALSE);
		screenContext->playspeed = PLAY_SPEED;
		screenContext->timeout = gtk_timeout_add(1000/screenContext->playspeed, draw_backward, screenContext->backwardButton); 
	}

	return TRUE;
}


gboolean fast_backward(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(screenContext->backwardButton))) {
		if(screenContext->playspeed > 1000) return TRUE;
		screenContext->playspeed *=2;
		if(screenContext->timeout != 0) {
			g_print("playspeed: -%dX\n", screenContext->playspeed/PLAY_SPEED);
			gtk_timeout_remove(screenContext->timeout);  
			screenContext->timeout = gtk_timeout_add(1000/screenContext->playspeed, draw_backward, screenContext->backwardButton); 
		}
		return TRUE;
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(screenContext->forwardButton))) {
		if(screenContext->playspeed == PLAY_SPEED) return TRUE;
		screenContext->playspeed /= 2;
		if(screenContext->timeout != 0) {
			g_print("playspeed: %dX\n", screenContext->playspeed/PLAY_SPEED);
			gtk_timeout_remove(screenContext->timeout);  
			screenContext->timeout = gtk_timeout_add(1000/screenContext->playspeed, draw_forward, screenContext->forwardButton); 
		}
		return TRUE;
	}
	return TRUE;
}

/* replay selected */
gboolean replay(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	screenContext->atClock = screenContext->startAt; 

	set_selected_traces_time(&screenContext->selectedTraces, screenContext->atClock);
	set_selected_contacts_time(&screenContext->selected, screenContext->contactTableMode, screenContext->atClock);
	set_cell_table_time(&screenContext->cellTable, screenContext->atClock);
	
	draw_dynamic_on_screen(screenContext);
	return TRUE;
}

/* input start timestamp */
void enter_from_time(GtkWidget *widget, GtkWidget *entry)
{
  const gchar *entry_text;
  entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
  
  screenContext->startAt = strtot(entry_text);
  replay(NULL, NULL, NULL);
}

/* input stop timestamp */
void enter_until_time(GtkWidget *widget, GtkWidget *entry)
{
  const gchar *entry_text;
  entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
  
  screenContext->endAt = strtot(entry_text);
}

/* input x */
void enter_x(GtkWidget *widget, GtkWidget *entry)
{
  const gchar *entry_text;
  double x0, y0;
  double zoom_scale = FIND_SCALE/screenContext->scale;

  entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
  
  screenContext->iPoint.x = atof(entry_text);
  if(is_point_in_box(&screenContext->iPoint, &screenContext->awin.cBox)) {
	gps_to_canvas(&screenContext->awin, screenContext->iPoint.x, screenContext->iPoint.y, &x0, &y0);
	screenContext->scale = FIND_SCALE;
	screenContext->awin.sBox.xmax = screenContext->awin.sBox.xmax*zoom_scale;
	screenContext->awin.sBox.ymax = screenContext->awin.sBox.ymax*zoom_scale;
	setup_drawing_content(screenContext, MAX(x0*zoom_scale-0.5*screenContext->scr_width, -MARGIN), MAX(y0*zoom_scale-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
	draw_canvas(screenContext);
  }
}

/* input y */
void enter_y(GtkWidget *widget, GtkWidget *entry)
{
  const gchar *entry_text;
  double x0, y0;
  double zoom_scale = FIND_SCALE/screenContext->scale;

  entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
  
  screenContext->iPoint.y = atof(entry_text);
  if(is_point_in_box(&screenContext->iPoint, &screenContext->awin.cBox)) {
	gps_to_canvas(&screenContext->awin, screenContext->iPoint.x, screenContext->iPoint.y, &x0, &y0);
	screenContext->scale = FIND_SCALE;
	screenContext->awin.sBox.xmax = screenContext->awin.sBox.xmax*zoom_scale;
	screenContext->awin.sBox.ymax = screenContext->awin.sBox.ymax*zoom_scale;
	setup_drawing_content(screenContext, MAX(x0*zoom_scale-0.5*screenContext->scr_width, -MARGIN), MAX(y0*zoom_scale-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
	draw_canvas(screenContext);
  }
}

void roadlist_tree_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gchar *name;
	int number;
  	double  x0, y0;

	struct Item *aItem;
	struct Road *aRoad;

	struct RGBO color;
	double zoom_scale = FIND_SCALE/screenContext->scale;

        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                gtk_tree_model_get (model, &iter, COLUMN_RD_NAME, &name, -1);
		number = atoi(name);
		aItem = duallist_find(&screenContext->region->roads, &number, (int(*)(void*, void*))road_has_id);
		aRoad = (struct Road*)aItem->datap;
		gps_to_canvas(&screenContext->awin, (aRoad->box.xmin + aRoad->box.xmax)/2, (aRoad->box.ymin + aRoad->box.ymax)/2, &x0, &y0);
		screenContext->scale = FIND_SCALE;
		screenContext->awin.sBox.xmax = screenContext->awin.sBox.xmax*zoom_scale;
		screenContext->awin.sBox.ymax = screenContext->awin.sBox.ymax*zoom_scale;
		setup_drawing_content(screenContext, MAX(x0*zoom_scale-0.5*screenContext->scr_width, -MARGIN), MAX(y0*zoom_scale-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
  		draw_canvas(screenContext);

		color.red = 220, color.green = 0, color.blue = 0, color.opacity = 255;
		draw_single_road(screenContext, screenContext->cr_on_screen, aRoad, &color);
                g_free (name);
        }
}

void crosslist_tree_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gchar *name;
	int number;
  	double  x0, y0;

	struct Item *aItem;
	struct Cross *aCross;

	struct RGBO color;
	double zoom_scale = FIND_SCALE/screenContext->scale;

        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
                gtk_tree_model_get (model, &iter, COLUMN_RD_NAME, &name, -1);
		number = atoi(name);
		aItem = duallist_find(&screenContext->region->crosses, &number, (int(*)(void*, void*))cross_has_number);
		aCross = (struct Cross*)aItem->datap;
		gps_to_canvas(&screenContext->awin, aCross->gPoint.x, aCross->gPoint.y, &x0, &y0);
		screenContext->scale = FIND_SCALE;
		screenContext->awin.sBox.xmax = screenContext->awin.sBox.xmax*zoom_scale;
		screenContext->awin.sBox.ymax = screenContext->awin.sBox.ymax*zoom_scale;
		setup_drawing_content(screenContext, MAX(x0*zoom_scale-0.5*screenContext->scr_width, -MARGIN), MAX(y0*zoom_scale-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
  		draw_canvas(screenContext);

		color.red = 220, color.green = 0, color.blue = 0, color.opacity = 255;
		draw_single_cross(screenContext, screenContext->cr_on_screen, aCross, &color);
                g_free (name);
        }
}


/* bus routes */

void route_selected(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean toDraw;
  gchar* name;
  struct Item *aItem;
  struct Busroute *aRoute;
  struct Point point;

  // get toggled iter 
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_TODRAW, &toDraw, COLUMN_NAME, &name, -1);

  // do something with the value 
  toDraw ^= 1;

  // set new value 
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_TODRAW, toDraw, -1);
  aItem = hashtable_find(&screenContext->routeTable, name);

  aRoute = (struct Busroute*)aItem->datap;
  if(toDraw != FALSE) {
	duallist_add_to_tail(&screenContext->selectedRoutes, aRoute);
	gps_to_canvas(&screenContext->awin, (aRoute->box.xmin+aRoute->box.xmax)/2, (aRoute->box.ymin+aRoute->box.ymax)/2, &(point.x), &(point.y));
	if(point.x > screenContext->scr_x+MARGIN && point.x < screenContext->scr_x+screenContext->scr_width-MARGIN && point.y > screenContext->scr_y+MARGIN && point.y < screenContext->scr_y+screenContext->scr_height-MARGIN) {
		draw_single_route(screenContext, screenContext->cr_on_screen, aRoute);
	} else {
		setup_drawing_content(screenContext, MAX(point.x-0.5*screenContext->scr_width, -MARGIN), MAX(point.y-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
	}
  } else {
	duallist_pick(&screenContext->selectedRoutes, name, (int(*)(void*, void*))route_has_name);
	draw_dynamic_on_screen(screenContext);
  }

  gtk_tree_path_free(path);
  g_free(name);
}

void routelist_tree_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gchar *name;
	struct Item *aItem;
	struct Busroute *aRoute;
	struct Point point;

        if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
                gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);
		aItem = duallist_find(&screenContext->selectedRoutes, name, (int(*)(void*, void*))route_has_name);
		if(aItem != NULL) {
			aRoute = (struct Busroute*)aItem->datap;
			gps_to_canvas(&screenContext->awin, (aRoute->box.xmin+aRoute->box.xmax)/2, (aRoute->box.ymin+aRoute->box.ymax)/2, &(point.x), &(point.y));
			if(point.x > screenContext->scr_x+MARGIN && point.x < screenContext->scr_x+screenContext->scr_width-MARGIN && point.y > screenContext->scr_y+MARGIN && point.y < screenContext->scr_y+screenContext->scr_height-MARGIN) {
			} else {
				setup_drawing_content(screenContext, MAX(point.x-0.5*screenContext->scr_width, -MARGIN), MAX(point.y-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
				draw_canvas(screenContext);
			}
		}
                g_free (name);
        }

}

gboolean select_all_route(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	struct Item *aItem;
	struct Busroute *aRoute;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		unsigned long at;
		for (at = 0; at<screenContext->routeTable.size;at++) {
			aItem = screenContext->routeTable.head[at];
			while(aItem != NULL) {
				aRoute = (struct Busroute*)aItem->datap;
				duallist_add_to_tail(&screenContext->selectedRoutes, aRoute);
			//	draw_single_route(screenContext, screenContext->cr_on_screen, aRoute);
				aItem = aItem->next;
			}
		}
		draw_selected_routes(screenContext, screenContext->cr_on_screen);
	} else {
		while(!is_duallist_empty(&screenContext->selectedRoutes)) {
			aRoute = duallist_pick_head(&screenContext->selectedRoutes);
		}
		draw_dynamic_on_screen(screenContext);

	}
	return TRUE;
}

gboolean select_coverage(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	struct Item *aItem, *bItem;
	struct Busroute *aRoute;
	struct Duallist *cellList;
	struct RGBO fillColor;

	fillColor.red = 250;
	fillColor.green = 150;
	fillColor.blue = 100;
	fillColor.opacity = 255;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		aItem = screenContext->selectedRoutes.head;
		while(aItem) {
			aRoute = (struct Busroute*)aItem->datap;
			cellList = get_route_coverage(screenContext->region, aRoute);
			if(cellList) {
				bItem = cellList->head;
				while(bItem) {
					draw_single_cell(screenContext, screenContext->cr_on_screen, (struct Cell*)bItem->datap, &fillColor);
					bItem = bItem->next;
				}
				duallist_destroy(cellList, NULL);
			}
			aItem = aItem->next;
		}
	} else {
		draw_dynamic_on_screen(screenContext);
	}
	return TRUE;
}

/* traces */

void trace_selected(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean toDraw;
  gchar* name;
  struct Item *aItem;
  struct Item *bItem;
  struct Trace *aTrace;

  // get toggled iter 
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_TODRAW, &toDraw, COLUMN_NAME, &name, -1);

  // do something with the value 
  toDraw ^= 1;

  aItem = hashtable_find(&screenContext->traceTable, name);

  aTrace = (struct Trace*)aItem->datap;
  // set new value 
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_TODRAW, toDraw, -1);
  if(toDraw != FALSE) {
	duallist_add_to_tail(&screenContext->selectedTraces, aTrace);

	bItem = aTrace->reports.head;
	while(bItem!=NULL && ((struct Report*)bItem->datap)->timestamp < screenContext->atClock) 
		bItem = bItem->next;
	aTrace->at = bItem;
	if(aTrace->at != NULL)
		aTrace->countdown = ((struct Report*)bItem->datap)->timestamp - screenContext->atClock;
	else
		aTrace->countdown =  -1;

	screenContext->focusTrace = aTrace;
  } else {
	aTrace = duallist_pick(&screenContext->selectedTraces, name, (int(*)(void*, void*))trace_has_name);
	bItem = aTrace->reports.head;
	while(bItem != NULL) {
		((struct Report*)bItem->datap)->shown = 0;
		bItem = bItem->next;
	}
	draw_dynamic_on_screen(screenContext);
  }

  gtk_tree_path_free(path);
  g_free(name);
}

void tracelist_tree_selection_changed(GtkTreeSelection *selection, gpointer data)
{
        GtkTreeIter iter;
        GtkTreeModel *model;
        gchar *name;
	struct Item *aItem;

        if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
                gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);
		aItem = duallist_find(&screenContext->selectedTraces, name, (int(*)(void*, void*))trace_has_name);
		if(aItem != NULL)
			screenContext->focusTrace = (struct Trace*)aItem->datap;
                g_free (name);
        }

}

gboolean select_all_trace(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	struct Item *aItem;
	struct Trace *aTrace;
	struct Item *bItem;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		unsigned long at;
		for (at = 0; at<screenContext->traceTable.size;at++) {
			aItem = screenContext->traceTable.head[at];
			while(aItem != NULL) {
				aTrace = (struct Trace*)aItem->datap;
				duallist_add_to_tail(&screenContext->selectedTraces, aTrace);

				bItem = aTrace->reports.head;
				while(bItem!=NULL && ((struct Report*)bItem->datap)->timestamp < screenContext->atClock) 
					bItem = bItem->next;
				aTrace->at = bItem;
				if(bItem != NULL)
					aTrace->countdown = ((struct Report*)bItem->datap)->timestamp - screenContext->atClock;
				else
					aTrace->countdown =  -1;

				screenContext->focusTrace = aTrace;
				aItem = aItem->next;
			}
		}
	} else {
		while(!is_duallist_empty(&screenContext->selectedTraces)) {
			aTrace = duallist_pick_head(&screenContext->selectedTraces);
			bItem = aTrace->reports.head;
			while(bItem != NULL) {
				((struct Report*)bItem->datap)->shown = 0;
				bItem = bItem->next;
			}
		}
		draw_dynamic_on_screen(screenContext);
	}
	return TRUE;
}


void contact_selected(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean toDraw;
  gchar* name;
  struct Item *aItem;
  struct Item *bItem;
  struct Pair *aPair;
  struct Ego *anEgo;
  struct Linkman *aLinkman;

  // get toggled iter 
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_TODRAW, &toDraw, COLUMN_NAME, &name, -1);

  // do something with the value 
  toDraw ^= 1;

  aItem = hashtable_find(&screenContext->contactTable, name);
  // set new value 
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_TODRAW, toDraw, -1);

  if(screenContext->contactTableMode == PAIRWISE_TABLE) {
	  aPair = (struct Pair*)aItem->datap;
	  if(toDraw != FALSE) {
		duallist_add_to_tail(&screenContext->selected, aPair);
		bItem = aPair->contents.head;
		while(bItem!=NULL && ((struct Contact*)bItem->datap)->startAt < screenContext->atClock) 
			bItem = bItem->next;
		aPair->at = bItem;
		if(aPair->at != NULL)
			aPair->countdown = ((struct Contact*)bItem->datap)->startAt - screenContext->atClock;
		else
			aPair->countdown =  -1;

	  } else {
		aPair = duallist_pick(&screenContext->selected, name, (int(*)(void*, void*))pair_has_names);
		bItem = aPair->contents.head;
		while(bItem != NULL) {
			((struct Contact*)bItem->datap)->shown = 0;
			bItem = bItem->next;
		}
		draw_dynamic_on_screen(screenContext);
	  }
  } else {
	  anEgo = (struct Ego*)aItem->datap;
	  if(toDraw != FALSE) {
		  duallist_add_to_tail(&screenContext->selected, anEgo);
		  aItem = anEgo->linkmen.head;
		  while(aItem) {
			aLinkman = (struct Linkman*)aItem->datap;
			bItem = aLinkman->contacts.head;
			while(bItem!=NULL && ((struct Contact*)bItem->datap)->startAt < screenContext->atClock) 
				bItem = bItem->next;
			aLinkman->at = bItem;
			if(aLinkman->at != NULL)
				aLinkman->countdown = ((struct Contact*)bItem->datap)->startAt - screenContext->atClock;
			else
				aLinkman->countdown =  -1;

			aItem = aItem->next;
		  }

	  } else {
		anEgo = duallist_pick(&screenContext->selected, name, (int(*)(void*, void*))ego_has_name);
		aItem = anEgo->linkmen.head;
		while(aItem) {
			aLinkman = (struct Linkman *)aItem->datap;
			bItem = aLinkman->contacts.head;
			while(bItem != NULL) {
				((struct Contact*)bItem->datap)->shown = 0;
				bItem = bItem->next;
			}
			aItem = aItem->next;
		}
		draw_dynamic_on_screen(screenContext);
	  }
  }
  gtk_tree_path_free(path);
  g_free(name);
}

gboolean select_all_contact(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	struct Item *aItem;
	struct Pair *aPair;
	struct Ego *anEgo;
	struct Linkman *aLinkman;
	struct Item *bItem, *cItem;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		unsigned long at;
		for (at = 0; at<screenContext->contactTable.size;at++) {
			aItem = screenContext->contactTable.head[at];
			while(aItem != NULL) {
  				if(screenContext->contactTableMode == PAIRWISE_TABLE) {
					aPair = (struct Pair*)aItem->datap;
					duallist_add_to_tail(&screenContext->selected, aPair);

					bItem = aPair->contents.head;
					while(bItem!=NULL && ((struct Contact*)bItem->datap)->startAt < screenContext->atClock) 
						bItem = bItem->next;
					aPair->at = bItem;
					if(bItem != NULL)
						aPair->countdown = ((struct Contact*)bItem->datap)->startAt - screenContext->atClock;
					else
						aPair->countdown =  -1;
				} else {
					anEgo = (struct Ego*)aItem->datap;
					duallist_add_to_tail(&screenContext->selected, anEgo);
					cItem = anEgo->linkmen.head;
					while(cItem) {
						aLinkman = (struct Linkman*)cItem->datap;
						bItem = aLinkman->contacts.head;
						while(bItem!=NULL && ((struct Contact*)bItem->datap)->startAt < screenContext->atClock) 
							bItem = bItem->next;
						aLinkman->at = bItem;
						if(aLinkman->at != NULL)
							aLinkman->countdown = ((struct Contact*)bItem->datap)->startAt - screenContext->atClock;
						else
							aLinkman->countdown =  -1;

						cItem = cItem->next;
					}

				}
				aItem = aItem->next;
			}
		}
	} else {
		while(!is_duallist_empty(&screenContext->selected)) {
  			if(screenContext->contactTableMode == PAIRWISE_TABLE) {
				aPair = duallist_pick_head(&screenContext->selected);
				bItem = aPair->contents.head;
				while(bItem != NULL) {
					((struct Contact*)bItem->datap)->shown = 0;
					bItem = bItem->next;
				}
			} else {
				anEgo = duallist_pick_head(&screenContext->selected);
				aItem = anEgo->linkmen.head;
				while(aItem) {
					aLinkman = (struct Linkman *)aItem->datap;
					bItem = aLinkman->contacts.head;
					while(bItem != NULL) {
						((struct Contact*)bItem->datap)->shown = 0;
						bItem = bItem->next;
					}
					aItem = aItem->next;
				}
			}
		}
		draw_dynamic_on_screen(screenContext);
	}
	return TRUE;
}

gboolean draw_rsu_topology(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		draw_links(screenContext);
		draw_rsus(screenContext);
	} else {
		draw_dynamic_on_screen(screenContext);
	}
	return TRUE;
}

gboolean draw_storage_nodes(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  	struct RGBO color;
	struct Item *aItem;
	struct Node *aNode;
	unsigned long i;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		color.red = 100;
		color.green = 0;
		color.blue = 255;
		color.opacity = 255;
		if(screenContext->nodeTable.count) {
			for (i = 0; i<screenContext->nodeTable.size; i++) {
				aItem = screenContext->nodeTable.head[i];
				while(aItem != NULL ) {
					aNode = (struct Node*)aItem->datap;
					draw_single_storage_node(screenContext, screenContext->cr_on_screen, aNode, &color);
					aItem = aItem->next;
				}
			}
		}
	} else {
		draw_dynamic_on_screen(screenContext);
	}
	return TRUE;
}

gboolean draw_forward(gpointer data)
{
	struct Point point;
	char attime[32];
	char buf[64];
	struct Item *aItem, *bItem;
	struct Trace *aSelectedTrace;
	struct Pair *aSelectedPair;
	struct Ego *aSelectedEgo;
	struct Linkman *aLinkman;
  	struct RGBO color;
	struct Report *lastReport;
	struct Cell *aCell;
	unsigned long i;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON((GtkWidget*)data)) ) {
		ttostr(screenContext->atClock, attime);
		sprintf(buf, "Clock: %s (%u)", attime, (unsigned)screenContext->atClock);
		show_text(screenContext, buf, MARGIN+20, MARGIN+20, FONT_SIZE);

		/* display cell dynamic displays */
		for(i = 0; i<screenContext->cellTable.size; i++) {
			aItem = screenContext->cellTable.head[i];
			while (aItem!=NULL)
			{
				aCell = (struct Cell*)aItem->datap;
				if(aCell->at != NULL && aCell->countdown == 0) {
					do {
						if( aCell->at == aCell->displays.head
						 || (aCell->at != aCell->displays.head
						 && ((struct Display*)aCell->at->datap)->value != ((struct Display*)aCell->at->prev->datap)->value)) {
							draw_cell_display(screenContext, screenContext->cr_on_screen, aCell);
						} 
						aCell->at = aCell->at->next;
					} while(aCell->at != NULL && ((struct Display*)aCell->at->datap)->timestamp == ((struct Display*)aCell->at->prev->datap)->timestamp);

					if (aCell->at != NULL)
						aCell->countdown = difftime(((struct Display*)aCell->at->datap)->timestamp, screenContext->atClock);
					else
						aCell->countdown = -1;
				} else if (aCell->at != NULL && aCell->countdown > 0) 
					aCell->countdown --;

				aItem = aItem->next;
			}
		}

		/*play GPS reports*/
		aItem = screenContext->selectedTraces.head;
		while(aItem != NULL) {
			aSelectedTrace = (struct Trace*)aItem->datap;
			if(aSelectedTrace->at != NULL && aSelectedTrace->countdown == 0) {
				do {
					if( aSelectedTrace == screenContext->focusTrace) {
						gps_to_canvas(&screenContext->awin, ((struct Report*)screenContext->focusTrace->at->datap)->gPoint.x, ((struct Report*)screenContext->focusTrace->at->datap)->gPoint.y, &(point.x), &(point.y));
						if(point.x > screenContext->scr_x+MARGIN && point.x < screenContext->scr_x+screenContext->scr_width-MARGIN && point.y > screenContext->scr_y+MARGIN && point.y < screenContext->scr_y+screenContext->scr_height-MARGIN) {
						} else {
							setup_drawing_content(screenContext, MAX(point.x-0.5*screenContext->scr_width, -MARGIN), MAX(point.y-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
							draw_canvas(screenContext);
						}
					}
					if(aSelectedTrace->at!=aSelectedTrace->reports.head) {
						lastReport = (struct Report*)aSelectedTrace->at->prev->datap;
						draw_single_report(screenContext, screenContext->cr_on_screen, lastReport, &aSelectedTrace->color.rgbo);
					} 
					color.red = 255, color.green = 255, color.blue = 0, color.opacity = 255;
					draw_single_report(screenContext, screenContext->cr_on_screen, (struct Report*)aSelectedTrace->at->datap, &color);
					((struct Report*)aSelectedTrace->at->datap)->shown = 1;
					aSelectedTrace->at = aSelectedTrace->at->next;
				} while(aSelectedTrace->at != NULL && ((struct Report*)aSelectedTrace->at->datap)->timestamp == ((struct Report*)aSelectedTrace->at->prev->datap)->timestamp);

				if (aSelectedTrace->at != NULL)
					aSelectedTrace->countdown = difftime(((struct Report*)aSelectedTrace->at->datap)->timestamp, screenContext->atClock);
				else
					aSelectedTrace->countdown = -1;
			} else if (aSelectedTrace->at != NULL && aSelectedTrace->countdown > 0) 
				aSelectedTrace->countdown --;
			aItem = aItem->next;
		}


		/* play contacts */
		aItem = screenContext->selected.head;
		while(aItem != NULL) {
			if(screenContext->contactTableMode == PAIRWISE_TABLE) {
				aSelectedPair = (struct Pair*)aItem->datap;
				if(aSelectedPair->at != NULL && aSelectedPair->countdown == 0) {
					do {
						draw_single_contact(screenContext, screenContext->cr_on_screen, (struct Contact*)aSelectedPair->at->datap, &aSelectedPair->color.rgbo);

						((struct Contact*)aSelectedPair->at->datap)->shown = 1;
						aSelectedPair->at = aSelectedPair->at->next;
					} while(aSelectedPair->at != NULL && ((struct Contact*)aSelectedPair->at->datap)->startAt == ((struct Contact*)aSelectedPair->at->prev->datap)->startAt);

					if (aSelectedPair->at != NULL)
						aSelectedPair->countdown = difftime(((struct Contact*)aSelectedPair->at->datap)->startAt, screenContext->atClock);
					else
						aSelectedPair->countdown = -1;
				} else if (aSelectedPair->at != NULL && aSelectedPair->countdown > 0) 
					aSelectedPair->countdown --;
				aItem = aItem->next;
			} else {
				aSelectedEgo = (struct Ego*)aItem->datap;
				bItem = aSelectedEgo->linkmen.head;
				while(bItem) {
					aLinkman = (struct Linkman *)bItem->datap;
					if(aLinkman->at != NULL && aLinkman->countdown == 0) {
						do {
							draw_single_contact(screenContext, screenContext->cr_on_screen, (struct Contact*)aLinkman->at->datap, &aLinkman->color.rgbo);

							((struct Contact*)aLinkman->at->datap)->shown = 1;
							aLinkman->at = aLinkman->at->next;
						} while(aLinkman->at != NULL && ((struct Contact*)aLinkman->at->datap)->startAt == ((struct Contact*)aLinkman->at->prev->datap)->startAt);

						if (aLinkman->at != NULL)
							aLinkman->countdown = difftime(((struct Contact*)aLinkman->at->datap)->startAt, screenContext->atClock);
						else
							aLinkman->countdown = -1;
					} else if (aLinkman->at != NULL && aLinkman->countdown > 0) 
						aLinkman->countdown --;
						
					bItem = bItem->next;
				}
				aItem = aItem->next;
			}

		}

		screenContext->atClock ++;
	}

	if(screenContext->atClock > screenContext->endAt) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((GtkWidget*)data), FALSE);
		return TRUE;
	}
	return TRUE;
}


gboolean draw_backward(gpointer data)
{
	struct Point point;
	char attime[32];
	char buf[64];
	struct Item *aItem;
	struct Trace *aSelected;
  	struct RGBO color;
	struct Report *lastReport;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON((GtkWidget*)data)) ) {
		ttostr(screenContext->atClock, attime);
		sprintf(buf, "Clock: %s (%u)", attime, (unsigned)screenContext->atClock);
		show_text(screenContext, buf, MARGIN+20, MARGIN+20, FONT_SIZE);

		aItem = screenContext->selectedTraces.head;
		while(aItem != NULL) {
			aSelected = (struct Trace*)aItem->datap;
			if(aSelected->at != NULL && aSelected->countdown == 0) {
				while(difftime(((struct Report*)aSelected->at->datap)->timestamp, screenContext->atClock) == 0) {
					if( aSelected == screenContext->focusTrace) {
						gps_to_canvas(&screenContext->awin, ((struct Report*)screenContext->focusTrace->at->datap)->gPoint.x, ((struct Report*)screenContext->focusTrace->at->datap)->gPoint.y, &(point.x), &(point.y));
						if(point.x > screenContext->scr_x+MARGIN && point.x < screenContext->scr_x+screenContext->scr_width-MARGIN && point.y > screenContext->scr_y+MARGIN && point.y < screenContext->scr_y+screenContext->scr_height-MARGIN) {
						} else {
							setup_drawing_content(screenContext, MAX(point.x-0.5*screenContext->scr_width, -MARGIN), MAX(point.y-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
							draw_canvas(screenContext);
						}
					}
					if(aSelected->at->prev!=NULL) {
						lastReport = (struct Report*)aSelected->at->prev->datap;
						color.red = 255, color.green = 255, color.blue = 0, color.opacity = 255;
						draw_single_report(screenContext, screenContext->cr_on_screen, lastReport, &color);
					} 
					rubber_single_report(screenContext, screenContext->cr_on_screen, (struct Report*)aSelected->at->datap);
					((struct Report*)aSelected->at->datap)->shown = 0;
					aSelected->at = aSelected->at->prev;
				}
				if (aSelected->at != NULL)
					aSelected->countdown = difftime(screenContext->atClock, ((struct Report*)aSelected->at->datap)->timestamp);
				else
					aSelected->countdown = -1;
			} else if (aSelected->at != NULL && aSelected->countdown > 0) 
				aSelected->countdown --;
			aItem = aItem->next;
		}
		screenContext->atClock --;
	}

	if(screenContext->atClock < screenContext->startAt) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((GtkWidget*)data), FALSE);
		return TRUE;
	}
	return TRUE;
}

/* mouse motion event */
gboolean
drawing_area_motion_event (GtkWidget  *widget,
			   GdkEventMotion *event,
			   gpointer data)
{
  /* x,y is the coordinate in the virtual pixmap */
  int x, y;
  struct Point point;

  x = event->x, y = event->y;
  if (event->is_hint)
	gdk_window_get_pointer(event->window, &x, &y, NULL);
  x = x+screenContext->scr_x, y = y+screenContext->scr_y;
 
  canvas_to_gps(&screenContext->awin, x, y, &point.x, &point.y); 

  /* the mouse is in our display area */
  if( x>=MARGIN+screenContext->scr_x && x<=screenContext->scr_width-MARGIN+screenContext->scr_x && y>=MARGIN+screenContext->scr_y && y<=screenContext->scr_height-MARGIN+screenContext->scr_y) {
	rub_bar(widget->window, screenContext->gc, MARGIN);
	brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	screenContext->areaOnScreen = 0;

	double dist, mindist;
	struct Segment seg;
	struct Point interpoint;
	struct Point aPoint;

	struct Road *prevRoad = NULL;
	struct Cell *prevCell = NULL;
	struct Report *prevReport = NULL;
	struct Cross *prevCross = NULL;

	struct Item *aItem;
	struct Report *aReport;

	struct Item *bItem;
	struct Road *aRoad;

	struct Item *cItem;
	struct Cell *aCell;

	struct Item *dItem;
	struct Cross *aCross;

	char buf[128];
	char attime[20];
	struct RGBO color;

	struct District *aDistrict;
	/* show the coordinates of the mouse */
	sprintf(buf, "(%10.6lf, %10.6lf)", point.x, point.y);
	show_text(screenContext, buf, screenContext->scr_width-MARGIN-200, MARGIN+20, FONT_SIZE);

	/* show other objects on map*/
	prevCell = screenContext->mouseAtCell;

	duallist_destroy(&screenContext->surroundings, NULL);
	surroundings_from_point(&screenContext->surroundings, screenContext->region, &point);	
	cItem = screenContext->surroundings.head;
	if(cItem == NULL) return TRUE;
	aCell = (struct Cell*)cItem->datap;

	/* selecting surrounding cells */
	screenContext->mouseAtCell = aCell;
	if((event->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) {
		draw_cells(screenContext, screenContext->cr_on_screen);
		return TRUE;
	}

	/* selecting district */
	aDistrict = point_in_district(screenContext->region, &point);
	if(aDistrict != NULL && (event->state & 8)) {
		show_district(screenContext, aDistrict, screenContext->cr_on_screen);
		return TRUE;
	}

	/* selecting a report */
	mindist = MAXVALUE;
	prevReport = screenContext->mouseAtReport;
	screenContext->mouseAtReport = NULL;
	aItem = aCell->reps.head;
	while(aItem != NULL) {
		aReport = (struct Report*)aItem->datap;
		gps_to_canvas(&screenContext->awin, aReport->gPoint.x, aReport->gPoint.y, &(aPoint.x), &(aPoint.y));
		if((dist = distance_in_pixel(x, y, aPoint.x, aPoint.y)) < mindist && dist < RECOGNITION_GRAN && aReport->shown) {
			mindist = dist;
			screenContext->mouseAtReport = aReport;
		}
		aItem = aItem->next;
	}

	if(screenContext->mouseAtReport != prevReport) {
		if(prevReport != NULL) {
			draw_single_report(screenContext, screenContext->cr_on_screen, prevReport, &prevReport->fromTrace->color.rgbo);

			gdk_draw_drawable(widget->window, screenContext->gc, screenContext->canvas, 
					  MARGIN+20, screenContext->scr_height-MARGIN-100, 
					  MARGIN+20, screenContext->scr_height-MARGIN-100, 
					  400, 100);
		}
		if(screenContext->mouseAtReport != NULL) {
			color.red = 0;
			color.green = 0;
			color.blue = 0;
			color.opacity = 255;
			draw_single_report(screenContext, screenContext->cr_on_screen, screenContext->mouseAtReport, &color);

			sprintf(buf, "Vehicle ID: %s", screenContext->mouseAtReport->fromTrace->vName);
			show_text(screenContext, buf, MARGIN+20, screenContext->scr_height-MARGIN-100, FONT_SIZE);
			ttostr(screenContext->mouseAtReport->timestamp, attime);
			sprintf(buf, "timestamp: %s(%ld)", attime, screenContext->mouseAtReport->timestamp);
			show_text(screenContext, buf, MARGIN+20, screenContext->scr_height-MARGIN-80, FONT_SIZE);
			sprintf(buf, "speed: %d, heading: %d MsgType: %d", screenContext->mouseAtReport->speed, screenContext->mouseAtReport->angle, screenContext->mouseAtReport->msgType);
			show_text(screenContext, buf, MARGIN+20, screenContext->scr_height-MARGIN-60, FONT_SIZE);
			if(screenContext->mouseAtReport->fromTrace->type == FILE_ORIGINAL_GPS_BUS ||
			   screenContext->mouseAtReport->fromTrace->type == FILE_MODIFIED_GPS_BUS) {
				sprintf(buf, "OnRoute: %s GPS:%s Parking:%s Dest:%s Direction:%s Door:%s Stop:%s",
					screenContext->mouseAtReport->fromTrace->onRoute,
					(screenContext->mouseAtReport->state & 0x80) == 0?"Y":"X",
					(screenContext->mouseAtReport->state & 0x40) == 0?"No":"Yes",
					(screenContext->mouseAtReport->state & 0x20) == 0?"No":"Yes",
					(screenContext->mouseAtReport->state & 0x10) == 0?"Upway":"Downway",
					(screenContext->mouseAtReport->state & 0x08) == 0?"Closed":"Open",
					(screenContext->mouseAtReport->state & 0x04) == 0?"Leaving":"Arriving");
				show_text(screenContext, buf, MARGIN+20, screenContext->scr_height-MARGIN-40, FONT_SIZE);
			}
			if(screenContext->mouseAtReport->onRoad!=NULL) {
				sprintf(buf, "onRoad: %d", ((struct CandRoad*)screenContext->mouseAtReport->onRoad->datap)->aRoad->id);
				show_text(screenContext, buf, MARGIN+20, screenContext->scr_height-MARGIN-20, FONT_SIZE);
			}
		}
	}

	if((event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK) {
		/* selecting a cross */
		prevCross = screenContext->mouseAtCross;
		dItem = aCell->crosses.head;
		screenContext->mouseAtCross = NULL;
		while(dItem != NULL) {
			aCross = (struct Cross*)dItem->datap;
			if(point.x >= aCross->box.xmin+(aCross->box.xmax-aCross->box.xmin)/4 && point.x <= aCross->box.xmax-(aCross->box.xmax-aCross->box.xmin)/4
			&& point.y >= aCross->box.ymin+(aCross->box.ymax-aCross->box.ymin)/4 && point.y <= aCross->box.ymax-(aCross->box.ymax-aCross->box.ymin)/4) {
				screenContext->mouseAtCross = aCross;
				break;
			}
			dItem = dItem->next;
		}

		if(screenContext->mouseAtCross != prevCross) {
			if(prevCross != NULL) {
				color.red = 255;
				color.green = 255;
				color.blue = 255;
				color.opacity = 255;
				draw_single_cross(screenContext, screenContext->cr_on_screen, prevCross, &color); 
				
				color.red = 255;
				color.green = 255;
				color.blue = 255;
				color.opacity = 255;
				bItem = prevCross->inRoads.head;
				while(bItem != NULL) {
					draw_single_road(screenContext, screenContext->cr_on_screen, bItem->datap, &color);
					bItem = bItem->next;
				}
				bItem = prevCross->outRoads.head;
				while(bItem != NULL) {
					draw_single_road(screenContext, screenContext->cr_on_screen, bItem->datap, &color);
					bItem = bItem->next;
				}

				gdk_draw_drawable(widget->window, screenContext->gc, screenContext->canvas, 
						  screenContext->scr_width-MARGIN-200, MARGIN+40, 
						  screenContext->scr_width-MARGIN-200, MARGIN+40, 
						  200, 100);
			}
			if(screenContext->mouseAtCross != NULL) {
				if(prevRoad != NULL) {
					color.red = 255;
					color.green = 255;
					color.blue = 255;
					color.opacity = 255;
					draw_single_road(screenContext, screenContext->cr_on_screen, prevRoad, &color);
				}
				gdk_draw_drawable(widget->window, screenContext->gc, screenContext->canvas, 
						  screenContext->scr_width-MARGIN-200, MARGIN+40, 
						  screenContext->scr_width-MARGIN-200, MARGIN+40, 
						  200, 100);
				sprintf(buf, "Cross No. %d", screenContext->mouseAtCross->number);
				show_text(screenContext, buf, screenContext->scr_width-MARGIN-200, MARGIN+40, FONT_SIZE);

				color.red = 200;
				color.green = 100;
				color.blue = 100;
				color.opacity = 255;
				draw_single_cross(screenContext, screenContext->cr_on_screen, screenContext->mouseAtCross, &color); 

				color.red = 200;
				color.green = 50;
				color.blue = 50;
				color.opacity = 255;
				bItem = screenContext->mouseAtCross->inRoads.head;
				while(bItem != NULL) {
					draw_single_road(screenContext, screenContext->cr_on_screen, bItem->datap, &color);
					bItem = bItem->next;
				}
				bItem = screenContext->mouseAtCross->outRoads.head;
				while(bItem != NULL) {
					draw_single_road(screenContext, screenContext->cr_on_screen, bItem->datap, &color);
					bItem = bItem->next;
				}

			}
		}


		/* selecting a road */
		/* when the mouse is in a cross, there should be no road being selected */
		if(screenContext->mouseAtCross == NULL) {
			mindist = MAXVALUE;
			prevRoad = screenContext->mouseAtRoad;
			screenContext->mouseAtRoad = NULL;
			bItem = aCell->roads.head;
			while( bItem != NULL) {
				aRoad = (struct Road*)bItem->datap;
//				if(is_point_in_box(&point, &aRoad->box)) {
					  dist = distance_point_to_polyline(&point, &aRoad->points, &interpoint, &seg);
					  if (dist< mindist && dist < RECOGNITION_GRAN) {
						  mindist = dist;
						  screenContext->mouseAtRoad = aRoad;
					  }
//				} 
				bItem = bItem->next;
			}

			if(prevRoad != NULL) {
				color.red = 255;
				color.green = 255;
				color.blue = 255;
				color.opacity = 255;
				draw_single_road(screenContext, screenContext->cr_on_screen, prevRoad, &color);

				gdk_draw_drawable(widget->window, screenContext->gc, screenContext->canvas, 
						  screenContext->scr_width-MARGIN-200, MARGIN+40, 
						  screenContext->scr_width-MARGIN-200, MARGIN+40, 
						  200, 100);
			}
			if(screenContext->mouseAtRoad != NULL) {
				color.red = 0; 
				color.green = 0;
				color.blue = 0;
				color.opacity = 255;
				draw_single_road(screenContext, screenContext->cr_on_screen, screenContext->mouseAtRoad, &color);

				sprintf(buf, "Road No. %d, Length: %.2fKM", screenContext->mouseAtRoad->id, screenContext->mouseAtRoad->length/1000);
				show_text(screenContext, buf, screenContext->scr_width-MARGIN-200, MARGIN+40, FONT_SIZE);
				sprintf(buf, "Head angle: %d, Tail angle: %d", screenContext->mouseAtRoad->headEndAngle, screenContext->mouseAtRoad->tailEndAngle);
				show_text(screenContext, buf, screenContext->scr_width-MARGIN-200, MARGIN+65, FONT_SIZE);
				sprintf(buf, "[%.3lf %.3lf %.3lf %.3lf]", screenContext->mouseAtRoad->box.xmin,
									  screenContext->mouseAtRoad->box.ymin,
									  screenContext->mouseAtRoad->box.xmax,
									  screenContext->mouseAtRoad->box.ymax);
				show_text(screenContext, buf, screenContext->scr_width-MARGIN-200, MARGIN+90, FONT_SIZE);
			}
			
		}
	}				

  } else if(x>screenContext->scr_x && x<MARGIN+screenContext->scr_x&&y>screenContext->scr_height/4+screenContext->scr_y&&y<3*screenContext->scr_height/4+screenContext->scr_y) {
	if(screenContext->areaOnScreen != 1) {
		screenContext->areaOnScreen = 1;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if((x>screenContext->scr_x && x<MARGIN+screenContext->scr_x&&y>3*screenContext->scr_height/4+screenContext->scr_y&&y<screenContext->scr_height-MARGIN+screenContext->scr_y) || (x>screenContext->scr_x && x<screenContext->scr_width/4+screenContext->scr_x&&y>=screenContext->scr_height-MARGIN+screenContext->scr_y&&y<screenContext->scr_height+screenContext->scr_y)) {
	if(screenContext->areaOnScreen != 2) {
		screenContext->areaOnScreen = 2;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if(x>screenContext->scr_width/4+screenContext->scr_x && x<3*screenContext->scr_width/4+screenContext->scr_x&&y>screenContext->scr_height-MARGIN+screenContext->scr_y&&y<screenContext->scr_height+screenContext->scr_y) {
	if(screenContext->areaOnScreen != 3) {
		screenContext->areaOnScreen = 3;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}
  } else if((x>screenContext->scr_width*3/4+screenContext->scr_x && x<screenContext->scr_width+screenContext->scr_x&&y>screenContext->scr_height-MARGIN+screenContext->scr_y&&y<screenContext->scr_height+screenContext->scr_y) || (x>screenContext->scr_width-MARGIN+screenContext->scr_x&&x<screenContext->scr_width+screenContext->scr_x&&y>screenContext->scr_height*3/4+screenContext->scr_y&&y<screenContext->scr_height+screenContext->scr_y)) {
	if(screenContext->areaOnScreen != 4) {
		screenContext->areaOnScreen = 4;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}
  } else if(x>screenContext->scr_width-MARGIN+screenContext->scr_x&&x<screenContext->scr_width+screenContext->scr_x&&y<screenContext->scr_height*3/4+screenContext->scr_y&&y>screenContext->scr_height/4+screenContext->scr_y) {
	if(screenContext->areaOnScreen != 5) {
		screenContext->areaOnScreen = 5;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}
  } else if((x>screenContext->scr_width-MARGIN+screenContext->scr_x&&x<screenContext->scr_width+screenContext->scr_x&&y>MARGIN+screenContext->scr_y&&y<screenContext->scr_height/4+screenContext->scr_y) || (x>screenContext->scr_width*3/4+screenContext->scr_x&&x<screenContext->scr_width+screenContext->scr_x&&y>screenContext->scr_y&&y<MARGIN+screenContext->scr_y)) {
	if(screenContext->areaOnScreen != 6) {
		screenContext->areaOnScreen = 6;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}
  } else if(x>screenContext->scr_width/4+screenContext->scr_x&& x<screenContext->scr_width*3/4+screenContext->scr_x&&y>screenContext->scr_y&&y<MARGIN+screenContext->scr_y) {
	if(screenContext->areaOnScreen != 7) {
		screenContext->areaOnScreen = 7;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}
  } else if((x>screenContext->scr_x && x<screenContext->scr_width/4+screenContext->scr_x&&y>screenContext->scr_y&&y<MARGIN+screenContext->scr_y) || (x>screenContext->scr_x&&x<MARGIN+screenContext->scr_x&&y>MARGIN+screenContext->scr_y&&y<screenContext->scr_height/4+screenContext->scr_y)) {
	if(screenContext->areaOnScreen != 8) {
		screenContext->areaOnScreen = 8;
		rub_bar(widget->window, screenContext->gc, MARGIN);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}
  }
  return TRUE;
}


gboolean
drawing_area_button_press_event (GtkWidget *widget,
				 GdkEventButton *event,
				 gpointer data)
{
  if(screenContext->areaOnScreen == 0) {
	  /* zoom in */
	  if(event->type == GDK_2BUTTON_PRESS && event->button == 1) {
		screenContext->scale = screenContext->scale * ZOOM_SCALE;
		screenContext->awin.sBox.xmax = screenContext->awin.sBox.xmax*ZOOM_SCALE;
		screenContext->awin.sBox.ymax = screenContext->awin.sBox.ymax*ZOOM_SCALE;
		setup_drawing_content(screenContext, MAX((event->x+screenContext->scr_x)*ZOOM_SCALE-0.5*screenContext->scr_width, -MARGIN), MAX((event->y+screenContext->scr_y)*ZOOM_SCALE-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
  		draw_canvas(screenContext);
	  }

	  /*zoom out*/
	  if(event->type == GDK_2BUTTON_PRESS && event->button == 3) {
		if( screenContext->scale >= 1) {
			screenContext->scale /= ZOOM_SCALE;
			screenContext->awin.sBox.xmax = screenContext->awin.sBox.xmax/ZOOM_SCALE;
			screenContext->awin.sBox.ymax = screenContext->awin.sBox.ymax/ZOOM_SCALE;
			setup_drawing_content(screenContext, MAX((event->x+screenContext->scr_x)/ZOOM_SCALE-0.5*screenContext->scr_width, -MARGIN), MAX((event->y+screenContext->scr_y)/ZOOM_SCALE-0.5*screenContext->scr_height, -MARGIN), NOTSET, NOTSET, NULL);
			draw_canvas(screenContext);
		}
	  }

  } else if (screenContext->areaOnScreen == 1) {
	int delta = MIN(screenContext->scr_x+MARGIN, screenContext->scr_width/MOVE_PACE);
	if (delta > 0) {		 
		setup_drawing_content(screenContext, screenContext->scr_x-delta, NOTSET, NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if (screenContext->areaOnScreen == 2) {
	int deltax = MIN(screenContext->scr_x+MARGIN, screenContext->scr_width/MOVE_PACE);
	int deltay = MIN(screenContext->awin.sBox.ymax-screenContext->scr_y-screenContext->scr_height+MARGIN, screenContext->scr_height/4);
	if(deltax > 0 || deltay > 0) {
		setup_drawing_content(screenContext, screenContext->scr_x-deltax, MAX(screenContext->scr_y+deltay, -MARGIN), NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if (screenContext->areaOnScreen == 3) {
	int delta = MIN(screenContext->awin.sBox.ymax-screenContext->scr_y-screenContext->scr_height+MARGIN, screenContext->scr_height/MOVE_PACE);
	if (delta > 0) {		 
		setup_drawing_content(screenContext, NOTSET, MAX(screenContext->scr_y+delta, -MARGIN), NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if (screenContext->areaOnScreen == 4) {
	int deltax = MIN(screenContext->awin.sBox.xmax-screenContext->scr_x-screenContext->scr_width+MARGIN, screenContext->scr_width/MOVE_PACE);
	int deltay = MIN(screenContext->awin.sBox.ymax-screenContext->scr_y-screenContext->scr_height+MARGIN, screenContext->scr_height/MOVE_PACE);
	if(deltax > 0 || deltay > 0) {
		setup_drawing_content(screenContext, MAX(screenContext->scr_x+deltax, -MARGIN), MAX(screenContext->scr_y+deltay, -MARGIN), NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if (screenContext->areaOnScreen == 5) {
	int delta = MIN(screenContext->awin.sBox.xmax-screenContext->scr_x-screenContext->scr_width+MARGIN, screenContext->scr_width/MOVE_PACE);
	if (delta > 0) {		 
		setup_drawing_content(screenContext, MAX(screenContext->scr_x+delta, -MARGIN), NOTSET, NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if (screenContext->areaOnScreen == 6) {
	int deltax = MIN(screenContext->awin.sBox.xmax-screenContext->scr_x-screenContext->scr_width+MARGIN, screenContext->scr_width/MOVE_PACE);
	int deltay = MIN(screenContext->scr_y+MARGIN, screenContext->scr_height/4);
	if(deltax > 0 || deltay > 0) {
		setup_drawing_content(screenContext, MAX(screenContext->scr_x+deltax, -MARGIN), screenContext->scr_y-deltay, NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if (screenContext->areaOnScreen == 7) {
	int delta = MIN(screenContext->scr_y+MARGIN, screenContext->scr_height/MOVE_PACE);
	if (delta > 0) {		 
		setup_drawing_content(screenContext, NOTSET, screenContext->scr_y-delta, NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  } else if (screenContext->areaOnScreen == 8) {
	int deltax = MIN(screenContext->scr_x+MARGIN, screenContext->scr_width/MOVE_PACE);
	int deltay = MIN(screenContext->scr_y+MARGIN, screenContext->scr_height/MOVE_PACE);
	if(deltax > 0 || deltay > 0) {
		setup_drawing_content(screenContext, screenContext->scr_x-deltax, screenContext->scr_y-deltay, NOTSET, NOTSET, NULL);
		draw_canvas(screenContext);
		brush_bar(widget->window, screenContext->gc, MARGIN, screenContext->areaOnScreen);
	}

  }
  return TRUE;
}


/* Create a new pixmap of the appropriate size to store our map */
gboolean
drawing_area_configure_event (GtkWidget *widget,
			      GdkEventConfigure *event,
			      gpointer data)
{
  struct WindowSize awin;

  setup_drawing_context(screenContext);

  if(screenContext->firstShown ) {
	  awin.cBox.xmin = screenContext->region->chosen_polygon->box.xmin;
	  awin.cBox.xmax = screenContext->region->chosen_polygon->box.xmax;
	  awin.cBox.ymin = screenContext->region->chosen_polygon->box.ymin;
	  awin.cBox.ymax = screenContext->region->chosen_polygon->box.ymax;

	  awin.sBox.xmin = 0;
	  awin.sBox.ymin = 0;
	  awin.sBox.xmax =widget->allocation.width-2*MARGIN;
	  awin.sBox.ymax =widget->allocation.height-2*MARGIN;
  	  setup_drawing_content(screenContext, -MARGIN, -MARGIN, widget->allocation.width, widget->allocation.height, &awin);
	  screenContext->firstShown = 0;
  } else 
	  setup_drawing_content(screenContext, NOTSET, NOTSET, widget->allocation.width, widget->allocation.height, NULL);
  draw_canvas(screenContext);

  return TRUE;
}


gboolean
drawing_area_expose_event (GtkWidget *widget,
			   GdkEventExpose *event,
			   gpointer data)
{
  gdk_draw_drawable(widget->window, 
		    screenContext->gc, 
		    screenContext->canvas, 
		    event->area.x,
		    event->area.y,
		    event->area.x,
		    event->area.y,
		    event->area.width,
		    event->area.height);
  return FALSE;
}

