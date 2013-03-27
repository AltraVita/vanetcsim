#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "files.h"
#include "trace.h"
#include "geometry.h"

/* Our main function */
int main( int   argc,
          char *argv[] )
{
  int first;
  struct Box lbox, box;
  struct Hashtable traces;
  FILE *fsource;

  if(argc < 2) {
	printf("Usage: %s [.lst .ogd ...]\n", argv[0]);
	exit(1);
  }

  first = 1;
  while(argc>1) {
  	hashtable_init(&traces, 2000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))trace_has_name);
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[1]);
		lbox.xmin = 0;
		load_source_file(fsource, NULL, &traces, (void*(*)(int, FILE*, struct Region *, void *, time_t *, time_t *, struct Box *))load_trace_with_hashtable, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &lbox);
		fclose(fsource);
		if(lbox.xmin) {
			if(first) {
				box.xmin = lbox.xmin;
				box.xmax = lbox.xmax;
				box.ymin = lbox.ymin;
				box.ymax = lbox.ymax;
				first --;
			} else {
				if(lbox.xmin < box.xmin)
					box.xmin = lbox.xmin;
				if(lbox.xmax > box.xmax)
					box.xmax = lbox.xmax;
				if(lbox.ymin < box.ymin)
					box.ymin = lbox.ymin;
				if(lbox.ymax > box.ymax)
					box.ymax = lbox.ymax;
			}
		}
	}

  	hashtable_destroy(&traces, (void(*)(void*))trace_free_func);
	argc--;
	argv++;
  }
  printf("%.6lf %.6lf %.6lf %.6lf %.6lf %.6lf %.6lf %.6lf\n", box.xmin, box.ymin, box.xmax, box.ymin, box.xmax, box.ymax, box.xmin, box.ymax);
  return 0;
}
