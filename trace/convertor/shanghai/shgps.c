#include <string.h>
#include "shgps.h"

int read_a_record(char *buf, int vtype, char *change_to_date, struct Record *aRcd)
{
    char *p = buf, *q;
    int i, status = 0;
   	
    memset(aRcd, '\0', 544);

    if(vtype == VEHICLE_TYPE_TAXI) { 
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->vId, p, q-p);
	else
		aRcd->vId[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->longitude, p, q-p);
	else
		aRcd->longitude[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->lattitude, p, q-p);
	else
		aRcd->lattitude[0] = '\0', status = 1;
	
	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->speed, p, q-p);
	else
		aRcd->speed[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->heading, p, q-p);
	else
		aRcd->heading[0] = '\0', status = 1;


	p = q+1;
	q = strchr(p, ',');
	strncpy(aRcd->timestamp, p, q-p);
	if(q!=p)
		strncpy(aRcd->timestamp, p, q-p);
	else
		aRcd->timestamp[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	strncpy(aRcd->D7, p, q-p);
	if(q!=p)
		strncpy(aRcd->D7, p, q-p);
	else
		aRcd->D7[0] = '\0', status = 1;


    } else {
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->msgType, p, q-p);
	else
		aRcd->msgType[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->vId, p, q-p);
	else
		aRcd->vId[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->routeId, p, q-p);
	else
		aRcd->routeId[0] = '\0', status = 1;
	
	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->longitude, p, q-p);
	else
		aRcd->longitude[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->lattitude, p, q-p);
	else
		aRcd->lattitude[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->speed, p, q-p);
	else
		aRcd->speed[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->routeLeng, p, q-p);
	else
		aRcd->routeLeng[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->timestamp, p, q-p);
	else
		aRcd->timestamp[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->gasVol, p, q-p);
	else
		aRcd->gasVol[0] = '\0';

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->D7, p, q-p);
	else
		aRcd->D7[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->D6, p, q-p);
	else
		aRcd->D6[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->D5, p, q-p);
	else
		aRcd->D5[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->D4, p, q-p);
	else
		aRcd->D4[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->D3, p, q-p);
	else
		aRcd->D3[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->D2, p, q-p);
	else
		aRcd->D2[0] = '\0', status = 1;

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');
	if(q!=p)
		strncpy(aRcd->errorInfo, p, q-p);
	else
		aRcd->errorInfo[0] = '\0';

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, ',');

	p = q+1;
	q = strchr(p, '\r');
	if( p[0] != '\0')
		strncpy(aRcd->heading, p, q-p);
	else
		aRcd->heading[0] = '\0', status = 1;

    }

    if(change_to_date != NULL) {
	for(i=0;i<10;i++)
		aRcd->timestamp[i] = change_to_date[i];
    }
    return status;
}

/* translate the Report.angle in normal sense */
int normal_angle_taxi(int repAngle)
{
	int rtAngle;

	if (90 >= 2*repAngle) 
		rtAngle = 90-2*repAngle;
	else
		rtAngle = 450-2*repAngle;
	return rtAngle;
}

int normal_angle_bus(int repAngle)
{
	int rtAngle;

	if (90 >= repAngle) 
		rtAngle = 90-repAngle;
	else
		rtAngle = 450-repAngle;
	return rtAngle;
}

int report_angle(int norAngle)
{
	int rtAngle;
	
	if(norAngle <= 90)
		rtAngle = (90-norAngle)/2;
	else
		rtAngle = (450-norAngle)/2;
	return rtAngle;
}


int are_two_reports_active(struct Report *aRep, struct Report *bRep)
{
	if(!equald(aRep->gPoint.x, bRep->gPoint.y, DELTA)
	&& !equald(aRep->gPoint.y, bRep->gPoint.y, DELTA))
		return 1;
	return 0;
}
