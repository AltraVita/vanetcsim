
struct Path *get_path_between_two_reports(struct Region *aRegion, struct Report *aRep, struct Report *bRep, double lengthreshold)
{
	struct Road *aRoad, *bRoad;
	struct Path *aPath;
	struct Point *aPoint, *bPoint;
	struct Item *aItem;
	int i;

	if(aRegion == NULL || aRep == NULL || bRep == NULL || aRep->onRoad == NULL || bRep->onRoad == NULL) {
		return NULL;
	}

	aRoad = ((struct CandRoad*)aRep->onRoad->datap)->aRoad;
	bRoad = ((struct CandRoad*)bRep->onRoad->datap)->aRoad;
	aPoint = &((struct CandRoad*)aRep->onRoad->datap)->gPoint;
	bPoint = &((struct CandRoad*)bRep->onRoad->datap)->gPoint;

	aPath = NULL;
	if(lengthreshold == -1 || (lengthreshold!=-1&&distance_in_meter(aRep->gPoint.x, aRep->gPoint.y, bRep->gPoint.x, bRep->gPoint.y) < lengthreshold)) {
		if(aRoad == bRoad) {
			aPath = (struct Path*)malloc(sizeof(struct Path));
			duallist_init(&aPath->roads);
			duallist_add_to_tail(&aPath->roads, aRoad);
			aPath->length = distance_to_tail_cross(aRoad, aPoint) - distance_to_tail_cross(aRoad, bPoint);
		} else if(bRoad->headEnd != aRoad->tailEnd) {
			aPath = find_shortest_path(aRegion, aRoad->tailEnd, bRoad->headEnd);
			if (aPath != NULL) {
				duallist_add_to_head(&aPath->roads, aRoad);
				duallist_add_to_tail(&aPath->roads, bRoad);

				aPath->length = distance_to_tail_cross(aRoad, aPoint);
				aItem = aPath->roads.head->next;
				for(i=0;i<aPath->roads.nItems-2;i++) {
					aRoad = (struct Road*)aItem->datap;
					aPath->length += aRoad->length;
					aItem = aItem->next;
				}
				aPath->length += distance_to_head_cross(bRoad, bPoint);
			}

		} else {
			aPath = (struct Path*)malloc(sizeof(struct Path));
			duallist_init(&aPath->roads);
			duallist_add_to_head(&aPath->roads, aRoad);
			duallist_add_to_tail(&aPath->roads, bRoad);
			aPath->length = distance_to_tail_cross(aRoad, aPoint);
			aPath->length += distance_to_head_cross(bRoad, bPoint);
		}
	}
	return aPath;
}

