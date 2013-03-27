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
	double ratio;
	FILE *fx, *fy=NULL, *ftest;
	unsigned cols, rows;
	double *matrixX, *matrixY;
	char buffer[65535], *p;
	int firstrow, i, j;
	struct Sample *samples = NULL, *newp, *tp;
	int converge = 1, number = 0;
	double error;
		

	if(argc < 4) {
		printf("Usage: %s X Y test_list \n", argv[0]);
		exit(0);
	}
	
	if((fx=fopen(argv[1], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[1]);
		exit(-1);
	}
	if((fy=fopen(argv[2], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[2]);
		exit(-1);
	}
	if((ftest=fopen(argv[3], "r"))==NULL) {
		printf("Cannot open %s to read!\n", argv[3]);
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
			p = strtok(buffer, "\t \n");
			cols = 1;
			while(1){
				p = strtok(NULL, "\t \n");
				if(p==NULL) 
					break;
				else	
					cols +=1;
			}
			firstrow = 0;
		}
	}

	matrixX = (double *)malloc(sizeof(double)*rows*cols);
	matrixY = (double *)malloc(sizeof(double)*rows*cols);

	fseek(fx, 0, SEEK_SET);
	for(i = 0; i<rows; i++) {
		fgets(buffer, 65535, fx);
		for(j=0;j<cols; j++) {
			if(j==0) 
				p = strtok(buffer, "\t \n");
			else 
				p = strtok(NULL, "\t \n");

			matrixX[i*cols+j] = atof(p);
		}
	}
	for(i = 0; i<rows; i++) {
		fgets(buffer, 65535, fy);
		for(j=0;j<cols; j++) {
			if(j==0) 
				p = strtok(buffer, "\t \n");
			else 
				p = strtok(NULL, "\t \n");

			matrixY[i*cols+j] = atof(p);
		}
	}
	while (fgetc(ftest)!=EOF)
	{
		fseek(ftest, -1, SEEK_CUR);
		
		fgets(buffer, 65535, ftest);
		newp = (struct Sample*)malloc(sizeof(struct Sample));

		p = strtok(buffer, "\t \n");
		newp->No = atoi(p);
		p = strtok(NULL, "\t \n");
		newp->value = atof(p);
		newp->next = NULL;
		if(samples == NULL) {
			samples = newp;
			tp = newp;
		} else {
			tp->next = newp;
			tp = newp;
		}
	}

	tp = samples;
	while(tp!=NULL) {
		if (matrixX[tp->No] - matrixY[tp->No] < -0.05 ||  matrixX[tp->No] - matrixY[tp->No] > 0.05) {
			converge = 0;
			break;
		}
		tp = tp->next;
	}

	fclose (fx);
	fclose (fy);
	fclose (ftest);

	if(converge) {
		number = 0;
		error = 0;
		tp = samples;
		while(tp!=NULL) {
			error += (tp->value-matrixY[tp->No])*(tp->value-matrixY[tp->No]);
			number ++;
			tp = tp->next;
		}
		error = sqrt(error/number);
		printf("Converge! RMSD is %lf\n", error);
		
	} else {
		printf("Not converge! \n");
	}

	if((fx=fopen("filled_series", "w"))==NULL) {
		printf("Cannot open %s to write!\n", "filled_series");
		exit(-1);
	}
	
	tp = samples;
	while(tp!=NULL) {
		matrixX[tp->No] = matrixY[tp->No];
		tp = tp->next;
	}

	for (i = 0; i<rows; i++) {
		for(j = 0; j< cols;j ++) {
			fprintf(fx, "%lf\t", matrixX[i*cols+j]);
		}
		fprintf(fx, "\n");
	}
	fclose(fx);	
	printf("File origin_series is updated.\n");
				

	free(matrixX);
	free(matrixY);
	while(samples!=NULL) {
		tp = samples->next;
		free(samples);
		samples = tp;
	}
	return 0;

}
