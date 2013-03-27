#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include<stdlib.h>
#define MAX_NUM 2000

int main(int args, char** argv)
{
	char listfile[128], dump[128];
	char listItem[128],chlst[128];
	char *directory;
	char *fileItem;
	char items[MAX_NUM][128];
	
	int len,i,count=0;
	char *p,*q;
	int fp;
	struct stat info;
	struct dirent* ptr;
	DIR* dir;
	FILE *list,*changelist;
		
	if(args < 3)
	{
		printf("Usage: %s .lst [directory]\n", argv[0]);
		return 1;
	}
	
	sprintf(listfile,"%s",argv[1]);
	directory = argv[2];
	
	strcpy(dump,listfile);
	p = strtok(dump,"/");
	q = p;
	while((p = strtok(NULL,"/")) != NULL)
	{
		q = p;
	}
	i = q-dump;
	//printf("%d\n",i);
	strncpy(chlst,listfile,i-1);
	sprintf(chlst,"%s/lchlist.lst",chlst);
	//printf("%s\n",chlst);
	
	i = MAX_NUM;
	if((fp = open(directory,O_RDONLY)) == -1)
	{
		printf("%s cannot be opened!\n",directory);
		return 1;
	}
	
	if(fstat(fp,&info) == -1)
	{
		printf("Geting the info is failed!\n");
		return 1;
	}
	close(fp);
	
	if(S_ISDIR(info.st_mode))
	{
		if((dir = opendir(directory)) == NULL)
		{
			printf("%s cannot be opened as directory\n",directory);
			return 1;
		}
		
		while((ptr = readdir(dir))!= NULL)
		{
			if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..")== 0) continue;
			strcpy(dump,ptr->d_name);
			fileItem = strtok(dump,".");
			sprintf(items[--i],"%s",fileItem);
		}
		
		//for( i = MAX_NUM-1; i >= len; i--)
		//{
		//	printf("%s %d\n",items[i],i);
		//}
		
		closedir(dir);
	}
	len = i;
	
	if((list = fopen(listfile,"r+")) == NULL || (changelist = fopen(chlst,"w")) == NULL)
	{
		printf("%s does not exist or %s cannot be opened\n",listfile,chlst);
		return 1;
	}

	while(fscanf(list,"%s\n",listItem) == 1)
	{
		strcpy(dump,listItem);
		p = strtok(dump,"/");
		q = p;
		while((p = strtok(NULL,"/")) != NULL) q = p;
		p = strtok(q,".");
		
		for( i = MAX_NUM-1; i >= len; i--)
		{
			if(strcmp(p,items[i]) == 0)
			{ 
				count++;
				break;
			}
		}
		
		if(i < len){
			//count++;
			fprintf(changelist,"%s\n",listItem);
		}
	}
	printf("%d\n",count);
	fclose(changelist);
	fclose(list);
}
