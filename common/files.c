#include<stdio.h>
#include<string.h>
#include<time.h>
#include"common.h"
#include"files.h"


void load_source_file(FILE *fsource, struct Region *aRegion, void *traceStore, void*(*trace_load_func)(int, FILE*, struct Region*, void*, time_t*, time_t*, struct Box*), void *cntSmpStore, void*(*cntSmp_load_func)(FILE *, struct Region*, void *, time_t *, time_t *), void *cntStore, int table_mode, void*(*cnt_load_func)(FILE *, struct Region*, void *, int, time_t *, time_t *), void* cellStore, void*(*cell_displays_load_func)(FILE*, struct Region*, void *, time_t *, time_t *), void *trajStore, void*(*traj_load_func)(FILE*, void*), void *routeStore, void*(*route_load_func)(FILE*, struct Region*, void*), time_t *startAt, time_t *endAt, struct Box *box)
{
	char buf[128], filename[128];
	FILE *fp;
	int magicNumber, first;
	time_t sAt, eAt, ssAt, eeAt;
	struct Box aBox, aaBox;

	first = 1;
	fscanf(fsource, "%d\n", &magicNumber);
	if(magicNumber == FILE_LIST) {
		memset(buf, '\0', 128);
		while (fgets(buf, 127, fsource)) {
			sscanf(buf, "%s", filename);
			if( (fp=fopen(filename, "r"))!=NULL) {
				if(startAt)
					sAt = *startAt;
				else
					sAt = 0;
				if(endAt)
					eAt = *endAt;
				else
					eAt = 0;
				if(box) {
					aBox.xmin = box->xmin;
					aBox.xmax = box->xmax;
					aBox.ymin = box->ymin;
					aBox.ymax = box->ymax;
				} else {
					aBox.xmin = 0;
					aBox.xmax = 0;
					aBox.ymin = 0;
					aBox.ymax = 0;
				}
				load_source_file(fp, aRegion, traceStore, trace_load_func, cntSmpStore, cntSmp_load_func, cntStore, table_mode, cnt_load_func, cellStore, cell_displays_load_func, trajStore, traj_load_func, routeStore, route_load_func, &sAt, &eAt, &aBox);
				fclose(fp);
				if(sAt) {
					if(first) {
						ssAt = sAt;
						eeAt = eAt;
						aaBox.xmin = aBox.xmin;
						aaBox.xmax = aBox.xmax;
						aaBox.ymin = aBox.ymin;
						aaBox.ymax = aBox.ymax;
						first --;
					} else {
						if (sAt < ssAt)
							ssAt = sAt;
						if (eAt > eeAt)
							eeAt = eAt;
						if (aBox.xmin < aaBox.xmin)
							aaBox.xmin = aBox.xmin;
						if (aBox.xmax > aaBox.xmax)
							aaBox.xmax = aBox.xmax;
						if (aBox.ymin < aaBox.ymin)
							aaBox.ymin = aBox.ymin;
						if (aBox.ymax > aaBox.ymax)
							aaBox.ymax = aBox.ymax;
					}
				}
			}
			memset(buf, '\0', 128);
		}
	} else {
		if(startAt)
			ssAt = *startAt;
		else
			ssAt = 0;
		if(endAt)
			eeAt = *endAt;
		else
			eeAt = 0;
		if(box) {
			aaBox.xmin = box->xmin;
			aaBox.xmax = box->xmax;
			aaBox.ymin = box->ymin;
			aaBox.ymax = box->ymax;
		} else {
			aaBox.xmin = 0;
			aaBox.xmax = 0;
			aaBox.ymin = 0;
			aaBox.ymax = 0;
		}

		if (magicNumber == FILE_ORIGINAL_GPS_TAXI 
			|| magicNumber == FILE_MODIFIED_GPS_TAXI
			|| magicNumber == FILE_ORIGINAL_GPS_BUS
			|| magicNumber == FILE_MODIFIED_GPS_BUS) {
			trace_load_func(magicNumber, fsource, aRegion, traceStore, &ssAt, &eeAt, &aaBox);

		} else if (magicNumber == FILE_CONTACT_SAMPLE) {
			cntSmp_load_func(fsource, aRegion, cntSmpStore, &ssAt, &eeAt);

		} else if (magicNumber == FILE_CONTACT) {
			cnt_load_func(fsource, aRegion, cntStore, table_mode, &ssAt, &eeAt);

		} else if(magicNumber == FILE_CELL_DISPLAYS) {
			fscanf(fsource, "%lf %lf\n", &aRegion->lower, &aRegion->upper);
			cell_displays_load_func(fsource, aRegion, cellStore, &ssAt, &eeAt);

		} else if (magicNumber == FILE_TRAJECTORY) {
			traj_load_func(fsource, trajStore);

		} else if (magicNumber == FILE_BUS_ROUTE) {
			route_load_func(fsource, aRegion, routeStore);
		}
	}

	if(startAt)
		*startAt = ssAt;
	if(endAt)
		*endAt = eeAt;
	if(box) {
		box->xmin = aaBox.xmin;
		box->xmax = aaBox.xmax;
		box->ymin = aaBox.ymin;
		box->ymax = aaBox.ymax;
	}
}

