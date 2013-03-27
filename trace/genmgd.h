#ifndef GENMGD_H
#define GENMGD_H

double produce_reports_on_road(struct Trace *aTrace, struct Road *aRoad, time_t *clock, time_t endAt, int tGran, double leftDist);

struct Trace* make_a_trace_map(struct Region *region, time_t startAt, time_t endAt, int tGran, long vId, struct Cross*(*cross_selection_func)(struct Region*, struct Cross*, double, double, double, double), struct Path*(*find_path_func)(struct Region*, struct Cross*, struct Cross*), double value1, double value2, double value3, double value4);
struct Trace* make_a_trace_map_influx(struct Region *region, time_t startAt, time_t endAt, int tGran, long vId, double orgX, double orgY, double range, double timer, struct Path*(*find_path_func)(struct Region*, struct Cross*, struct Cross*));
struct Cross* initial_cross_selection(struct Region* region, double orgX, double orgY);
struct Cross* random_cross_selection(struct Region* region, struct Cross* atCross, double value1, double value2, double range, double value4);
struct Cross* gaussian_cross_selection(struct Region* region, struct Cross* atCross, double xMu, double yMu, double xSigma, double ySigma);

struct Trace* make_a_trace_free(time_t startAt, time_t endAt, int tGran, long vId, void(*point_selection_func)(struct Point*, struct Point*, double, double, double , double), double orgx, double orgy, double value3, double value4);
struct Trace* make_a_trace_free_influx(time_t startAt, time_t endAt, int tGran, long vId, double orgx, double orgy, double range, double timer);

void random_point_selection(struct Point* sPoint, struct Point *dPoint, double v1, double v2, double range, double v4);
void gaussian_point_selection(struct Point* sPoint, struct Point *dPoint, double xMu, double yMu, double xSigma, double ySigma);
#endif
