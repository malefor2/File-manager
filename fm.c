#include<stdio.h>
#include<ncurses.h>
#include<dirent.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>	//implement cmd
#include<sys/stat.h>
#include<stdlib.h>
#include"cmds.h"

int sel= 0;
int indexx[10000];
char del(char*);
char *selection[10000];
char curdir[255];
int row, col;

int listFiles(bool show_hidden){
	DIR *dir;
	int n= 0;
	struct stat type;
	int j= 0;
	char path[255];
	strcpy(path, curdir);
	strcat(path, "/");
	dir= opendir(curdir);
	struct dirent *ent;
	while((ent= readdir(dir))!= NULL){
		if(strcmp(ent->d_name, "..")== 0 || strcmp(ent->d_name, ".")== 0){
			continue;
		}
		if(!show_hidden){
			if(ent->d_name[0]!= '.'){
				strcat(path, ent->d_name);
				stat(path, &type);
				if(S_ISDIR(type.st_mode)){
					indexx[n]= 1;
				}
				else{
					indexx[n]= 0;
				}
				selection[j]= ent->d_name;
				j++;
				n++;
				del(path);
			}
		}
		else{
			strcat(path, ent->d_name);
			stat(path, &type);
			if(S_ISDIR(type.st_mode)){
				indexx[n]= 1;
			}
			else{
				indexx[n]= 0;
			}
			selection[j]= ent->d_name;
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
	wprintw(loc_bar, "%s/%s", curdir, selection[sel]);
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
			wprintw(win, "%s\n", selection[i]);
			wattroff(win, A_REVERSE);
		}
		else{
			if(indexx[i]== 1){
				wattron(win, COLOR_PAIR(1));
				wattron(win, A_BOLD);
				wprintw(win, "%s\n", selection[i]);
				wattroff(win, A_BOLD);
				wattroff(win, COLOR_PAIR(1));
			}
			else{
				wprintw(win, "%s\n", selection[i]);
			}
		}
		wrefresh(stdscr);
		wrefresh(win);
	}
	return win;
}

void go_back(){
	int i= strlen(curdir);
	for(i; i>0; i--){
		if(curdir[i]== '/'){
			curdir[i]= '\0';
			break;
		}
		else{
			curdir[i]= ' ';
		}
	}
	curdir[0]= '/';
	if(i<2){
		curdir[1]= '\0';
	}
	return;
}

char del(char *path){
	int i= strlen(path);
	for(i; i>0; i--){
		if(path[i]== '/'){
			path[i+1]= '\0';
			break;
		}
	}
	return *path;
}

void getCommand(){
	char cmd[80];
	echo();
	mvaddch(row-1, 1, ':');
	mvgetnstr(row-1, 2, cmd, sizeof(cmd));
	noecho();
	return;
}

void sortAbc(int length){

}

int main(int argc, char *argv[]){
	if(argc<2){
		getcwd(curdir, sizeof(curdir));
	}
	else{
		strcpy(curdir, argv[1]);
	}
	initscr();
	start_color();
	use_default_colors();
	init_color(COLOR_RED, 28, 28, 28);
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_RED);
	curs_set(0);
	keypad(stdscr, true);
	getmaxyx(stdscr, row, col);
	noecho();
	int ch;
	int len;
	bool hidden= false;
	int view= 0;
	scrollok(stdscr, true);
	create_bar(len);
	while(ch!= 'q'){
		getmaxyx(stdscr, row, col);
		move(row-1, 1);
		len= listFiles(hidden);
		if(len<1){
			clear();
			selection[0]= "empty";
		}
		list_window(len, view);
		create_bar(len);
		switch(ch= getch()){
			case KEY_UP:
				if(sel== 0){
					break;
				}
				sel--;
				if(len>row){
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
				if(len>row){
					if(sel>view+row-5){
						view++;
					}
				}
				break;
			case 10: case KEY_RIGHT:
				if(indexx[sel]== 1){
					strcat(curdir, "/");
					strcat(curdir, selection[sel]);
					curdir[strlen(curdir)+1]='\0';
					sel= 0;
				}
				view= 0;
				break;
			case KEY_LEFT:
				go_back();
				sel= 0;
				view= 0;
				break;
			case 'S':
				endwin();
				system("source ./test.sh");
				exit(EXIT_SUCCESS);
				break;
			case ':':
				getCommand();
				break;
			case 'H':
				hidden= !hidden;
				break;
			case 'E':
				deleteFile();
				break;
		}		
		refresh();
	}
	endwin();
	return 0;
}
