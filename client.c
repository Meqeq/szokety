#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>

WINDOW *big_window;

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

void connect() {
    // podłączenie do serwera
    // w odpowiedzi 
}

void move(int dir) {
    // zapytanie do servera
}

int main(int argc , char *argv[])
{
    /*
    if(argc != 3) {
        perror("kek");
        return 1;
    }
*/
    int serverSocket = socket(AF_INET , SOCK_STREAM , 0);

    if(serverSocket == -1) {
        perror("Could not create socket");
        return 2;
    }

    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(strtol(argv[2], NULL, 0));

    if (connect(serverSocket , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect error");
        return 3;
    }

    printf("Connected");

    /*char *message , server_reply[2000];

    message = "GET / HTTP/1.1\r\n\r\n";
	if( send(serverSocket , message , strlen(message) , 0) < 0)
	{
		puts("Send failed");
		return 1;
	}
	puts("Data Send\n");*/

    init_ncurses();
    box(big_window, 0, 0);

    mvwprintw(big_window, 5, 12, "W");

    wrefresh(big_window);

    char key;
    while( (key = getch()) != 27 ) {
        switch(key) {
            case KEY_UP:

            
        }
    }

	return 0;
}