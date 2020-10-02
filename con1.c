 #include <stdio.h> 
 #include <stdlib.h> 
 #include <ncurses.h>
 #include <errno.h>
 #include <sys/types.h>
 #include <unistd.h> 
 #include <signal.h>
 #include <string.h>

 WINDOW *big_window;

void zamien(int dioda, int stan, int key);


void clean(void) {
    endwin();
    system("clear");
}


void init_ncurses() {
    initscr();
    clear();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_GREEN, COLOR_BLUE);
    init_pair(4, COLOR_CYAN, COLOR_BLUE);
    cbreak();
    keypad(stdscr,TRUE);
    noecho();
    refresh();
        
    curs_set(0);
    big_window = newwin(25, 80, 0, 0);      

    wattron(big_window, COLOR_PAIR(1));

    wbkgdset(big_window, COLOR_PAIR(2));

    wclear(big_window);
    wrefresh(big_window);

    on_exit(clean, NULL);
}

void main( /* argc, argv[] */ )
  {

   init_ncurses();

   box(big_window, 0, 0);

   wattron(big_window, COLOR_PAIR(4));
   wattron(big_window, A_BOLD);

   mvwprintw(big_window, 5, 12, "napis");
   wattroff(big_window, A_BOLD);


   wattroff(big_window, COLOR_PAIR(2));
   wattron(big_window, COLOR_PAIR(1));
   //mvwprintw(big_window, 12, 5, "Dioda 1");

   wrefresh(big_window);
   wattroff(big_window, COLOR_PAIR(1));
   wattron(big_window, COLOR_PAIR(2));
   mvwprintw(big_window, 12, 5, "Dioda 1");
   wrefresh(big_window);
   int t = 1;
   // 72, 75, 77, 80
   while( (key = getch()) != 27 ) // ESC
    {
	switch(key) {
	 case 10:
		if(stan[t]==2) {
                	wattroff(big_window, COLOR_PAIR(2));
                	wattron(big_window, COLOR_PAIR(1));
                	mvwprintw(big_window, 11+t, 5, "Dioda %d", t);
			stan[t]=1;
		}
		else if(stan[t]==1) {
                        wattroff(big_window, COLOR_PAIR(1));
                        wattron(big_window, COLOR_PAIR(2));
                        mvwprintw(big_window, 11+t, 5, "Dioda %d", t);
                        stan[t]=2;
		}
		break;
	case KEY_UP:
		if(t==1) break;
		if(stan[t-1] == 2){
		 t--;
                 wattroff(big_window, COLOR_PAIR(2));
                 wattron(big_window, COLOR_PAIR(1));
                 mvwprintw(big_window, 11+t, 5, "Dioda %d", t+1);
		 break;
		}
		t--;
		wattroff(big_window, COLOR_PAIR(1));
		wattron(big_window, COLOR_PAIR(2));
		mvwprintw(big_window, 11+t, 5, "Dioda %d", t);
		stan[t]=1;
		if(stan[t+1] == 2) break;
		wattroff(big_window, COLOR_PAIR(2));
                wattron(big_window, COLOR_PAIR(1));
		mvwprintw(big_window, 12+t, 5, "Dioda %d", t+1);
		stan[t+1]=0;
		break;
	case KEY_DOWN:
                if(t==8) break;
                if(stan[t+1] == 2)
		{
		 t++;
                 wattroff(big_window, COLOR_PAIR(2));
                 wattron(big_window, COLOR_PAIR(1));
                 mvwprintw(big_window, 10+t, 5, "Dioda %d", t-1);
                 break;
		}
		t++;
                wattroff(big_window, COLOR_PAIR(1));
                wattron(big_window, COLOR_PAIR(2));
                mvwprintw(big_window, 11+t, 5, "Dioda %d", t);
                stan[t]=1;
                if(stan[t-1] == 2) break;
                wattroff(big_window, COLOR_PAIR(2));
                wattron(big_window, COLOR_PAIR(1));
                mvwprintw(big_window, 10+t, 5, "Dioda %d", t-1);
                stan[t-1]=0;
		break;
	default:
		mvwprintw(big_window, 20, 5, "asdfasfddas %d", 3);
	}
    wrefresh(big_window);
    }


   endwin();
   system("clear");
  }



void zamien(int dioda, int stan, int key)
{

}

