/* Pick certain GPS reports from Shanghai taxi and bus trace data */

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<time.h>
#include"common.h"
#include"shgps.h"


int main(int argc, char *argv[])
{
  // deal with GPS data
  FILE *fgps;
  char buf[256], *name = NULL;
  time_t tstart = -1, tend, timestamp;
  double xmin=-1,ymin=-1, xmax=-1, ymax=-1;
  int vtype = VEHICLE_TYPE_TAXI;
  struct Record aRcd;

  if(argc < 2) {
    printf("Usage: %s [-bus] [-v name] [-c xmin ymin xmax ymax] [-t from until] GPSfile\n", argv[0]);
    exit(1);
  } 
  while( (argv[1][0])=='-' ) {
    switch ( argv[1][1]) {
      case 'b':
	vtype = VEHICLE_TYPE_BUS;
	argc-=1;
	argv+=1;
	break;
      case 'c':
        xmin = atof(argv[2]);
        ymin = atof(argv[3]);
        xmax = atof(argv[4]);
        ymax = atof(argv[5]);
        argc-=5;
        argv+=5;
        break;
      case 'v':
	name = argv[2];
        argc-=2;
        argv+=2;
        break;
      case 't':
	tstart = strtot(argv[2]);
	tend = strtot(argv[3]);
        argc-=3;
        argv+=3;
        break;
       default:
        printf("Bad option %s\n", argv[1]);
    	printf("Usage: %s [-bus] [-v name] [-c xmin ymin xmax ymax] [-t from until] GPSfile\n", argv[0]);
        exit(1);
    }
  }

  if((fgps=fopen(argv[1], "r"))==NULL) {
    printf("Cannot open %s for reading\n", argv[1]);
    exit(1);
  }

  while (fgetc(fgps)!=EOF)
  {
    fseek(fgps, -1, SEEK_CUR);
    fgets(buf, 256, fgps);

    if(read_a_record(buf, vtype, NULL, &aRcd))
	continue;
 
    if(name != NULL && strcmp( aRcd.vId, name) != 0)
	continue;
    if(xmin != -1 && (atof(aRcd.longitude)<xmin || atof(aRcd.longitude)>xmax || atof(aRcd.lattitude)<ymin || atof(aRcd.lattitude)>ymax))
	continue;
    timestamp = strtot(aRcd.timestamp);
    if(tstart!=-1 && (timestamp < tstart || timestamp > tend))
	continue;

    printf("%s", buf);
  }
  fclose(fgps);
  return 0;
}
                                                                                                                             

