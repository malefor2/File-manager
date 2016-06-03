#include<stdlib.h>
#include<stdio.h>

int deleteFile(char *path){
	int r= remove(path);
	if(r== 0){
		return 1;
	}
	else
		return 0;
}
