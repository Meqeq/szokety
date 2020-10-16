#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <pthread.h>

#define MAX_PLAYERS 4

typedef struct Player {
    int x;
    int y;
    int color;
} Player;

typedef struct GameData {
    int playersCount;
    Player players[MAX_PLAYERS];
} GameData;

typedef struct InitGameData {
    char multicastAddress[16];
    int playerID;
    GameData gameData;
} InitGameData;
InitGameData initGameData = {0};

void* listenMulticast(void* args);

WINDOW *big_window;

void clean(void);
void init_ncurses();

int serverSocket;

int main(int argc , char *argv[])
{
    /*
    if(argc != 3) {
        perror("kek");
        return 1;
    }
*/
    connectToServer(argv[2]);

    printf(initGameData.playerID);


/*
    pthread_t t = 0;
    int result = pthread_create(&t, NULL, listenMulticast, NULL);

    if( result!= 0 ) {
        perror("Thread creation failed!\n");
        exit(EXIT_FAILURE);
    }


    int serverSocket = socket(AF_INET , SOCK_STREAM , 0);

    if(serverSocket == -1) {
        perror("Could not create socket");
        return 2;
    }

    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(strtol(argv[2], NULL, 0));

   /* if (connect(serverSocket , (struct sockaddr *)&server , sizeof(server)) < 0) {
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
	puts("Data Send\n");

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
*/
	return 0;
}

void connectToServer(char* address) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Could not create socket");
        return;
    }
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons(strtol("1235", NULL, 0));
    if (connect(serverSocket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Connection error");
        return;
    }
    printf("Connected");

    // receive init game data
    read(serverSocket, &initGameData, sizeof(InitGameData));
}


void* listenMulticast(void* args) {

}

/*
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
}*/