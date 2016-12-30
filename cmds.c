#define _XOPEN_SOURCE 500
#include<stdio.h>
#include<ftw.h>
#include<unistd.h>

int ftwdel(const char *path, const struct stat *st, int tf, struct FTW *buf){
	int r = remove(path);
	if(r){
		perror(path);
	}
	return r;
}


int deleteDir(char *path){
	return nftw(path, ftwdel, 64, FTW_DEPTH | FTW_PHYS);
}
