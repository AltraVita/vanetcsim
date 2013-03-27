#ifndef SHP_H
#define SHP_H

struct Point
{
  double x;
  double y;
};

  
struct PolylineRcd
{
  double box[4];
  int nParts;
  int nPoints;
  int *parts;
  struct Point *points;
};

struct PolygonRcd
{
  double box[4];
  int nParts;
  int nPoints;
  int *parts;
  struct Point *points;
};

struct PolygonMRcd
{
  double box[4];
  int nParts;
  int nPoints;
  int *parts;
  struct Point *points;
  double MRange[2];
  double *measures;
};

struct Record_header
{
  int number;
  int length;
};

struct Polyline_record
{
  struct Record_header header;
  int type;
  struct PolylineRcd apolyline;

  /* attributes from .shp file*/
  char lable[32];
  int width;
  char direction;
};

struct Polygon_record
{
  struct Record_header header;
  int type;
  struct PolygonRcd apolygon;
};

struct PolygonM_record
{
  struct Record_header header;
  int type;
  struct PolygonMRcd apolygonM;
};

#endif
