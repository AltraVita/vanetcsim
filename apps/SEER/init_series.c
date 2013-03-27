#include<stdio.h>
#include<math.h>
#include<stdlib.h>

struct Sample
{
  unsigned No;
  double value;
  struct Sample *next;
};

int main(int argc, char**argv)
{
	FILE *fx, *fy, *ftest;
	unsigned cols, rows;
	unsigned *order, tmp, from, to;
	double *matrixX, *totalvalue, *count, ratio;
	char buffer[65535], *p;
	int firstrow, i, j;

	if(argc < 3) {
		printf("Usage: %s X ratio\n", argv[0]);
		exit(0);
	}
	ratio = atof(argv[2]);
	
	if((fx=fopen(argv[1], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[1]);
		exit(-1);
	}
	if((fy=fopen("original_series", "w"))==NULL) {
		printf("Cannot write to file original_series!\n");
		exit(-1);
	}
	if((ftest=fopen("test_list", "w"))==NULL) {
		printf("Cannot write to file test_list!\n");
		exit(-1);
	}
	

	rows = 0;
	firstrow = 1;
	while (fgetc(fx)!=EOF)
	{
		fseek(fx, -1, SEEK_CUR);
		
		fgets(buffer, 65535, fx);
		rows +=1;
		if(firstrow) {
			p = strtok(buffer, "\t");
			cols = 1;
			while(1){
				p = strtok(NULL, "\t");
				if(p[0]=='\n') 
					break;
				else	
					cols +=1;
			}
			firstrow = 0;
		}
	}

	matrixX = (double *)malloc(sizeof(double)*rows*cols);
	totalvalue = (double *)malloc(sizeof(double)*cols);
	count = (double *)malloc(sizeof(double)*cols);
	for (j = 0; j<cols; j++) {
		totalvalue[j] =0;
		count[j]=0;
	}

	fseek(fx, 0, SEEK_SET);
	for(i = 0; i<rows; i++) {
		fgets(buffer, 65535, fx);
		for(j=0;j<cols; j++) {
			if(j==0) 
				p = strtok(buffer, "\t");
			else 
				p = strtok(NULL, "\t");

			if(strcmp(p, "NaN")==0)
				matrixX[i*cols+j] = -1;
			else {	
				matrixX[i*cols+j] = atof(p);
				totalvalue[j] += matrixX[i*cols];
				count[j] += 1;
			}
		}
	}
	for (j = 0; j<cols; j++) {
		if(count[j]!=0)
			totalvalue[j] /= count[j];
		else
			totalvalue[j] = 0;
	}


	for (i = 0; i<rows; i++) {
		for(j = 0; j< cols;j ++) {
			if(matrixX[i*cols +j] != -1)
				matrixX[i*cols+j] -= totalvalue[j];
			else
				matrixX[i*cols+j] = 0;
		}
	}



	order = (unsigned*)malloc(sizeof(unsigned)*rows*cols);
	for(i = 0; i<rows*cols; i++)
		order[i] = i;
	for (i = 0; i<10000; i++) {
		from = rand()%(rows*cols);
		to = rand()%(rows*cols);
		tmp = order[from];
		order[from]=order[to];
		order[to]=tmp;
	}
	tmp = (rows*cols)*ratio;
	for (i = 0; i<tmp; i++) {
		if(matrixX[order[i]]!=0){
			fprintf(ftest, "%d\t%lf\t\n", order[i], matrixX[order[i]]);
			matrixX[order[i]] = 0;
		}
	}
	free(order);

	for (i = 0; i<rows; i++) {
		for(j = 0; j< cols;j ++) {
			fprintf(fy, "%lf\t", matrixX[i*cols+j]);
		}
		fprintf(fy, "\n");
	}
	

	fclose (fx);
	fclose (fy);
	fclose (ftest);
	free(matrixX);
	free(totalvalue);
	free(count);
	return 0;

}
