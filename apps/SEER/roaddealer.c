#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "utility.h" 
#include "tracedata.h"

double R = 6.371229*1e6;
struct Window_size awin;
struct Polygon *chosen_polygon = NULL;
struct Region *region = NULL;
char *shpfile=NULL, *attrfile=NULL, *distfile=NULL;
char *riverfile=NULL, *parkfile = NULL;
struct TraceData traces;

int main(int argc, char **argv) 
{
  int x, y, width, height, interval=0, scrap = 0;
  double cellsize = 150;
  char *pnts = NULL;
  char *starttime = NULL;
  char *endtime = NULL;
  char *conFile = NULL;
  char *tracefile = NULL;
  char *amddir = NULL;
  char *roadlistfile = NULL;
  char buffer[256];

  int dump_condition = -2;

  FILE *fshp, *fattr, *ftrace, *famd;
  char draw_flag = 0;
  double commRange = cellsize;
  struct Point point, point1, point2;
  int i,j, m1, m2, n1, n2;
  struct TraceList *newTrace, *aTrace, *tmTrace;
  struct LogList *alog;
  struct Hash_table allContacts; 
  double tablesize = 1000;
  int trace_count = 0;
  time_t mintm=(unsigned)0, maxtm =(unsigned)0;
  char onceonly = 1;
  int centerroad = 0;

  if(argc < 3) {
	printf("Usage: %s [-r \"x1 y1 x2 y2 x3 y3...\"] [-d tracedata] [-l roadlist] [-i timeslide] [-j scrap] [-s cellsize] [-c comm range] [-h hashtable size] [-w file.conts] [-q center_road]  [-f \"start yyyy-mm-dd hh:mm:ss\"]  [-t \"end yyyy-mm-dd hh:mm:ss\"] [-a dir_dump_amended_trace] [-print] shapefile attrfile\n", argv[0]);
	exit(1);
  }
  while(  (argv[1][0])=='-' ) {
    switch ( argv[1][1]) {
      case 'a':
        amddir = argv[2];
        argc-=2;
        argv+=2;
        break;
      case 'c':
        commRange = atof(argv[2]);
        argc-=2;
        argv+=2;
        break;
      case 'd':
        tracefile = argv[2];
        argc-=2;
        argv+=2;
        break;
      case 'f':
        starttime = argv[2];
        argc-=2;
        argv+=2;
        break;
      case 'h':
        tablesize = atoi(argv[2]);
        argc-=2;
        argv+=2;
        break;
      case 'i':
        interval = atoi(argv[2]);
        argc-=2;
        argv+=2;
        break;
      case 'j':
        scrap = atoi(argv[2]);
        argc-=2;
        argv+=2;
        break;
      case 'l':
        roadlistfile = argv[2];
        argc-=2;
        argv+=2;
        break;
      case 'p':
	dump_condition = atoi(argv[2]);
	argc-=2;
	argv+=2;
	break;
      case 'q':
	centerroad = atoi(argv[2]);
	argc-=2;
	argv+=2;
	break;
      case 'r':
	pnts = argv[2];
        argc-=2;
        argv+=2;
        break;
      case 's':
        cellsize = atof(argv[2]);
	commRange = cellsize;
        argc-=2;
        argv+=2;
        break;
      case 't':
        endtime = argv[2];
        argc-=2;
        argv+=2;
        break;
      case 'w':
        conFile = argv[2];
        argc-=2;
        argv+=2;
        break;
      default:
        printf("Bad option %s\n", argv[1]);
	printf("Usage: %s [-r \"x1 y1 x2 y2 x3 y3...\"] [-d tracedata] [-l roadlist] [-i timeslide] [-j scrap] [-s cellsize] [-c comm range] [-h hashtable size] [-w file.conts] [-q center_road]  [-f \"start yyyy-mm-dd hh:mm:ss\"]  [-t \"end yyyy-mm-dd hh:mm:ss\"] [-a dir_dump_amended_trace] [-print] shapefile attrfile\n", argv[0]);
        exit(1);
      }
  }

  if(argc < 3) {
	printf("Usage: %s [-r \"x1 y1 x2 y2 x3 y3...\"] [-d tracedata] [-l roadlist] [-i timeslide] [-j scrap] [-s cellsize] [-c comm range] [-h hashtable size] [-w file.conts] [-q center_road]  [-f \"start yyyy-mm-dd hh:mm:ss\"]  [-t \"end yyyy-mm-dd hh:mm:ss\"] [-a dir_dump_amended_trace] [-print] shapefile attrfile\n", argv[0]);
	exit(1);
  }

  shpfile = argv[1];
  attrfile = argv[2];

  if(pnts == NULL) {
  	if ((fshp=fopen(shpfile, "r"))==NULL || (fattr=fopen(attrfile, "r"))==NULL)
	  {
        	printf("Cannot open the shape or attribute file for reading. \n");
	        return;
	  }
	fseek(fshp, 36, SEEK_SET);
	fread(&(awin.cxmin), sizeof(double), 1, fshp);
	fread(&(awin.cymin), sizeof(double), 1, fshp);
	fread(&(awin.cxmax), sizeof(double), 1, fshp);
	fread(&(awin.cymax), sizeof(double), 1, fshp);
	fclose(fshp);
	fclose(fattr);
	point.x = awin.cxmin, point.y = awin.cymin;
	build_polygon(&chosen_polygon, &point);
	point.x = awin.cxmin, point.y = awin.cymax;
	build_polygon(&chosen_polygon, &point);
	point.x = awin.cxmax, point.y = awin.cymax;
	build_polygon(&chosen_polygon, &point);
	point.x = awin.cxmax, point.y = awin.cymin;
	build_polygon(&chosen_polygon, &point);
	close_polygon(chosen_polygon);
  } else {
  	double x1, y1;
	char *px, *py;

	px = strtok(pnts, " ");
	py = strtok(NULL, " ");
	x1 = atof(px);
	y1 = atof(py);
	point.x = x1, point.y = y1;
	build_polygon(&chosen_polygon, &point);
	awin.cxmin = awin.cxmax = x1;
	awin.cymin = awin.cymax = y1;
	while(1) {	
		px = strtok(NULL, " ");
		if(px==NULL)
			break;
		py = strtok(NULL, " ");
		x1 = atof(px);
		y1 = atof(py);
		point.x = x1, point.y = y1;
		build_polygon(&chosen_polygon, &point);
		if(awin.cxmin > x1) awin.cxmin = x1;
		if(awin.cymin > y1) awin.cymin = y1;
		if(awin.cxmax < x1) awin.cxmax = x1;
		if(awin.cymax < y1) awin.cymax = y1;
	}
	close_polygon(chosen_polygon);
  }
 
  build_region(chosen_polygon, cellsize*180/(M_PI*R), &region);
  //fix_roadmap(region);

  /*emit hops from the centerroad*/
  if(centerroad!=0) {
	calculate_road_hops(centerroad);
	dump_road_hops();
  }
 
  region->starttime = strtot(starttime);
  region->endtime = strtot(endtime);
  if(region->endtime < region->starttime)
	region->endtime=region->starttime;

  region->interval = interval;
  region->scrap = scrap;
  region->commRange = commRange;


  traces.head = NULL;
  traces.nTraces = 0;
  traces.showWhich = NULL;
  if(tracefile != NULL) {
	if ((ftrace = fopen(tracefile, "r"))!= NULL) {
		char filename[128], astring[128];
		FILE *tracef;
		while (fgetc(ftrace)!=EOF)
		{
			fseek(ftrace, -1, SEEK_CUR);
			fgets(filename, 127, ftrace);
			sscanf(filename,"%s", astring);
			if((tracef = fopen(astring, "r"))!= NULL) {
				printf("Loading Trace:%s\n", astring);
				/* load_trace will return a list of traces*/
				newTrace = load_trace(astring, 0, tracef, &mintm, &maxtm);
				if(onceonly == 1) {
					onceonly =0;
					if(region->starttime == 0)
						region->starttime = mintm;
					if(region->endtime == 0)
						region->endtime = maxtm;
					if(region->interval == -1)
						region->interval = region->endtime-region->starttime;

					setup_roads_slides(roadlistfile); 
					//setup_cell_slides();
				}
				traces.nTraces ++;
				aTrace = newTrace;
				while(aTrace!=NULL) {
					tmTrace = aTrace->next;
					check_trace(aTrace);
					amend_trace(aTrace);
					if(aTrace->logs != 0) {
						if(amddir!=NULL) {
							memset(buffer, 0, 255);
							strncpy(buffer, amddir, strlen(amddir)+1);
							strncat(buffer, "/", 1);
							strncat(buffer, aTrace->name, strlen(aTrace->name));
							if((famd = fopen(buffer, "w"))!=NULL) {
								dump_amended_trace(aTrace, famd);
								fflush(famd);
								fclose(famd);
							}
						}
						alog = aTrace->logs;
						while(alog != NULL) {
							install_log_to_road_slide(alog->aLog);
							alog = alog->next;
						}
					}
					free_trace(aTrace);
					aTrace = tmTrace;
				}
				fclose(tracef);
			}
		}
		
		fclose(ftrace);
		if(dump_condition == 0) {
			calculate_road_condition(NULL, NULL);
			dump_road_condition();
		} else if (dump_condition != -2) {
			dump_every_sample_on_road(dump_condition);
		}
	}
  }	
  free_polygon(chosen_polygon);
  free_region(region);
}
