#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "color.h"
#include "common.h"
#include "files.h"
#include "gtk_cairo.h"
#include "events.h"
#include "trace.h"
#include "contact.h"


struct ScreenContext *screenContext = NULL;

/* Our main function */
int main( int   argc,
          char *argv[] )
{
  GdkScreen *screen;
  GtkWidget *mainWindow, *mainVBox;
  GtkWidget *controlHBox, *geoHBox, *roadCrossVBox, *aVBox;
  GtkWidget *drawingAreaFrame;
  GtkWidget *controlInternalVBox = NULL, *roadListSW, *crossListSW, *traceInternalVBox, *contactInternalVBox, *routeInternalVBox, *routeInternalHBox ;
  GtkWidget *playHBox, *fromInputHBox, *untilInputHBox, *xInputHBox, *yInputHBox;
  GtkWidget *fastBackwardButton, *backwardButton, *forwardButton, *fastForwardButton, *replayButton;
  GtkWidget *fromLabel, *fromEntry, *untilLabel,*xLabel, *yLabel, *untilEntry, *xEntry, *yEntry;
  GtkWidget *roadListTV, *crossListTV;
  GtkWidget *traceListSW, *selectAllTraceButton;
  GtkWidget *traceListTV;
  GtkWidget *aListSW, *selectAllContactButton;
  GtkWidget *aListTV;
  GtkWidget *routeListSW, *selectAllRouteButton, *routeCoverButton;
  GtkWidget *routeListTV;
  GtkWidget *drawRsuTopoButton;
  GtkWidget *drawStorageButton;
  GtkWidget *drawingArea;

  GtkTreeIter iter;
  GtkListStore *roadListStore, *crossListStore, *routeListStore, *traceListStore, *aListStore;
  GtkTreeModel *routeListTVModel, *traceListTVModel, *aListTVModel;
  GtkCellRenderer *roadListRenderer, *crossListRenderer, *routeListRenderer, *traceListRenderer, *aListRenderer;
  GtkTreeViewColumn *roadListTVColumn, *crossListTVColumn, *routeListTVColumn, *traceListTVColumn, *aListTVColumn;
  GtkTreeSelection *roadListTreeSelect, *crossListTreeSelect, *routeListTreeSelect, *traceListTreeSelect, *aListTreeSelect;

  char buf[128];
  FILE *fsource;
//  gint screenheight, screenwidth;
  struct Item *aRouteItem;
  struct Item *aTraceItem;
  struct Item *aRoadItem;
  struct Item *aCrossItem;
  struct Item *aItem;
  struct Item *aPairItem, *tempPairItem, *aContactItem;
  struct Pair *aPair;
  struct Contact *aContact;
  time_t startAt = 0, endAt = 0, sAt, eAt;
 
  char first;
  unsigned long at;

  int cGran=1, fGran = -1;
  int satisfied;


  if(argc < 2) {
	g_print("Usage: %s .map [-debug] [-c colormap] [-v .vmap] [-m contact_load_mode ] [-n contacts number] [-f contacts interval(hours)]  [.lst .cont .ogd .mgd .dsp .bus ...]\n", argv[0]);
	exit(1);
  }

  if((fsource=fopen(argv[1], "rb"))!=NULL) {
	screenContext = (struct ScreenContext*)malloc(sizeof(struct ScreenContext));
	g_print("Loading map ...\n");
	screenContext->region = region_load_func(fsource, NULL, -1, -1);
	init_screen_context(screenContext);
	fclose(fsource);
  }

  while(argc>2 && argv[2][0] == '-') {
	switch(argv[2][1]) {
	case 'd':
		screenContext->debug = 1;
		argc-=1;
		argv+=1;
		break;
	case 'n':
		cGran = atoi(argv[3]);
		argc-=2;
		argv+=2;
		break;

	case 'f':
		fGran = atoi(argv[3]);
		argc-=2;
		argv+=2;
		break;

	case 'm':
		if(!strcmp(argv[3], "pairwise"))
			screenContext->contactTableMode = PAIRWISE_TABLE;
		else
			screenContext->contactTableMode = EGO_TABLE;
		argc-=2;
		argv+=2;
		break;

	case 'c':
		screenContext->colormap = load_colormap(argv[3]);
		argc-=2;
		argv+=2;
		break;

	case 'v':
  		if((fsource=fopen(argv[3], "rb"))!=NULL) {
			g_print("Loading RSU topology in .vmap ...\n");
			screenContext->vRegion = region_load_func(fsource, NULL, -1, -1);
			fclose(fsource);
		}
		argc-=2;
		argv+=2;
		break;

	default:
		g_print("Usage: %s .map [-debug] [-c colormap] [-v .vmap] [.lst .cont .ogd .mgd .dsp .bus ...]\n", argv[0]);
	}
  }

  if(screenContext->contactTableMode == PAIRWISE_TABLE) 
	hashtable_init(&screenContext->contactTable, NUM_OF_CONTACTPAIRS, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  else
	hashtable_init(&screenContext->contactTable, NUM_OF_TRACES, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))ego_has_name);

  srand(time(NULL));

  first = 1;
  while(argc>2) {
	if((fsource=fopen(argv[2], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[2]);
		sAt = startAt;
		eAt = endAt;
		load_source_file(fsource, screenContext->region, &screenContext->traceTable, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, &screenContext->contactTable, screenContext->contactTableMode, (void*(*)(FILE*, struct Region*, void *, int, time_t *, time_t *))load_contacts_with_hashtable, &screenContext->cellTable, (void*(*)(FILE*, struct Region*, void *, time_t *, time_t *))load_cell_displays_with_hashtable, NULL, NULL, &screenContext->routeTable, (void*(*)(FILE*, struct Region*, void *))load_route_with_hashtable, &sAt, &eAt, NULL);
		fclose(fsource);
		if(sAt) {
			if(first) {
				screenContext->startAt = sAt;
				screenContext->endAt = eAt;
				first --;
			} else {
				if (sAt < screenContext->startAt)
					screenContext->startAt = sAt;
				if (eAt > screenContext->endAt)
					screenContext->endAt = eAt;
			}
		}
	}

	argc--;
	argv++;
  }

  screenContext->atClock = screenContext->startAt;
  if(screenContext->cellTable.count > 0)
	set_cell_table_time(&screenContext->cellTable, screenContext->atClock);

  printf("Decorating windows, please wait ...\n");
  gtk_init(&argc, &argv);

  /* all widget stuff */
  mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(mainWindow), "Urban Vehicular Ad hoc Network Viewer");
  g_signal_connect(mainWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(mainWindow), 4);

  /* vbox for control and drawing areas */
  mainVBox = gtk_vbox_new(FALSE, 8);
  gtk_container_set_border_width(GTK_CONTAINER(mainVBox), 4); 
  gtk_container_add (GTK_CONTAINER(mainWindow), mainVBox);

  /* hbox for control area */
  controlHBox = gtk_hbox_new(FALSE, 5);

  /* geo-related */
  geoHBox = gtk_hbox_new(FALSE, 5);

  roadCrossVBox = gtk_vbox_new(TRUE,2);
  /* --> road list */
  roadListStore = gtk_list_store_new(NUM_RD_COLUMNS, G_TYPE_STRING);
  aRoadItem = screenContext->region->roads.head;
  while (aRoadItem!=NULL)
  {
	  gtk_list_store_append(roadListStore, &iter);
	  sprintf(buf, "%d", ((struct Road*)aRoadItem->datap)->id);
	  gtk_list_store_set(roadListStore, &iter, COLUMN_RD_NAME, buf, -1);
	  aRoadItem = aRoadItem->next;
  }
  roadListTV = gtk_tree_view_new_with_model(GTK_TREE_MODEL(roadListStore));
  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(roadListTV), TRUE);
  gtk_tree_view_set_search_column(GTK_TREE_VIEW(roadListTV), COLUMN_RD_NAME);
  g_object_unref(roadListStore);

  roadListSW = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(roadListSW),GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(roadListSW), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(roadListSW), roadListTV);

  roadListRenderer = gtk_cell_renderer_text_new ();
  roadListTVColumn = gtk_tree_view_column_new_with_attributes ("Road Id",
								roadListRenderer,
								"text",
								COLUMN_RD_NAME,
								NULL);
  gtk_tree_view_column_set_sort_column_id(roadListTVColumn, COLUMN_RD_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW(roadListTV), roadListTVColumn);

  roadListTreeSelect = gtk_tree_view_get_selection (GTK_TREE_VIEW(roadListTV));
  gtk_tree_selection_set_mode (roadListTreeSelect, GTK_SELECTION_SINGLE);
  g_signal_connect(G_OBJECT (roadListTreeSelect), "changed",
		    G_CALLBACK (roadlist_tree_selection_changed),
		    NULL);
  gtk_box_pack_start (GTK_BOX(roadCrossVBox), roadListSW, TRUE, TRUE, 0);

  /* --> cross list */
  crossListStore = gtk_list_store_new(NUM_RD_COLUMNS, G_TYPE_STRING);
  aCrossItem = screenContext->region->crosses.head;
  while (aCrossItem!=NULL)
  {
	  gtk_list_store_append(crossListStore, &iter);
	  sprintf(buf, "%d", ((struct Cross*)aCrossItem->datap)->number);
	  gtk_list_store_set(crossListStore, &iter, COLUMN_RD_NAME, buf, -1);
	  aCrossItem = aCrossItem->next;
  }
  crossListTV = gtk_tree_view_new_with_model(GTK_TREE_MODEL(crossListStore));
  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(crossListTV), TRUE);
  gtk_tree_view_set_search_column(GTK_TREE_VIEW(crossListTV), COLUMN_RD_NAME);
  g_object_unref(crossListStore);

  crossListSW = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(crossListSW),GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(crossListSW), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(crossListSW), crossListTV);

  crossListRenderer = gtk_cell_renderer_text_new ();
  crossListTVColumn = gtk_tree_view_column_new_with_attributes ("Cross Id",
								crossListRenderer,
								"text",
								COLUMN_RD_NAME,
								NULL);
  gtk_tree_view_column_set_sort_column_id(crossListTVColumn, COLUMN_RD_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW(crossListTV), crossListTVColumn);

  crossListTreeSelect = gtk_tree_view_get_selection (GTK_TREE_VIEW(crossListTV));
  gtk_tree_selection_set_mode(crossListTreeSelect, GTK_SELECTION_SINGLE);
  g_signal_connect(G_OBJECT(crossListTreeSelect), "changed",
		    G_CALLBACK (crosslist_tree_selection_changed),
		    NULL);
  gtk_box_pack_start (GTK_BOX(roadCrossVBox), crossListSW, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(geoHBox), roadCrossVBox, FALSE, FALSE, 0);

  /* --> input (x, y) */
  aVBox = gtk_vbox_new(TRUE,2);

  xInputHBox = gtk_hbox_new (FALSE, 5);

  xLabel = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(xLabel), "X:");
  gtk_box_pack_start (GTK_BOX (xInputHBox), xLabel, FALSE, FALSE, 0);

  xEntry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY(xEntry), 20);
  gtk_widget_set_size_request(GTK_WIDGET(xEntry), 100, -1);
  g_signal_connect (G_OBJECT(xEntry), "activate", 
		    G_CALLBACK (enter_x), (gpointer)xEntry);
  gtk_entry_set_text (GTK_ENTRY (xEntry), "");
  gtk_box_pack_start (GTK_BOX(xInputHBox), xEntry, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(aVBox), xInputHBox, FALSE, FALSE, 0);

  yInputHBox = gtk_hbox_new (FALSE, 5);

  yLabel = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(yLabel), "Y:");
  gtk_box_pack_start (GTK_BOX(yInputHBox), yLabel, FALSE, FALSE, 0);

  yEntry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY(yEntry), 20);
  gtk_widget_set_size_request(GTK_WIDGET(yEntry), 100, -1);
  g_signal_connect (G_OBJECT(yEntry), "activate", 
		    G_CALLBACK (enter_y), (gpointer)yEntry);
  gtk_entry_set_text (GTK_ENTRY (yEntry), "");
  gtk_box_pack_start (GTK_BOX(yInputHBox), yEntry, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(aVBox), yInputHBox, FALSE, FALSE, 0);

  /* --> check box for RSU topology */
  drawRsuTopoButton = gtk_check_button_new_with_label("RSU topology");
  g_signal_connect(G_OBJECT(drawRsuTopoButton), "toggled", 
		   G_CALLBACK(draw_rsu_topology), NULL);
  gtk_box_pack_start (GTK_BOX(aVBox), drawRsuTopoButton, TRUE, TRUE, 0);

  /* --> check box for storage point*/
  drawStorageButton = gtk_check_button_new_with_label("Storage layout");
  g_signal_connect(G_OBJECT(drawStorageButton), "toggled", 
		   G_CALLBACK(draw_storage_nodes), NULL);
  gtk_box_pack_start (GTK_BOX(aVBox), drawStorageButton, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX(geoHBox), aVBox, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX(controlHBox), geoHBox, FALSE, FALSE, 10);


  /* vbox for internal layout within control area */ 
  if(screenContext->traceTable.count || screenContext->contactTable.count || screenContext->cellTable.count) {
	  controlInternalVBox = gtk_vbox_new(TRUE, 5);

	  /* trace play control buttons */
	  playHBox = gtk_hbox_new (FALSE, 2);
	 
	  fastBackwardButton = gtk_button_new_with_label ("<<");
	  g_signal_connect (G_OBJECT (fastBackwardButton), "clicked",
			    G_CALLBACK (fast_backward), NULL);
	  gtk_box_pack_start (GTK_BOX(playHBox), fastBackwardButton, FALSE, FALSE, 0);

	  backwardButton = gtk_toggle_button_new_with_label (" < ");
	  g_signal_connect (G_OBJECT (backwardButton), "clicked",
			    G_CALLBACK (backward), NULL);
	  gtk_box_pack_start (GTK_BOX(playHBox), backwardButton, FALSE, FALSE, 0);

	  forwardButton = gtk_toggle_button_new_with_label (" > ");
	  g_signal_connect (G_OBJECT (forwardButton), "clicked",
			    G_CALLBACK (forward), NULL);
	  gtk_box_pack_start (GTK_BOX(playHBox), forwardButton, FALSE, FALSE, 0);
	 
	  fastForwardButton = gtk_button_new_with_label (">>");
	  g_signal_connect(G_OBJECT(fastForwardButton), "clicked", 
			   G_CALLBACK(fast_forward), NULL);
	  gtk_box_pack_start (GTK_BOX(playHBox), fastForwardButton, FALSE, FALSE, 0);

	  replayButton = gtk_button_new_with_label ("Replay");
	  g_signal_connect (G_OBJECT (replayButton), "clicked",
			    G_CALLBACK (replay), NULL);
	  gtk_box_pack_start (GTK_BOX(playHBox), replayButton, FALSE, FALSE, 0);
	  
	  gtk_box_pack_start(GTK_BOX(controlInternalVBox), playHBox, FALSE, FALSE, 0);
	  /* input time duration */
	  fromInputHBox = gtk_hbox_new (FALSE, 5);

	  fromLabel = gtk_label_new (NULL);
	  gtk_label_set_markup (GTK_LABEL(fromLabel), "From:");
	  gtk_box_pack_start (GTK_BOX (fromInputHBox), fromLabel, FALSE, FALSE, 0);

	  fromEntry = gtk_entry_new ();
	  gtk_entry_set_max_length (GTK_ENTRY(fromEntry), 20);
  	  gtk_widget_set_size_request(GTK_WIDGET(fromEntry), 130, -1);
	  g_signal_connect (G_OBJECT(fromEntry), "activate", 
			    G_CALLBACK (enter_from_time), (gpointer)fromEntry);
	  ttostr(screenContext->startAt, buf);
	  gtk_entry_set_text (GTK_ENTRY (fromEntry), buf);
	  gtk_box_pack_start (GTK_BOX(fromInputHBox), fromEntry, FALSE, FALSE, 0);

	  gtk_box_pack_start (GTK_BOX(controlInternalVBox), fromInputHBox, FALSE, FALSE, 0);

	  untilInputHBox = gtk_hbox_new (FALSE, 5);

	  untilLabel = gtk_label_new (NULL);
	  gtk_label_set_markup (GTK_LABEL(untilLabel), "Until :");
	  gtk_box_pack_start (GTK_BOX (untilInputHBox), untilLabel, FALSE, FALSE, 0);

	  untilEntry = gtk_entry_new ();
	  gtk_entry_set_max_length (GTK_ENTRY(untilEntry), 20);
  	  gtk_widget_set_size_request(GTK_WIDGET(untilEntry), 130, -1);
	  g_signal_connect (G_OBJECT(untilEntry), "activate", 
			    G_CALLBACK (enter_until_time), (gpointer)untilEntry);
	  ttostr(screenContext->endAt, buf);
	  gtk_entry_set_text (GTK_ENTRY (untilEntry), buf);
	  gtk_box_pack_start (GTK_BOX(untilInputHBox), untilEntry, FALSE, FALSE, 0);

	  gtk_box_pack_start (GTK_BOX(controlInternalVBox), untilInputHBox, FALSE, FALSE, 0);

	  gtk_box_pack_start(GTK_BOX(controlHBox), controlInternalVBox, FALSE, FALSE, 10);
  }


  /* trace list */
  if(screenContext->traceTable.count ) {
	  traceInternalVBox = gtk_vbox_new(FALSE, 5);

	  traceListStore = gtk_list_store_new(NUM_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING);
	  for(at = 0; at<screenContext->traceTable.size; at++) {
		  aTraceItem = screenContext->traceTable.head[at];
		  while (aTraceItem!=NULL)
		  {
			  gtk_list_store_append(traceListStore, &iter);
			  gtk_list_store_set(traceListStore, &iter, 
					     COLUMN_TODRAW, FALSE,
					     COLUMN_NAME, ((struct Trace*)aTraceItem->datap)->vName,
					     -1);
			  aTraceItem = aTraceItem->next;
		  }
	  }
	  traceListTV = gtk_tree_view_new_with_model(GTK_TREE_MODEL(traceListStore));
	  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(traceListTV), TRUE);
	  gtk_tree_view_set_search_column(GTK_TREE_VIEW(traceListTV), COLUMN_NAME);
	  g_object_unref(traceListStore);

	  traceListSW = gtk_scrolled_window_new(NULL, NULL);
	  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(traceListSW),GTK_SHADOW_ETCHED_IN);
	  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(traceListSW), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	  gtk_container_add(GTK_CONTAINER(traceListSW), traceListTV);


	  traceListTVModel = gtk_tree_view_get_model(GTK_TREE_VIEW(traceListTV));
	  traceListRenderer = gtk_cell_renderer_toggle_new();
	  g_signal_connect(traceListRenderer, "toggled", G_CALLBACK(trace_selected), traceListTVModel);		
	  traceListTVColumn = gtk_tree_view_column_new_with_attributes( "Show", traceListRenderer, "active", COLUMN_TODRAW, NULL);

	  gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(traceListTVColumn), GTK_TREE_VIEW_COLUMN_FIXED);
	  gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(traceListTVColumn), 50);
	  gtk_tree_view_append_column(GTK_TREE_VIEW(traceListTV), traceListTVColumn);

	  traceListRenderer = gtk_cell_renderer_text_new();
	  traceListTVColumn = gtk_tree_view_column_new_with_attributes( "Vehicle ID",
									traceListRenderer,
									"text",
									COLUMN_NAME,
									NULL);
	  gtk_tree_view_column_set_sort_column_id(traceListTVColumn, COLUMN_NAME);
	  gtk_tree_view_append_column(GTK_TREE_VIEW(traceListTV), traceListTVColumn);

	  traceListTreeSelect = gtk_tree_view_get_selection(GTK_TREE_VIEW(traceListTV));
	  gtk_tree_selection_set_mode(traceListTreeSelect, GTK_SELECTION_SINGLE);
	  g_signal_connect (G_OBJECT(traceListTreeSelect), "changed",
			    G_CALLBACK (tracelist_tree_selection_changed),
			    NULL);
	  gtk_box_pack_start (GTK_BOX(traceInternalVBox), traceListSW, FALSE, FALSE, 0);

	  selectAllTraceButton = gtk_check_button_new_with_label("Selete all");
	  g_signal_connect(G_OBJECT(selectAllTraceButton), "toggled", 
			   G_CALLBACK(select_all_trace), NULL);
	  gtk_box_pack_start (GTK_BOX(traceInternalVBox), selectAllTraceButton, FALSE, FALSE, 0);

	  gtk_box_pack_start(GTK_BOX(controlHBox), traceInternalVBox, FALSE, FALSE, 10);
  }
  /* contact list */
  if(screenContext->contactTable.count ) {
	  contactInternalVBox = gtk_vbox_new(FALSE, 5);
	  for (at=0;at<screenContext->contactTable.size;at++) {
		  aPairItem = screenContext->contactTable.head[at];
		  while (aPairItem != NULL) {
			  aPair = (struct Pair*)aPairItem->datap;
			  satisfied = 1;
			  if(aPair->contents.nItems >= cGran) {
				  aContactItem = aPair->contents.head;
				  while(aContactItem) {
					aContact = (struct Contact*)aContactItem->datap;
					if(aContactItem->next && fGran!=-1 && ((struct Contact*)aContactItem->next->datap)->startAt-aContact->endAt>fGran*3600) {
						satisfied = 0;
						break;
					}
					aContactItem = aContactItem->next;
				  }
			  } else {
				  satisfied = 0;
			  }
			  if(satisfied) {
				  aPairItem = aPairItem->next;
			  } else {
				  tempPairItem = aPairItem->next;
				  sprintf(buf, "%s,%s", aPair->vName1, aPair->vName2);
				  pair_free_func(hashtable_pick(&screenContext->contactTable, buf));
				  aPairItem = tempPairItem;
			  }
		  }
	  }

	  aListStore = gtk_list_store_new(NUM_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING);
	  for(at = 0; at<screenContext->contactTable.size; at++) {
		  aItem = screenContext->contactTable.head[at];
		  while (aItem!=NULL)
		  {
			  gtk_list_store_append(aListStore, &iter);
			  if(screenContext->contactTableMode == PAIRWISE_TABLE) 
				sprintf(buf, "%s,%s", ((struct Pair*)aItem->datap)->vName1, ((struct Pair*)aItem->datap)->vName2);
			  else
				sprintf(buf, "%s", ((struct Ego*)aItem->datap)->vName);

			  gtk_list_store_set(aListStore, &iter, 
					     COLUMN_TODRAW, FALSE,
					     COLUMN_NAME, buf,
					     -1);
			  aItem = aItem->next;
		  }
	  }
	  aListTV = gtk_tree_view_new_with_model(GTK_TREE_MODEL(aListStore));
	  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(aListTV), TRUE);
	  gtk_tree_view_set_search_column(GTK_TREE_VIEW(aListTV), COLUMN_NAME);
	  g_object_unref(aListStore);

	  aListSW = gtk_scrolled_window_new(NULL, NULL);
	  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(aListSW),GTK_SHADOW_ETCHED_IN);
	  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(aListSW), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	  gtk_container_add(GTK_CONTAINER(aListSW), aListTV);
	  gtk_box_pack_start (GTK_BOX(contactInternalVBox), aListSW, FALSE, FALSE, 0);

	  aListTVModel = gtk_tree_view_get_model(GTK_TREE_VIEW(aListTV));
	  aListRenderer = gtk_cell_renderer_toggle_new();
	  g_signal_connect(aListRenderer, "toggled", G_CALLBACK(contact_selected), aListTVModel);		
	  aListTVColumn = gtk_tree_view_column_new_with_attributes( "Show", aListRenderer, "active", COLUMN_TODRAW, NULL);

	  gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(aListTVColumn), GTK_TREE_VIEW_COLUMN_FIXED);
	  gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(aListTVColumn), 50);
	  gtk_tree_view_append_column(GTK_TREE_VIEW(aListTV), aListTVColumn);

	  aListRenderer = gtk_cell_renderer_text_new();
	  aListTVColumn = gtk_tree_view_column_new_with_attributes( screenContext->contactTableMode == PAIRWISE_TABLE?"Contact Pair":"Ego",
									aListRenderer,
									"text",
									COLUMN_NAME,
									NULL);
	  gtk_tree_view_column_set_sort_column_id(aListTVColumn, COLUMN_NAME);
	  gtk_tree_view_append_column(GTK_TREE_VIEW(aListTV), aListTVColumn);

	  aListTreeSelect = gtk_tree_view_get_selection(GTK_TREE_VIEW(aListTV));
	  gtk_tree_selection_set_mode(aListTreeSelect, GTK_SELECTION_SINGLE);

	  selectAllContactButton = gtk_check_button_new_with_label("Selete all");
	  gtk_box_pack_start (GTK_BOX(contactInternalVBox), selectAllContactButton, FALSE, FALSE, 0);
	  g_signal_connect(G_OBJECT(selectAllContactButton), "toggled", 
			   G_CALLBACK(select_all_contact), NULL);

	  gtk_box_pack_start(GTK_BOX(controlHBox), contactInternalVBox, FALSE, FALSE, 10);
  }

  /* bus route list */
  if(screenContext->routeTable.count) {
	  routeInternalVBox = gtk_vbox_new(FALSE, 5);

	  routeListStore = gtk_list_store_new(NUM_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING);
	  for(at = 0; at<screenContext->routeTable.size; at++) {
		  aRouteItem = screenContext->routeTable.head[at];
		  while (aRouteItem!=NULL)
		  {
			  gtk_list_store_append(routeListStore, &iter);
			  gtk_list_store_set(routeListStore, &iter, 
					     COLUMN_TODRAW, FALSE,
					     COLUMN_NAME, ((struct Busroute*)aRouteItem->datap)->name,
					     -1);
			  aRouteItem = aRouteItem->next;
		  }
	  }

	  setup_cells_with_routes(screenContext->region, &screenContext->routeTable);
	  deploy_static_nodes_at_most_routes_cells(screenContext->region, &screenContext->nodeTable, 0);

	  routeListTV = gtk_tree_view_new_with_model(GTK_TREE_MODEL(routeListStore));
	  gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(routeListTV), TRUE);
	  gtk_tree_view_set_search_column(GTK_TREE_VIEW(routeListTV), COLUMN_NAME);
	  g_object_unref(routeListStore);

	  routeListSW = gtk_scrolled_window_new(NULL, NULL);
	  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(routeListSW),GTK_SHADOW_ETCHED_IN);
	  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(routeListSW), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	  gtk_container_add(GTK_CONTAINER(routeListSW), routeListTV);

	  gtk_box_pack_start (GTK_BOX(routeInternalVBox), routeListSW, FALSE, FALSE, 0);

	  routeListTVModel = gtk_tree_view_get_model(GTK_TREE_VIEW(routeListTV));
	  routeListRenderer = gtk_cell_renderer_toggle_new();
	  g_signal_connect(routeListRenderer, "toggled", G_CALLBACK(route_selected), routeListTVModel);		
	  routeListTVColumn = gtk_tree_view_column_new_with_attributes( "Show", routeListRenderer, "active", COLUMN_TODRAW, NULL);

	  gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(routeListTVColumn), GTK_TREE_VIEW_COLUMN_FIXED);
	  gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(routeListTVColumn), 50);
	  gtk_tree_view_append_column(GTK_TREE_VIEW(routeListTV), routeListTVColumn);

	  routeListRenderer = gtk_cell_renderer_text_new();
	  routeListTVColumn = gtk_tree_view_column_new_with_attributes( "Bus route",
									routeListRenderer,
									"text",
									COLUMN_NAME,
									NULL);
	  gtk_tree_view_column_set_sort_column_id(routeListTVColumn, COLUMN_NAME);
	  gtk_tree_view_append_column(GTK_TREE_VIEW(routeListTV), routeListTVColumn);

	  routeListTreeSelect = gtk_tree_view_get_selection(GTK_TREE_VIEW(routeListTV));
	  gtk_tree_selection_set_mode(routeListTreeSelect, GTK_SELECTION_SINGLE);
	  g_signal_connect (G_OBJECT(routeListTreeSelect), "changed",
			    G_CALLBACK (routelist_tree_selection_changed),
			    NULL);

	  selectAllRouteButton = gtk_check_button_new_with_label("Selete all");
	  gtk_box_pack_start (GTK_BOX(routeInternalVBox), selectAllRouteButton, FALSE, FALSE, 0);
	  g_signal_connect(G_OBJECT(selectAllRouteButton), "toggled", 
			   G_CALLBACK(select_all_route), NULL);

	  routeInternalHBox = gtk_hbox_new(FALSE, 5); 

	  routeCoverButton = gtk_check_button_new_with_label("Coverage");
	  gtk_box_pack_start (GTK_BOX(routeInternalHBox), routeCoverButton, FALSE, FALSE, 0);
	  g_signal_connect(G_OBJECT(routeCoverButton), "toggled", 
			   G_CALLBACK(select_coverage), NULL);


	  gtk_box_pack_start (GTK_BOX(routeInternalVBox), routeInternalHBox, FALSE, FALSE, 0);
	  gtk_box_pack_end (GTK_BOX(controlHBox), routeInternalVBox, FALSE, FALSE, 10);
  }

  gtk_box_pack_start (GTK_BOX(mainVBox), controlHBox, FALSE, FALSE, 0);

  /* drawing area */
  drawingAreaFrame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(drawingAreaFrame), GTK_SHADOW_IN);
  gtk_box_pack_end(GTK_BOX(mainVBox), drawingAreaFrame, TRUE, TRUE, 0);

  drawingArea = gtk_drawing_area_new();
  screen = gdk_screen_get_default();
//  screenheight = gdk_screen_get_height(screen);
//  screenwidth = gdk_screen_get_width(screen);
//  gtk_widget_set_size_request (drawingArea, screenwidth*0.5, screenheight*0.5);
  gtk_widget_set_size_request (drawingArea, 500, 500);
  gtk_container_add (GTK_CONTAINER(drawingAreaFrame), drawingArea);

  g_signal_connect (drawingArea, "expose_event",
	            G_CALLBACK (drawing_area_expose_event), NULL);
  g_signal_connect (drawingArea, "configure_event",
                    G_CALLBACK (drawing_area_configure_event), NULL);
  g_signal_connect (drawingArea, "motion_notify_event",
                    G_CALLBACK (drawing_area_motion_event), NULL);
  g_signal_connect (drawingArea, "button_press_event",
                    G_CALLBACK (drawing_area_button_press_event), NULL);
  gtk_widget_set_events(drawingArea, GDK_EXPOSURE_MASK
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_BUTTON_PRESS_MASK
  			| GDK_POINTER_MOTION_HINT_MASK);

  screenContext->drawingArea = drawingArea;
  screenContext->forwardButton = forwardButton;
  screenContext->backwardButton = backwardButton;

  /* show all widgets */
  gtk_widget_show_all(mainWindow);

  gtk_main();
  
  destroy_screen_context(screenContext);
  return 0;
}

