#include<stdio.h>
#include"common.h"

int main(int argc, char **argv)
{
	time_t clock;

	if(argc < 2) {
		printf("Usage: %s \"yyyy-mm-dd hh:mm:ss\"\n", argv[0]);
		return 0;
	}
	clock = strtot(argv[1]);	
	printf("time_t value is:%ld\n", clock);
	return 0;

}
