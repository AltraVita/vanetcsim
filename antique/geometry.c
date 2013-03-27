

/*
double distance_to_tail_cross(struct Road *aRoad, struct Point *fromPoint)
{
	struct Item *aItem;
	struct Point *aPoint, *bPoint;
	struct Segment aSeg;
	double dist = -1;

	aItem = aRoad->points.head;
	while(aItem != NULL) {
		if(aItem->next != NULL) {
			aPoint = (struct Point*)aItem->datap;
			bPoint = (struct Point*)aItem->next->datap;
			aSeg.aPoint.x = aPoint->x;
			aSeg.aPoint.y = aPoint->y;
			aSeg.bPoint.x = bPoint->x;
			aSeg.bPoint.y = bPoint->y;
			if(is_point_on_segment(fromPoint, &aSeg)) 
				break;
		}
		aItem = aItem->next;
	}
	if(aItem != NULL) {
		dist = distance_in_meter(fromPoint->x, fromPoint->y, bPoint->x, bPoint->y);
		aItem = aItem->next;
		while(aItem!=NULL) {
			if(aItem->next != NULL) {
				aPoint = (struct Point*)aItem->datap;
				bPoint = (struct Point*)aItem->next->datap;
				dist += distance_in_meter(aPoint->x, aPoint->y, bPoint->x, bPoint->y);
			}
			aItem = aItem->next;
		}
	} else 
		printf("Point is not on road.\n");
	return dist;
}

double distance_to_head_cross(struct Road *aRoad, struct Point *fromPoint)
{
	struct Item *aItem;
	struct Point *aPoint, *bPoint;
	struct Segment aSeg;
	double dist = -1;

	aItem = aRoad->points.head;
	while(aItem != NULL) {
		if(aItem->next != NULL) {
			aPoint = (struct Point*)aItem->datap;
			bPoint = (struct Point*)aItem->next->datap;
			aSeg.aPoint.x = aPoint->x;
			aSeg.aPoint.y = aPoint->y;
			aSeg.bPoint.x = bPoint->x;
			aSeg.bPoint.y = bPoint->y;
			if(is_point_on_segment(fromPoint, &aSeg)) 
				break;
		}
		aItem = aItem->next;
	}
	if(aItem != NULL) {
		dist = distance_in_meter(fromPoint->x, fromPoint->y, aPoint->x, aPoint->y);
		while(aItem!=aRoad->points.head) {
			aPoint = (struct Point*)aItem->datap;
			bPoint = (struct Point*)aItem->prev->datap;
			dist += distance_in_meter(aPoint->x, aPoint->y, bPoint->x, bPoint->y);
			aItem = aItem->prev;
		}
	} else 
		printf("Point is not on road.\n");
	return dist;
}


double distance_on_path(struct Path *aPath, struct Point *fromPoint, struct Point *toPoint)
{
	struct Item *aItem;
	struct Road *aRoad;
	double dist;
	int i;

	if(aPath == NULL) return 0;

	aRoad = (struct Road*)aPath->roads.head->datap;
	if(aPath->roads.nItems == 1) {
		if(fromPoint == NULL && toPoint == NULL)
			dist = aRoad->length;
		else if (fromPoint != NULL && toPoint == NULL)
			dist = distance_to_tail_cross(aRoad, fromPoint);
		else if (fromPoint == NULL && toPoint != NULL)
			dist = distance_to_head_cross(aRoad, toPoint);
		else
			dist = distance_to_tail_cross(aRoad, fromPoint) - distance_to_tail_cross(aRoad, toPoint);
	} else {
		if(fromPoint == NULL)
			dist = aRoad->length;
		else
			dist = distance_to_tail_cross(aRoad, fromPoint);
		aItem = aPath->roads.head->next;
		for(i=0;i<aPath->roads.nItems-2;i++) {
			aRoad = (struct Road*)aItem->datap;
			dist += aRoad->length;
			aItem = aItem->next;
		}
		aRoad = (struct Road*)aItem->datap;
		if(toPoint == NULL)
			dist += aRoad->length;
		else
			dist += distance_to_head_cross(aRoad, toPoint);
	}
	return dist;
}

double distance_on_path_(struct Path *aPath, struct Segment *fromSeg, struct Point *fromPoint, struct Segment *toSeg, struct Point *toPoint)
{
	struct Duallist *pointsPath, points;
	struct Item *aItem;
	struct Point *aPoint, *bPoint, *newPoint;
	struct Segment aSeg;
	double dist;
	
	if(aPath == NULL) return 0;

	duallist_init(&points);

	pointsPath = polyline_on_path(aPath);
	aItem = pointsPath->head;
	while(aItem != NULL) {
		if(aItem->next != NULL) {
			aPoint = (struct Point*)aItem->datap;
			bPoint = (struct Point*)aItem->next->datap;
			aSeg.aPoint.x = aPoint->x;
			aSeg.aPoint.y = aPoint->y;
			aSeg.bPoint.x = bPoint->x;
			aSeg.bPoint.y = bPoint->y;
			if(segment_equal_func(&aSeg, fromSeg)) 
				break;
		}
		aItem = aItem->next;
	}
	if(aItem != NULL) {
		newPoint = (struct Point*)malloc(sizeof(struct Point));
		newPoint->x = fromPoint->x;
		newPoint->y = fromPoint->y;
		duallist_add_to_tail(&points, newPoint);
		if(segment_equal_func(fromSeg, toSeg)) {
			newPoint = (struct Point*)malloc(sizeof(struct Point));
			newPoint->x = toPoint->x;
			newPoint->y = toPoint->y;
			duallist_add_to_tail(&points, newPoint);
		} else {
			aItem = aItem->next;
			while(aItem != NULL) {
				if(aItem->next != NULL) {
					aPoint = (struct Point*)aItem->datap;
					bPoint = (struct Point*)aItem->next->datap;
					aSeg.aPoint.x = aPoint->x;
					aSeg.aPoint.y = aPoint->y;
					aSeg.bPoint.x = bPoint->x;
					aSeg.bPoint.y = bPoint->y;
					if(!segment_equal_func(&aSeg, toSeg)) {
						newPoint = (struct Point*)malloc(sizeof(struct Point));
						newPoint->x = aPoint->x;
						newPoint->y = aPoint->y;
						duallist_add_to_tail(&points, newPoint);
					} else 
						break;
				}
				aItem = aItem->next;
			}
			if(aItem != NULL) {
				aPoint = (struct Point*)aItem->datap;
				newPoint = (struct Point*)malloc(sizeof(struct Point));
				newPoint->x = aPoint->x;
				newPoint->y = aPoint->y;
				duallist_add_to_tail(&points, newPoint);

				newPoint = (struct Point*)malloc(sizeof(struct Point));
				newPoint->x = toPoint->x;
				newPoint->y = toPoint->y;
				duallist_add_to_tail(&points, newPoint);
			}
		}
	}
			
	dist = polyline_length_meter(&points);
	duallist_destroy(pointsPath, free);
	duallist_destroy(&points, free);
	free(pointsPath);
	return dist;
}

*/
