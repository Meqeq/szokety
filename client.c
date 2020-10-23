#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <pthread.h>

#include "types.h"

WINDOW *big_window;
void clean(void);
void init_ncurses();

int connectToServer(struct sockaddr_in* server, char* address, char* port);
int createMulticast(struct sockaddr_in* multi, struct ip_mreq* group);
void printPlayers(InitGameData* igd);

void* gameLoop(void* args);

int main(int argc, char* argv[]) {

    if(argc != 3) {
        printf("Usage: %s <address> <port>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in server = { 0 };
    int serverSocket = connectToServer(&server, argv[1], argv[2]);
    if(serverSocket < 0) {
        perror("KEKEE\n");
    }

    struct PlayerAction pa = { 1, 3, 6 };
    int result = sendto(serverSocket, &pa, sizeof(pa), 0, (struct sockaddr *) &server, sizeof(struct sockaddr));
    if(result < 0) {
        perror("SEND KEK\n");
    }

    struct InitGameData igd = {0};

    result = recvfrom(serverSocket, &igd, sizeof(InitGameData), 0, NULL, 0);

    printf("dwdwdw: %s %d\n", igd.address, igd.id);

    init_ncurses();

    pthread_t t = 0;
    result = pthread_create(&t, NULL, gameLoop, (void *)&igd);

    int key;
    while( (key = getch()) != 27 ) {
        int move = 0;
        switch(key) {
            case 97:
            case 260: // move left
                move = 1;
                break;
            case 100:
            case 261: // move right
                move = 2;
                break;
            case 119:
            case 259: // move up
                move = 3;
                break; 
            case 115:
            case 258: // move down
                move = 4;
                break;
        }

        if(move) {
            PlayerAction pa = { 0 };
            pa.type = 2;
            pa.move = move;
            pa.id = igd.id;
            result = sendto(serverSocket, &pa, sizeof(pa), 0, (struct sockaddr *) &server, sizeof(struct sockaddr));
            if(result < 0) {
                perror("Move send\n");
            }
        }
    }
/*


    struct PlayerAction pa = { strtol(argv[3], NULL, 0), 3, 6 };
    int result = sendto(serverSocket, &pa, sizeof(pa), 0, (struct sockaddr *) &server, sizeof(struct sockaddr));
    if(result < 0) {
        perror("Kuźwa send\n");
    }*/
}

void* gameLoop(void* args) {
    InitGameData* igd = (InitGameData*) args;

    box(big_window, 0, 0);

    struct sockaddr_in localSock;
    struct ip_mreq group;

    printPlayers(igd);

    /* Read from the socket. */

    int sd = createMulticast(&localSock, &group);

    while(1) {
        int datalen = sizeof(GameData);
        if(read(sd, &igd->gd, datalen) < 0) {
            perror("Reading datagram message error");
            close(sd);
            exit(1);
        } else {
        }

        printPlayers(igd);
    }
}

void printPlayers(InitGameData* igd) {
    wclear(big_window);

    for(int i = 0; i < igd->gd.count; i++) {
        wattron(big_window, COLOR_PAIR(i+1));

        mvwprintw(big_window, igd->gd.players[i].x, igd->gd.players[i].y, "@");

        wattroff(big_window, COLOR_PAIR(i+1));
    }

    wrefresh(big_window);
}

int createMulticast(struct sockaddr_in* localSock, struct ip_mreq* group) {
    int sd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sd < 0) {
        perror("Opening datagram socket error");
        exit(1);
    }

    {
        int reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
            perror("Setting SO_REUSEADDR error");
            close(sd);
            exit(1);
        }
    }

    memset((char *) localSock, 0, sizeof(struct sockaddr_in));

    localSock->sin_family = AF_INET;
    localSock->sin_port = htons(4321);
    localSock->sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr*)localSock, sizeof(struct sockaddr_in))) {
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
    }

    group->imr_multiaddr.s_addr = inet_addr("226.1.1.1");
    group->imr_interface.s_addr = inet_addr("172.31.166.181");

    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)group, sizeof(struct ip_mreq)) < 0) {
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    }

    return sd;
}

int connectToServer(struct sockaddr_in* server, char* address, char* port) {
    int serverSocket = socket(PF_INET, SOCK_DGRAM, 0);

    if(serverSocket < 0) {
        return -1;
    }

    server->sin_family      = AF_INET;
    server->sin_addr.s_addr = inet_addr(address);   
    server->sin_port        = htons(strtol(port, NULL, 0));

    int result = connect(serverSocket, (struct sockaddr *) server, sizeof(struct sockaddr));   
    if(result < 0) {
        perror("Connect failed\n");
    }

    return serverSocket;
}


void clean(void) {
    endwin();
    system("clear");
}

void init_ncurses() {
    initscr();
    clear();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_GREEN, COLOR_WHITE);
    init_pair(4, COLOR_CYAN, COLOR_WHITE);
    init_pair(5, COLOR_RED, COLOR_WHITE);
    init_pair(6, COLOR_MAGENTA, COLOR_WHITE);
    init_pair(7, COLOR_YELLOW, COLOR_WHITE);
    cbreak();
    keypad(stdscr,TRUE);
    noecho();
    timeout(300);

    refresh();
        
    curs_set(0);
    big_window = newwin(25, 80, 0, 0);      

    wattron(big_window, COLOR_PAIR(1));

    wbkgdset(big_window, COLOR_PAIR(2));

    wclear(big_window);
    wrefresh(big_window);

    on_exit(clean, NULL);
}