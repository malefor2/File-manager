/*
 * TODO:
 * search multiple hits
 * permissions
 * selecting multiple files
 * rewrite list files function
 * dynamic array for files
 * more commands, searching
 * sort out colors
 * better controls
 * config file
 * fix deletedir
 */

/* BUGS:
 */

#include<stdio.h>
#include<curses.h>
#include<dirent.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h>
#include"cmds.h"

enum sorting{
	SIZE,
	NAME
};

DIR *dir;
int sel= 0;
char del(char*);
char curdir[255];
int row, col;
int find(char*, int);
int sort = NAME;

typedef struct{
	char name[255];
	int dir;
	int size;
}fileInfo;

fileInfo file[10000];

int compareName(const void *s1, const void *s2){
	fileInfo *fileInfo1 = (fileInfo *)s1;
	fileInfo *fileInfo2 = (fileInfo *)s2;
	return strcasecmp(fileInfo1->name, fileInfo2->name);
}

int compareSize(const void *s1, const void *s2){
	fileInfo *fileInfo1 = (fileInfo *)s1;
	fileInfo *fileInfo2 = (fileInfo *)s2;
	return (fileInfo2->size - fileInfo1->size);
}

int listFiles(bool show_hidden){
	int n= 0;
	struct stat type;
	int j= 0;
	char path[255];
	strcpy(path, curdir);
	strcat(path, "/");
	dir= opendir(curdir);
	if(dir == NULL){
		printf("opendir fail");
		endwin();
		exit(1);
	}
	struct dirent *ent;
	while((ent= readdir(dir))!= NULL){
		if(strcmp(ent->d_name, "..")== 0 || strcmp(ent->d_name, ".")== 0){
			continue;
		}
		if(!show_hidden){
			if(ent->d_name[0]!= '.'){
				strcat(path, ent->d_name);
				stat(path, &type);
				file[n].size= type.st_size;
				if(S_ISDIR(type.st_mode)){
					file[n].dir= 1;
				}
				else{
					file[n].dir= 0;
				}
				strcpy(file[n].name, ent->d_name);
				n++;
				del(path);
			}
		}
		else{
			strcat(path, ent->d_name);
			stat(path, &type);
			file[n].size= type.st_size;
			if(S_ISDIR(type.st_mode)){
				file[n].dir= 1;
			}
			else{
				file[n].dir= 0;
			}
			strcpy(file[n].name, ent->d_name);
			n++;
			del(path);
		}
	}
	if(sort == SIZE){
		qsort(file, n, sizeof(fileInfo), compareSize);
	}
	else if(sort == NAME){
		qsort(file, n, sizeof(fileInfo), compareName);
	}
	return n;
}

WINDOW *create_bar(int len){
	WINDOW *loc_bar;
	loc_bar= newwin(1, col-2, row-2, 1);
	wbkgd(loc_bar, COLOR_PAIR(2));
	wattron(loc_bar, A_BOLD);
	wprintw(loc_bar, "%s", curdir);
	mvwprintw(loc_bar, 0, col-12, "%4i/%4i", sel+1, len, len);
	wattroff(loc_bar, A_BOLD);
	wrefresh(stdscr);
	wrefresh(loc_bar);
	wclear(loc_bar);
	return loc_bar;
}

WINDOW *list_window(int len, int cam){
	WINDOW *win;
	if(cam<0)
		cam= 0;
	if(len>row-2){
		if(cam>len-row+1)
			cam= len-row+2;
	}
	win= newwin(row-2, col, 0, 0);
	int w;
	if(len>row-2){
		w= cam+row-2;
	}
	else{
		w= len;
	}
	for(int i= cam; i<w; i++){
		if(sel== i){
			wattron(win, A_REVERSE);
			if(file[i].dir== 1){
				wattron(win, A_BOLD);
			}
			wprintw(win, "%s\n", file[i].name);
			wattroff(win, A_BOLD);
			wattroff(win, A_REVERSE);

		}
		else{
			if(file[i].dir== 1){
				wattron(win, COLOR_PAIR(1));
				wattron(win, A_BOLD);
				wprintw(win, "%s\n", file[i].name);
				wattroff(win, A_BOLD);
				wattroff(win, COLOR_PAIR(1));
			}
			else{
				wprintw(win, "%s\n", file[i].name);
			}
		}
		mvwprintw(win, i, col-10,  "%d\n", file[i].size);
		wrefresh(stdscr);
		wrefresh(win);
	}
	return win;
}

void go_back(char buf[]){
	int i, k, l=0;
	int s= strlen(curdir);
	for(k= s; k>=0; k--){
		if(curdir[k]== '/'){
			for(k++; k<s; k++){
				buf[l]= curdir[k];
				l++;
			}
			buf[l]= '\0';
			break;
		}
	}
	for(i= s; i>0; i--){
		if(curdir[i]== '/'){
			curdir[i] = '\0';
			break;
		}
	}
	if(i<=2){
		curdir[1]= '\0';
	}
	return;
}

char del(char *path){
	int i;
	for(i= strlen(path); i>0; i--){
		if(path[i]== '/'){
			path[i+1]= '\0';
			break;
		}
	}
	return *path;
}

int find(char *obj, int len){
	int i = 0;
	int found[100];
	for(i; i<len; i++){
		if(strstr(file[i].name, obj) != NULL){
			return i;
		}
	}
	return sel;
}

void getCommand(int len){
	char cmd[2][255];
	char path[255];
	strcpy(path, curdir);
	strcat(path, "/");
	strcat(path, file[sel].name);
	echo();
	mvaddch(row-1, 1, ':');
	mvscanw(row-1, 2, "%s %s", cmd[0], cmd[1]);
	if(strcmp(cmd[0], "delete")== 0){
		if(file[sel].dir== 1){
			mvprintw(row-1, 1, "%s is a directory, delete?[y/n]", file[sel].name);
			if(getch()== 'y'){
				deleteDir(path);
				noecho();
				return;
			}
			else{
				noecho();
				return;
			}
		}
		int r= remove(path);
		if(r== 0){
			printw("file deleted");
		}
		else{
			printw("error deleting file");
		}
	}
	if(strcmp(cmd[0], "find")== 0){
		sel = find(cmd[1], len);
		noecho();
		return;
	}
	if(strcmp(cmd[0], "sort")==0){
		if(strcmp(cmd[1], "size")){
			sort = 1;
		}
		else if(strcmp(cmd[1], "name")){
			sort = 0;
		}
	}
	noecho();
	return;
}

int main(int argc, char *argv[]){
	if(argc<2){
		getcwd(curdir, sizeof(curdir));
	}
	else{
		strcpy(curdir, argv[1]);
	}
	initscr();
	use_default_colors();
	start_color();
	use_default_colors();
	init_color(COLOR_RED, 28, 28, 28);
	init_pair(1, COLOR_GREEN, -1);
	init_pair(2, COLOR_BLUE, COLOR_RED);
	curs_set(0);
	keypad(stdscr, true);
	getmaxyx(stdscr, row, col);
	noecho();
	int histrow, histcol;
	int ch;
	int len;
	bool hidden= false;
	int view= 0;
	char buf[255];
	char *kek = "reset";
	scrollok(stdscr, true);
	len= listFiles(hidden);
	create_bar(len);
	while(ch!= 'q'){
		closedir(dir);
		getmaxyx(stdscr, row, col);
		len= listFiles(hidden);
		if(histrow!= row || histcol!= col){
			clear();
		}
		histrow= row;
		histcol= col;
		move(row-1, 1);
		if(len<1){
			clear();
		}
		if(len-1<sel){
			sel=len-1;
		}
		list_window(len, view);
		create_bar(len);
		switch(ch= getch()){
			case KEY_UP:
				if(sel== 0 || len<1){
					break;
				}
				sel--;
				if(len>row-2){
					if(sel<view+2){
						view--;
					}
				}
				break;
			case KEY_DOWN:
				if((sel+1)==len){
					break;
				}
				sel++;
				if(len>row-2){
					if(sel>view+row-5){
						view++;
					}
				}
				break;
			case 10: case KEY_RIGHT:
				if(file[sel].dir== 1){
					if(strlen(curdir)>2){
						strcat(curdir, "/");
					}
					strcat(curdir, file[sel].name);
					curdir[strlen(curdir)+1]='\0';
					sel= 0;
					view= 0;
				}
				len= listFiles(hidden);
				break;
			case KEY_LEFT: case KEY_BACKSPACE:
				go_back(buf);
				view= sel-row/2;
				len= listFiles(hidden);
				sel = find(buf, len);
				break;
			case 'S':
				endwin();
				system("bash & cd ~/code");
				exit(EXIT_SUCCESS);
				break;
			case ':':
				getCommand(len);
				clear();
				len= listFiles(hidden);
				break;
			case 8:
				sel= 0;
				view= 0;
				hidden= !hidden;
				len= listFiles(hidden);
				break;
			case KEY_HOME:
				sel= 0;
				view= 0;
				break;
			case KEY_END:
				sel= len;
				view= len-row+3;
				break;
			case KEY_NPAGE:
				sel+= row;
				view+= row;
				if(sel>len){
					sel= len;
				}
				if(view>(len-row)){
					view= len-row+3;
				}
				break;
			case KEY_PPAGE:
				sel-= row;
				view-= row;
				if(sel<0 || view< 0){
					sel= 0;
					view= 0;
				}
				break;
			default:
				break;
		}
		if(sel > view+row || sel < view){
			view = sel-row/2;
		}
		refresh();
	}
	endwin();
	return 0;
}
