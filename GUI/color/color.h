#ifndef COLOR_H
#define COLOR_H
 
struct Color
{
  double red;
  double green;
  double blue;
};

struct Colormap
{
  int nColors;
  struct Color *colors;
};

struct RGBO
{
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char opacity;
};

union Int_RGBO
{
  unsigned integer;
  struct RGBO rgbo;
};

#endif
