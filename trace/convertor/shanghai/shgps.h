#ifndef SHGPS_H
#define SHGPS_H

#include "trace.h"

struct Record
{
	char vId[32];
	char msgType[32];
	char routeId[32];
	char longitude[32];
	char lattitude[32];
	char speed[32];
	char heading[32];
	char routeLeng[32];
	char timestamp[32];
	char gasVol[32];
	char D7[32];
	char D6[32];
	char D5[32];
	char D4[32];
	char D3[32];
	char D2[32];
	char errorInfo[32];
};
	
int read_a_record(char *buf, int vtype, char *change_to_date, struct Record *aRcd);

int normal_angle_taxi(int repAngle);
int normal_angle_bus(int repAngle);
int report_angle(int norAngle);
int are_two_reports_active(struct Report *aRep, struct Report *bRep);

#endif
