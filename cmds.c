#include<stdlib.h>
#include<stdio.h>
#include<dirent.h>
#include<string.h>

char dele(char *path){
	int i;
	for(i= strlen(path); i>0; i--){
		if(path[i]== '/'){
			path[i+1]= '\0';
			break;
		}
	}
	return *path;
}

int deleteDir(char *path){
	char *file;
	printf("%s", path);
	DIR *dir= opendir(path);
	if(dir== NULL){
		exit(0);
	}
	struct dirent *ent;
	while((ent= readdir(dir))!= NULL){
		if(strcmp(ent->d_name, "..")== 0 || strcmp(ent->d_name, ".")== 0){
			continue;
		}
		strcat(file, ent->d_name);
		remove(file);
		dele(file);
	}
	closedir(dir);
	remove(path);
	return 0;
}
