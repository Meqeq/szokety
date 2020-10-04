#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <zconf.h>

/*
 * Mamy  8 kolorow, czyli na razie max 8 graczy
 * */


WINDOW *big_window;
int serverSocket;
int playerId;

int multiSocket;
struct sockaddr_in multiAddr;

fd_set readfds;
struct timeval selectTimeout;

typedef struct Player {
    int id;
    int x;
    int y;
} Player;

typedef struct GameData {
    int playersCount;
    Player players[8];
} GameData;
GameData gameData;

typedef struct InitGameData {
    char multicastAddress[16];
    int playerID;
    GameData gameData;
} InitGameData;
InitGameData initGameData;

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

    // Player colors
    init_pair(5, COLOR_BLACK, COLOR_BLUE);
    init_pair(6, COLOR_RED, COLOR_BLUE);
    init_pair(7, COLOR_GREEN, COLOR_BLUE);
    init_pair(8, COLOR_YELLOW, COLOR_BLUE);
    init_pair(9, COLOR_WHITE, COLOR_BLUE);
    init_pair(10, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(11, COLOR_CYAN, COLOR_BLUE);

    cbreak();
    keypad(stdscr, TRUE);
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


void movePlayer(int x, int y) {
    // zapytanie do servera, PRZESYLANE SA TYLKO ZMIANY A NIE AKTULANA POZYCJA
    Player *player = (Player *) calloc(1, sizeof(Player)); // inicjalizowac w main
    player->id = playerId;
    player->x = x;
    player->y = y;
    if (send(serverSocket, player, sizeof(Player), 0) < 0) {
        perror("Send failed");
        return;   // TODO: wysylac w petli while
    }
    free(player);
}

void drawMap() {
    init_ncurses();
    box(big_window, 0, 0);
    wrefresh(big_window);
}


void updateMap() {
    drawMap();
    for (int i = 0; i <= gameData.playersCount; i++) {
        //draw player
        attron(COLOR_PAIR(gameData.players[i].id));
        mvwprintw(big_window, gameData.players[i].x, gameData.players[i].y, "W");
        //printf("P%d (%d,%d);", gameData.players[i].id, gameData.players[i].x, gameData.players[i].y);
        attroff(COLOR_PAIR(gameData.players[i].id));
    }
    wrefresh(big_window);
    //printf("(%d) Map has been updated, current player data: (%d, %d)\n", gameData.playersCount, gameData.players[initGameData.playerID].x, gameData.players[initGameData.playerID].y);
}


void createMulticast() {
    // create socket
    multiSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (multiSocket < 0) {
        perror("socket");
        return;
    }

    // set up destination address
    memset(&multiAddr, 0, sizeof(multiAddr));
    multiAddr.sin_family = AF_INET;
    multiAddr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    multiAddr.sin_port = htons(1234);

    // bind to receive address
    if (bind(multiSocket, (struct sockaddr *) &multiAddr, sizeof(multiAddr)) < 0) {
        perror("bind");
        return;
    }

    // use setsockopt() to request that the kernel join a multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(initGameData.multicastAddress); //inet_addr("239.0.0.224");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(multiSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        return;
    }


    // create select and add to select
    FD_ZERO(&readfds);
    FD_SET(multiSocket, &readfds);
    selectTimeout.tv_sec = 0;
    selectTimeout.tv_usec = 5000;


}

void receiveMulticast() {
    FD_ZERO(&readfds);
    FD_SET(multiSocket, &readfds);
    int data = select(multiSocket + 1, &readfds, NULL, NULL, &selectTimeout);
    if ((data == -1)) {
        perror("select");
    } else if (data) {

        read(multiSocket, &gameData, sizeof(GameData));
        printf("Received multicast with new game data!!\n");
        updateMap();
    }
}


void connectToServer() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Could not create socket");
        return;
    }
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(strtol("1235", NULL, 0));
    if (connect(serverSocket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Connection error");
        return;
    }
    printf("Connected");

    // receive init game data
    read(serverSocket, &initGameData, sizeof(InitGameData));
    gameData = initGameData.gameData;
    playerId = initGameData.playerID;
}


int main(int argc, char *argv[]) {
    /*
     * TODO: wstawic argumenty
    if(argc != 3) {
        perror("kek");
        return 1;
    } */

    // Laczenie z serwerem (TODO: przeerobic na funkcje)
    connectToServer();
    createMulticast();

    // Utworzenie interfejsu garficznego
    drawMap();

    // Glowna petla gry
    char key;
    while ((key = getc(stdin)) != 27) { // TODO CZEMU NIE DZIALA getc() ??
        switch (key) {
            case 'd':
                movePlayer(1, 0);
                break;
            case 'a':
                movePlayer(-1, 0);
                break;
            case 'w':
                movePlayer(0, 1);
                break;
            case 's':
                movePlayer(0, -1);
                break;

        }
        receiveMulticast();
    }

    printf("\nCya next time!\n\n");
    close(serverSocket);
    return 0;
}