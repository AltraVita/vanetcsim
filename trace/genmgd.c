#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<string.h>
#include<glib.h>
#include<sys/stat.h>
#include"common.h"
#include"geometry.h"
#include"trace.h"
#include"rnorrexp.h"
#include"genmgd.h"
#include"files.h"

#define RANDOM_METHOD 0
#define GAUSSIAN_METHOD 1
#define INFLUX_METHOD 2

#define FREE_FASHION 0
#define MAP_FASHION 1


int main(int argc, char **argv)
{
  FILE *fsource, *fdump, *fdumpcross;
  struct Trace* aTrace;
  struct Region *region = NULL;
  struct Item *aItem;
  struct Cross *aCross;
  int i, initID = 0;

  int nTraces = 1;
  time_t startAt = 0;
  time_t endAt = 24*3600;
  int tGran = 1;
  double range = 5000;
  double interval = 3600;
  double variable = 1;
  double timer;
  double xMu, xSigma, yMu, ySigma;
  int method = RANDOM_METHOD;
  int fashion = MAP_FASHION;
  char *dumpcross = NULL;
  double orgX, orgY;

  char directory[256], filename[256];
  sprintf(directory, ".");
  

  if(argc < 2) {
	printf("Usage: %s [-fashion free | map .map] [-method random orgX orgY range | gaussian xMu yMu xSigma ySigma | influx orgX, orgY, range, interval, variable] [-timespan startAt endAt] [-gt temporal_gran] [-i initId] [-st number_of_traces] [-wd directory] [-ws cross_statistics] \n", argv[0]);
	exit(1);
  }

  while(argc > 1 && argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 'f':
		if (!strcmp("free", argv[2])) {
			fashion = FREE_FASHION;
			argc-=2;
			argv+=2;
			break;
		}
		if (!strcmp("map", argv[2])) {
			fashion = MAP_FASHION;
			if((fsource=fopen(argv[3], "rb"))!=NULL) {
				region = region_load_func(fsource, NULL, -1, -1);
				fclose(fsource);
			}
			argc-=3;
			argv+=3;
			break;
		}

	case 'm':
		if (!strcmp("random", argv[2])) {
			method = RANDOM_METHOD;
			orgX = atof(argv[3]);
			orgY = atof(argv[4]);
			range = atoi(argv[5]);
			argc-=5;
			argv+=5;
			break;
		}
		if (!strcmp("gaussian", argv[2])) {
			method = GAUSSIAN_METHOD;
			xMu = atof(argv[3]);
			yMu = atof(argv[4]);
			xSigma = atof(argv[5]);
			ySigma = atof(argv[6]);
			argc-=6;
			argv+=6;
			break;
		}

		if (!strcmp("influx", argv[2])) {
			method = INFLUX_METHOD;
			orgX = atof(argv[3]);
			orgY = atof(argv[4]);
			range = atof(argv[5]);
			interval = atof(argv[6]);
			variable = atoi(argv[7]);
			argc-=7;
			argv+=7;
			break;
		}

	case 't':
		startAt = strtot(argv[2]);
		endAt = strtot(argv[3]);
		argc-=3;
		argv+=3;
		break;

	case 'g':
		if(argv[1][2] == 't')
			tGran = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'i':
		initID = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 's':
		if(argv[1][2] == 't')
			nTraces = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'w':
		if(argv[1][2] == 'd')
			sprintf(directory, "%s", argv[2]); 
		if(argv[1][2] == 's')
			dumpcross = argv[2];
		argc-=2;
		argv+=2;
		break;
		
	default:
		printf("Usage: %s [-fashion free | map .map] [-method random orgX orgY range | gaussian xMu yMu xSigma ySigma | influx orgX, orgY, range, interval, variable] [-timespan startAt endAt] [-gt temporal_gran] [-i initId] [-st number_of_traces] [-wd directory] [-ws cross_statistics] \n", argv[0]);
	}
  }


  srand(time(NULL));
  zigset(time(NULL)); 

  i = initID;
  while(i<initID+nTraces) {
	printf("Making a trace: Taxi_%d...\n", i);
	if(fashion == MAP_FASHION && method == RANDOM_METHOD)
		aTrace = make_a_trace_map(region, startAt, endAt, tGran, i, random_cross_selection, find_shortest_path, orgX, orgY, range, -1);
	else if (fashion == MAP_FASHION && method == GAUSSIAN_METHOD)
		aTrace = make_a_trace_map(region, startAt, endAt, tGran, i, gaussian_cross_selection, find_shortest_path, xMu, yMu, xSigma, ySigma);
	else if (fashion == MAP_FASHION && method == INFLUX_METHOD) {
		if (variable != 0) 
			timer = interval*g_random_double();
		else
			timer = interval;
		aTrace = make_a_trace_map_influx(region, startAt, endAt, tGran, i, orgX, orgY, range, timer, find_shortest_path);
	} else if (fashion == FREE_FASHION && method == RANDOM_METHOD) 
		aTrace = make_a_trace_free(startAt, endAt, tGran, i, random_point_selection, orgX, orgY, range, -1);
	else if (fashion == FREE_FASHION && method == GAUSSIAN_METHOD)
		aTrace = make_a_trace_free(startAt, endAt, tGran, i, gaussian_point_selection, xMu, yMu, xSigma, ySigma);
	else if (fashion == FREE_FASHION && method == INFLUX_METHOD) {
		if (variable != 0) 
			timer = interval*g_random_double();
		else
			timer = interval;
		aTrace = make_a_trace_free_influx(startAt, endAt, tGran, i, orgX, orgY, range, timer);
	}

	if(aTrace != NULL) {
		mkdir(directory,0777);
		sprintf(filename, "%s/%s.mgd", directory, aTrace->vName);
		aTrace->type = FILE_MODIFIED_GPS_TAXI;
		if ( (fdump=fopen(filename, "w"))!=NULL) {
			trace_dump_func(fdump, aTrace);
			fclose(fdump);
		} else {
			perror("Cannot open file to dump");
		}
		trace_free_func(aTrace);
		i ++;
	}
  }
  if((fdumpcross=fopen(dumpcross, "w"))!=NULL) {
	aItem = region->crosses.head;
	while(aItem != NULL) {
		aCross = (struct Cross *)aItem->datap;
		fprintf(fdumpcross, "%d %ld\n", aCross->number, aCross->count);
		aItem = aItem->next;
	}
	fclose(fdumpcross);
  }
  if (region)
	  region_free_func(region);
  return 0;
}

struct Trace* make_a_trace_map(struct Region *region, time_t startAt, time_t endAt, int tGran, long vId, struct Cross*(*cross_selection_func)(struct Region*, struct Cross*, double, double, double, double), struct Path*(*find_path_func)(struct Region*, struct Cross*, struct Cross*), double value1, double value2, double value3, double value4)
{
	time_t clock;
	struct Trace *newTrace = NULL;
	struct Cross *sCross, *dCross;
	struct Path *aPath;
	struct Item *aItem;
	double moveDist;
	int count = 0;

	newTrace = (struct Trace*)malloc(sizeof(struct Trace));
	trace_init_func(newTrace);
	sprintf(newTrace->vName, "v%ld", vId);

	moveDist = 0;
	clock = startAt;
	sCross = initial_cross_selection(region, value1, value2);
	if(sCross == NULL) {
		printf("Initial cross selection fail. Quit!\n");
		exit (1);
	}
	while(clock <= endAt && count < MAXIMUM_SELECTION) {
		dCross = cross_selection_func(region, sCross, value1, value2, value3, value4);
		aPath = find_path_func(region, sCross, dCross);
		/* temporarily use var3 to store trace speed*/
		newTrace->var3 = rand()%20+60;
		if(aPath != NULL) {
			count = 0;
			aItem = aPath->roads.head;
			while(aItem != NULL) {
				moveDist = produce_reports_on_road(newTrace, (struct Road*)aItem->datap, &clock, endAt, tGran, moveDist);
				aItem = aItem->next;
			}
			if (newTrace->reports.head!=NULL)
				/* Drop passenger at the destination */
				((struct Report*)newTrace->reports.head->prev->datap)->state = 0;
			path_free_func(aPath);
			sCross = dCross;
		} else {
			count ++;
		}
	}
	if(count > MAXIMUM_SELECTION) {
		trace_free_func(newTrace);
		newTrace = NULL;
	}
	return newTrace;
}

struct Trace* make_a_trace_map_influx(struct Region *region, time_t startAt, time_t endAt, int tGran, long vId, double orgX, double orgY, double range, double timer, struct Path*(*find_path_func)(struct Region*, struct Cross*, struct Cross*))
{
	time_t clock;
	struct Trace *newTrace = NULL;
	struct Cross *origin, *sCross, *dCross;
	struct Path *aPath;
	struct Item *aItem;
	double moveDist;
	int count = 0;
	double timer_, runtime;

	newTrace = (struct Trace*)malloc(sizeof(struct Trace));
	trace_init_func(newTrace);
	sprintf(newTrace->vName, "v%ld", vId);

	moveDist = 0;
	clock = startAt;
	origin = initial_cross_selection(region, orgX, orgY);
	if(origin == NULL) {
		printf("Initial cross selection fail. Quit!\n");
		exit (1);
	}
	sCross = origin;
	timer_ = timer;
	while(clock <= endAt && count < MAXIMUM_SELECTION) {
		if(equald(timer_, 0, DELTA) || smallerd(timer_, 0, DELTA)) {
			dCross = origin;
			aPath = find_path_func(region, sCross, dCross);
			/* temporarily use var3 to store trace speed*/
			newTrace->var3 = rand()%20+60;
			if(aPath != NULL) {
				aItem = aPath->roads.head;
				while(aItem != NULL) {
					moveDist = produce_reports_on_road(newTrace, (struct Road*)aItem->datap, &clock, endAt, tGran, moveDist);
					aItem = aItem->next;
				}
				if (newTrace->reports.head!=NULL)
					/* Drop passenger at the destination */
					((struct Report*)newTrace->reports.head->prev->datap)->state = 0;
				path_free_func(aPath);
				sCross = dCross;
			}
			timer_ = timer;
		} else {
			dCross = random_cross_selection(region, sCross, -1, -1, range, -1);
			aPath = find_path_func(region, sCross, dCross);
			/* temporarily use var3 to store trace speed*/
			newTrace->var3 = rand()%20+60;
			if(aPath != NULL) {
				count = 0;
				runtime = aPath->length*3.6/newTrace->var3;
				if(runtime < timer_) {
					aItem = aPath->roads.head;
					while(aItem != NULL) {
						moveDist = produce_reports_on_road(newTrace, (struct Road*)aItem->datap, &clock, endAt, tGran, moveDist);
						aItem = aItem->next;
					}
					if (newTrace->reports.head!=NULL)
						/* Drop passenger at the destination */
						((struct Report*)newTrace->reports.head->prev->datap)->state = 0;
					sCross = dCross;
					timer_ = timer_ - runtime;
				}
				path_free_func(aPath);
				timer_ = 0;
			} else {
				count ++;
			}
		}
	}
	if(count > MAXIMUM_SELECTION) {
		trace_free_func(newTrace);
		newTrace = NULL;
	}
	return newTrace;
}


struct Cross* initial_cross_selection(struct Region* region, double orgX, double orgY)
{
	unsigned long randnum, i;
	struct Item *aItem;
	struct Cell *aCell; 
	struct Cross *rtCross = NULL;
	struct Point aPoint;

	aPoint.x = orgX, aPoint.y = orgY;
	aCell = point_in_cell(region, &aPoint);

	if(aCell!= NULL && aCell->crosses.nItems != 0) {
		randnum = rand()%aCell->crosses.nItems;
		aItem = aCell->crosses.head;
		for(i=0; i<randnum; i++)
			aItem = aItem->next;
		rtCross = aItem->datap;
	}  
	return rtCross;
}


struct Cross* random_cross_selection(struct Region* region, struct Cross* atCross, double value1, double value2, double range, double value4)
{
	unsigned long randnum;
	struct Item *aItem;
	struct Box aBox;
	struct Cell *aCell; 
	struct Cross *rtCross = NULL;
	struct Duallist cells;

	int m1, m2, n1, n2, i, j;

	if(atCross == NULL) 
		return NULL;

	create_box(&atCross->gPoint, range, &aBox);
	m1 = floor((aBox.xmin - region->chosen_polygon->box.xmin)/region->cellSize);
	if(m1<0) m1 = 0;
	n1 = floor((aBox.ymin - region->chosen_polygon->box.ymin)/region->cellSize);
	if(n1<0) n1 = 0;
	m2 = floor((aBox.xmax - region->chosen_polygon->box.xmin)/region->cellSize);
	if(m2>region->hCells-1) m2 = region->hCells-1;
	n2 = floor((aBox.ymax - region->chosen_polygon->box.ymin)/region->cellSize);
	if(n2>region->vCells-1) n2 = region->vCells-1;


	duallist_init(&cells);
	for(i=m1; i<=m2; i++)
		for(j=n1; j<=n2; j++) {
			aCell = region->mesh + i*region->vCells + j;
			if(aCell->crosses.nItems != 0) 
				duallist_add_to_tail(&cells, aCell);
		}
	if(cells.nItems != 0) {
		randnum = rand()%cells.nItems;
		aItem = cells.head;
		for(i=0; i<randnum; i++)
			aItem = aItem->next;
		aCell = (struct Cell*)aItem->datap;
		randnum = rand()%aCell->crosses.nItems;
		aItem = aCell->crosses.head;
		for(i=0; i<randnum; i++)
			aItem = aItem->next;
		rtCross = aItem->datap;
	}  
	duallist_destroy(&cells, NULL);
	return rtCross;
}


/* The normal distribution is over cell numbers on the map surface*/
struct Cross* gaussian_cross_selection(struct Region* region, struct Cross* atCross, double xMu, double yMu, double xSigma, double ySigma)
{
	unsigned long randnum;
	struct Item *aItem;
	struct Cell *aCell; 
	struct Cross *rtCross = NULL;
	struct Duallist crosses;
	struct Point aPoint;
	int i, j;
	int m1, m2, n1, n2;
	double dist1, dist2;

	if(atCross == NULL) 
		return NULL;
	dist1 = distance_in_latitude(xSigma);
	dist2 = distance_in_latitude(ySigma);
	/* from here */
	aPoint.x = dist1 * rnor() + xMu;
	aPoint.x = dist2 * rnor() + yMu;

	aCell = point_in_cell(region, &aPoint);
	m1 = aCell->xNumber-1;
	if(m1<0) m1 = 0;
	n1 = aCell->yNumber-1;
	if(n1<0) n1 = 0;
	m2 = aCell->xNumber+1;
	if(m2>region->hCells-1) m2 = region->hCells-1;
	n2 = aCell->yNumber+1;
	if(n2>region->vCells-1) n2 = region->vCells-1;


	duallist_init(&crosses);
	for(i=m1; i<=m2; i++)
		for(j=n1; j<=n2; j++) {
			aCell = region->mesh + i*region->vCells + j;
			aItem = aCell->crosses.head;
			while(aItem != NULL) {
				duallist_add_to_tail(&crosses, aItem->datap);
				aItem = aItem->next;
			}
		}

	if(crosses.nItems != 0) {
		randnum = rand()%crosses.nItems;
		aItem = crosses.head;
		for(i=0; i<randnum; i++)
			aItem = aItem->next;
		rtCross = aItem->datap;
	}  
	duallist_destroy(&crosses, NULL);
	return rtCross;
}



double produce_reports_on_road(struct Trace *aTrace, struct Road *aRoad, time_t *clock, time_t endAt, int tGran, double leftDist)
{
	struct Item *aItem;
	double pace, moveDist, dist;
	struct Point *aPoint, *bPoint, *cPoint;
	struct Report *newRep;

	/* use var3 field as speed basis*/
	pace = tGran * aTrace->var3 / 3.6;

	moveDist = leftDist;
	aItem = aRoad->points.head;
	while(aItem!=NULL && aItem->next!=NULL && *clock <= endAt) {
		aPoint = (struct Point*)aItem->datap;
		bPoint = (struct Point*)aItem->next->datap;
		dist = distance_in_meter(aPoint->x, aPoint->y, bPoint->x, bPoint->y);
		if(dist + moveDist >= pace ) {
			newRep = (struct Report*)malloc(sizeof(struct Report));
			report_init_func(newRep);
			newRep->gPoint.x = (bPoint->x-aPoint->x)*(pace-moveDist)/dist+aPoint->x;		
			newRep->gPoint.y = (bPoint->y-aPoint->y)*(pace-moveDist)/dist+aPoint->y;		
			newRep->timestamp = *clock + tGran;
			newRep->speed = aTrace->var3;
			newRep->angle = angle_between(aPoint->x, aPoint->y, bPoint->x, bPoint->y);
			newRep->state = 1;
			newRep->onRoadId = aRoad->id;
			newRep->fromTrace = aTrace;
			duallist_add_to_tail(&aTrace->reports, newRep);
			
			*clock += tGran;
			moveDist = dist + moveDist - pace;
			cPoint = &newRep->gPoint;

			while(moveDist > pace) {
				newRep = (struct Report*)malloc(sizeof(struct Report));
				report_init_func(newRep);
				newRep->gPoint.x = (bPoint->x-cPoint->x)*pace/moveDist+cPoint->x;		
				newRep->gPoint.y = (bPoint->y-cPoint->y)*pace/moveDist+cPoint->y;		
				newRep->timestamp = *clock + tGran;
				newRep->speed = aTrace->var3;
				newRep->angle = angle_between(aPoint->x, aPoint->y, bPoint->x, bPoint->y);
				newRep->state = 1;
				newRep->onRoadId = aRoad->id;
				newRep->fromTrace = aTrace;
				duallist_add_to_tail(&aTrace->reports, newRep);
				
				*clock += tGran;
				moveDist = moveDist - pace;
				cPoint = &newRep->gPoint;
			}				
		} else {
			moveDist += dist;
		}
		aItem = aItem->next;
	}
	return moveDist;
}



struct Trace* make_a_trace_free(time_t startAt, time_t endAt, int tGran, long vId, void(*point_selection_func)(struct Point*, struct Point*, double, double, double , double), double orgx, double orgy, double value3, double value4)
{
	time_t clock;
	struct Trace *newTrace;
	struct Point sPoint, dPoint;
	double moveDist;
	struct Road *aRoad;
	double timer;

	newTrace = (struct Trace*)malloc(sizeof(struct Trace));
	trace_init_func(newTrace);
	sprintf(newTrace->vName, "v%ld", vId);

	moveDist = 0;
	clock = startAt;
	sPoint.x = orgx, sPoint.y = orgy;
	timer = value4*g_random_double();
	while(clock <= endAt) {
		point_selection_func(&sPoint, &dPoint, orgx, orgy, value3, value4);
		/* temporarily use var3 to store trace speed*/
		newTrace->var3 = rand()%20+60;
		aRoad = (struct Road*)malloc(sizeof(struct Road));
		duallist_init(&aRoad->points);
		duallist_add_to_tail(&aRoad->points, &sPoint);	
		duallist_add_to_tail(&aRoad->points, &dPoint);	
		moveDist = produce_reports_on_road(newTrace, aRoad, &clock, endAt, tGran, moveDist);
		if (newTrace->reports.head!=NULL)
			/* Drop passenger at the destination */
			((struct Report*)newTrace->reports.head->prev->datap)->state = 0;
		duallist_destroy(&aRoad->points, NULL);
		free(aRoad);
		sPoint.x = dPoint.x, sPoint.y = dPoint.y;
	}
	return newTrace;
}

void random_point_selection(struct Point* sPoint, struct Point *dPoint, double v1, double v2, double range, double v4)
{
	unsigned long irange;
	double rundist, runangle;

	irange = range;
	rundist = distance_in_latitude(rand()%irange);
	runangle = rand()%360;

	dPoint->x = sPoint->x + rundist * cos(runangle*M_PI/180);
	dPoint->y = sPoint->y + rundist * sin(runangle*M_PI/180);
}


/* The normal distribution is over cell numbers on the map surface*/
void gaussian_point_selection(struct Point* sPoint, struct Point *dPoint, double xMu, double yMu, double xSigma, double ySigma)
{
	double dist1, dist2;

	dist1 = distance_in_latitude(xSigma);
	dist2 = distance_in_latitude(ySigma);
	/* from here */
	dPoint->x = dist1 * rnor() + xMu;
	dPoint->y = dist2 * rnor() + yMu;
}

struct Trace* make_a_trace_free_influx(time_t startAt, time_t endAt, int tGran, long vId, double orgx, double orgy, double range, double timer)
{
	time_t clock;
	struct Trace *newTrace;
	struct Point sPoint, dPoint;
	double moveDist;
	struct Road *aRoad;
	double dist, rundist, runangle;
	double runtime;
	double timer_;

	newTrace = (struct Trace*)malloc(sizeof(struct Trace));
	trace_init_func(newTrace);
	sprintf(newTrace->vName, "v%ld", vId);

	moveDist = 0;
	clock = startAt;
	sPoint.x = orgx, sPoint.y = orgy;


	timer_ = timer;
	while(clock <= endAt ) {
		/* temporarily use var3 to store trace speed*/
		newTrace->var3 = rand()%20+60;
		dist = range*g_random_double();
		rundist = distance_in_latitude(dist);
		runangle = rand()%360;
		runtime = dist*3.6/newTrace->var3;

		if(equald(timer_, 0, DELTA) || smallerd(timer_, 0, DELTA)) {
			dPoint.x = orgx;
			dPoint.y = orgy;
			timer_ = timer;
		} else if (runtime < timer_) {
			dPoint.x = sPoint.x + rundist * cos(runangle*M_PI/180);
			dPoint.y = sPoint.y + rundist * sin(runangle*M_PI/180);
			timer_ = timer_ - runtime; 
		} else {
			dPoint.x = sPoint.x + rundist * cos(runangle*M_PI/180) * (timer_/runtime);
			dPoint.y = sPoint.y + rundist * sin(runangle*M_PI/180) * (timer_/runtime);
			timer_ = 0;
		}
		aRoad = (struct Road*)malloc(sizeof(struct Road));
		duallist_init(&aRoad->points);
		duallist_add_to_tail(&aRoad->points, &sPoint);	
		duallist_add_to_tail(&aRoad->points, &dPoint);	
		moveDist = produce_reports_on_road(newTrace, aRoad, &clock, endAt, tGran, moveDist);
		if (newTrace->reports.head!=NULL)
			/* Drop passenger at the destination */
			((struct Report*)newTrace->reports.head->prev->datap)->state = 0;
		duallist_destroy(&aRoad->points, NULL);
		free(aRoad);
		sPoint.x = dPoint.x, sPoint.y = dPoint.y;
	}

	return newTrace;
}

