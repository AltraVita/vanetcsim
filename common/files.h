#ifndef FILES_H
#define FILES_H

#include<stdio.h>
#include<time.h>
#include"common.h"
#include"geometry.h"

enum
{
  FILE_ORIGINAL_GPS_TAXI,
  FILE_CONTACT_SAMPLE,
  FILE_CONTACT,
  FILE_LIST,
  FILE_CELL_DISPLAYS,
  FILE_MODIFIED_GPS_TAXI,
  FILE_ORIGINAL_GPS_BUS,
  FILE_MODIFIED_GPS_BUS,
  FILE_ICT,
  FILE_TRAJECTORY,
  FILE_SYBIL_HONEST_TRAJ,
  FILE_SYBIL_MALICIOUS_TRAJ,
  FILE_BUS_ROUTE
};

void load_source_file(FILE *fsource, struct Region *aRegion, void *traceStore, void*(*trace_load_func)(int, FILE*, struct Region*, void*, time_t*, time_t*, struct Box*), void *cntSmpStore, void*(*cntSmp_load_func)(FILE *, struct Region*, void *, time_t *, time_t *), void *cntStore, int table_mode, void*(*cnt_load_func)(FILE *, struct Region*, void *, int, time_t *, time_t *), void* cellStore, void*(*cell_displays_load_func)(FILE*, struct Region*, void *, time_t *, time_t *), void *trajStore, void*(*traj_load_func)(FILE*, void*), void *routeStore, void*(*route_load_func)(FILE*, struct Region*, void*), time_t *startAt, time_t *endAt, struct Box *box);
#endif
