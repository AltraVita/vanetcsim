#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
void makelist(char*);

int main(int argc, char **argv){
	struct stat info;
	int fp;
	if(argc < 2)
	{
		printf("Usage: %s [directory list]\n",argv[0]);
		return 1;
	}
	
	argc--;
	while(argc > 0)
	{
		
		if((fp = open(argv[1],O_RDONLY))!= -1)
		{
			
			fstat(fp,&info);
			close(fp);
			if(S_ISDIR(info.st_mode)){
				makelist(argv[1]);
			}
			else {
				printf("%s is not a directory...\n",argv[1]);
			}	
		}
		argv++;
		argc--;
		
	}
	
	return 0;
}

void makelist(char* directory)
{
	struct stat info;
	struct dirent* ptr;
	FILE *filep,*listp;
	char filename[128];
	char listfile[128];
	char temp[128];
	int fp;
	int type;

	DIR* dir = opendir(directory);
	sprintf(filename,"%s",directory);
	sprintf(listfile,"%s/list.lst",filename);
	listp = fopen(listfile,"w");
	fprintf(listp,"%d\n",3);
	while((ptr = readdir(dir))!=NULL)
	{

		if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..")== 0) 
			continue;
		sprintf(temp,"%s/%s",filename,ptr->d_name);
//printf("%s\n",temp);
		if((fp = open(temp,O_RDONLY)) == -1)
			continue;
//printf("%d\n",fp);
		type= fstat(fp,&info);
//printf("%d\n",type);
		close(fp);
		
		if(S_ISDIR(info.st_mode))
		{
			
			//printf("%s\n",filename);
			makelist(temp);
		}else{
			filep = fopen(temp,"r");
			fscanf(filep,"%d\n",&type);
			fclose(filep);
			if(type != 0 && type != 6 && type != 5 && type != 7)
				continue;
			if(listp == NULL) continue;
			fprintf(listp,"data/%s\n",temp);
		}
	}
	fclose(listp);
	
	closedir(dir);
	
}
