/* TODO:
 * sorting
 * rewrite list files function
 * dynamic array for files
 * more commands, searching
 * sort out colors
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

DIR *dir;
int sel= 0;
char del(char*);
char curdir[255];
int row, col;
void sortAbc(int);

typedef struct{
	char name[255];
	int dir;
	int size;
}fileInfo;

fileInfo file[100000];

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
				strcpy(file[j].name, ent->d_name);
				j++;
				n++;
				del(path);
			}
		}
		else{
			strcat(path, ent->d_name);
			stat(path, &type);
			if(S_ISDIR(type.st_mode)){
				file[n].dir= 1;
			}
			else{
				file[n].dir= 0;
			}
			strcpy(file[j].name, ent->d_name);
			j++;
			n++;
			del(path);
		}
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
		//mvwprintw(win, i, col,  "%d\n", file[i].size);
		wrefresh(stdscr);
		wrefresh(win);
	}
	return win;
}

void go_back(){
	int i;
	for(i= strlen(curdir); i>0; i--){
		if(curdir[i]== '/'){
			curdir[i]= '\0';
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

void getCommand(){
	char cmd[20];
	char path[255];
	strcpy(path, curdir);
	strcat(path, "/");
	strcat(path, file[sel].name);
	echo();
	mvaddch(row-1, 1, ':');
	mvgetstr(row-1, 2, cmd);
	//mvscanw(row-1, 2, "%s", cmd);
	if(strcmp(cmd, "delete")== 0){
		if(file[sel].dir== 1){
			mvprintw(row-1, 1, "%s is a directory, delete?[y/n]", file[sel].name);
			if(getch()!= 'y'){
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
		noecho();
		return;
	}
	noecho();
	return;
}

void sortAbc(int length){
	fileInfo temp;
	for(int i= 1; i<length; i++){
		for(int j= 0; j<length-i; j++){
			if(strcmp(file[j].name, file[j+1].name)>0){
				temp= file[j];
				file[j]= file[j+1];
				file[j+1]= temp;
			}
		}
	}
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
	int ch;
	int len;
	int prev= 0;
	bool hidden= false;
	int view= 0;
	scrollok(stdscr, true);
	create_bar(len);
	len= listFiles(hidden);
	while(ch!= 'q'){
		getmaxyx(stdscr, row, col);
		move(row-1, 1);
		closedir(dir);
		len= listFiles(hidden);
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
				prev= sel;
				if(file[sel].dir== 1){
					if(strlen(curdir)>2){
						strcat(curdir, "/");
					}
					strcat(curdir, file[sel].name);
					curdir[strlen(curdir)+1]='\0';
					sel= 0;
					view= 0;
				}
				//len= listFiles(hidden);
				break;
			case KEY_LEFT:
				go_back();
				//len= listFiles(hidden);
				sel= prev;
				view= 0;
				break;
			case 'S':
				endwin();
				system("source ./test.sh");
				exit(EXIT_SUCCESS);
				break;
			case ':':
				getCommand();
				clear();
				break;
			case 8:
				hidden= !hidden;
				break;
		}
		refresh();
	}
	endwin();
	return 0;
}
