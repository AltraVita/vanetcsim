/*
 * This is a small piece of code for calculating the distance between
 * any two two-demension GPS coordinates in meters.
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include"common.h"
#include"geometry.h"

int main(int argc, char*argv[])
{
	if(argc!=5) {
		printf("Usage: %s x1 y1 x2 y2\n", argv[0]);
		exit(0);
	}
	printf("The distance on the Earth is: %.2lf meters\n", distance_in_meter(atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4])));
	return 0;
}
