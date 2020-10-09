#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <asm/errno.h>

#define MAX_PLAYERS 8
#define MULTICAST_PORT 1234
#define SERVER_PORT 1235
const char *MULTICAST_ADDR = "239.4.2.224";

struct sockaddr_in multiAddr, serverAdr;
int currentPlayers;
int multiSock;

int serverSocket, clientSockets[MAX_PLAYERS];
int opt;
int max_sd;
struct timeval selectTimeout;

//  client descriptors
fd_set readfds;


typedef struct Player {
    int id;
    int x;
    int y;
} Player;
Player players[MAX_PLAYERS];
int playerSockets[MAX_PLAYERS];

typedef struct GameData {
    int playersCount;
    Player players[MAX_PLAYERS];
} GameData;
GameData gameData;

typedef struct InitGameData {
    char multicastAddress[16];
    int playerID;
    GameData gameData;
} InitGameData;
InitGameData initGameData;

void addPlayer(int socket) {
    players[currentPlayers].id = currentPlayers;
    players[currentPlayers].x = 5;
    players[currentPlayers].y = 5;
    playerSockets[currentPlayers] = socket;
    currentPlayers++;
}

void addConnection(int serverSocket, struct sockaddr_in *clientAddr) {
    int cl_addr_length = sizeof(struct sockaddr_in);
    int newClient = accept(serverSocket, (struct sockaddr *) clientAddr, (socklen_t *) &cl_addr_length);
    if (newClient < 0) {
        perror("Connection error");
        exit(EXIT_FAILURE);
    } else {
        if (currentPlayers < MAX_PLAYERS) {
            addPlayer(newClient);
        } else {
            printf("Too many players!");
        }
    }
}

void createMulticastSocket() {
    // create multicast socket
    multiSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (multiSock < 0) {
        printf("\n Socket creation error \n");
        return;
    }
    // set up destination address
    multiAddr.sin_family = AF_INET;
    multiAddr.sin_port = htons(MULTICAST_PORT);
    if (inet_pton(AF_INET, MULTICAST_ADDR, &multiAddr.sin_addr) <= 0)  // multiAddr.sin_addr.s_addr = inet_addr(group);
    {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }
}

void sendMulticast() {
    int nbytes = sendto(multiSock, &gameData, sizeof(GameData), 0, (struct sockaddr *) &multiAddr, sizeof(multiAddr));
    if (nbytes < 0) {
        perror("sendto");
        return;
    }
}


void initServerSocket() {
    // create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    // set server socket to allow multiple connections
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // set address options
    serverAdr.sin_family = AF_INET;
    serverAdr.sin_addr.s_addr = INADDR_ANY;
    serverAdr.sin_port = htons(SERVER_PORT);

    //bind the serverSocket to serverAdr
    if (bind(serverSocket, (struct sockaddr *) &serverAdr, sizeof(serverAdr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}


void startListening() {
    if (listen(serverSocket, 8) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}


void initClientSockets() {
    memset(clientSockets, 0, sizeof(clientSockets));
    memset(&serverAdr, '0', sizeof(serverAdr));

    selectTimeout.tv_sec = 0;
    selectTimeout.tv_usec = 700;
}


void prepareClientSockets() {
    //clear the client socket set
    FD_ZERO(&readfds);

    //add server socket to set
    FD_SET(serverSocket, &readfds);
    max_sd = serverSocket;

    //add child sockets to set
    for (int i = 0; i < MAX_PLAYERS; i++) {
        int sd = clientSockets[i];
        if (sd > 0)
            FD_SET(sd, &readfds);

        //highest file descriptor number, need it for the select function
        if (sd > max_sd)
            max_sd = sd;
    }
}

int checkData() {
    prepareClientSockets();
    int data = select(max_sd + 1, &readfds, NULL, NULL, &selectTimeout);
    if ((data < 0) && (errno != EINTR)) {
        perror("select");
    }
    return data;
}


void closeSocket(int socket) {
    close(socket);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (clientSockets[i] == socket) {
            clientSockets[i] = 0;
        }
    }
}

void sendInitData(int socket, int playerID) {
    InitGameData initGameData1;
    initGameData1.gameData = gameData;
    strcpy(initGameData1.multicastAddress, "239.4.2.224");
    initGameData1.playerID = playerID;
    send(socket, &initGameData1, sizeof(InitGameData), 0);
}

void handleNewClient() {
    int addrlen = sizeof(struct sockaddr_in);
    int newSocket = accept(serverSocket, (struct sockaddr *) &serverAdr, (socklen_t *) &addrlen);
    if (newSocket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("New client accepted!\n");

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (clientSockets[i] == 0) {
            clientSockets[i] = newSocket;
            break;
        }
    }

    // create new player
    gameData.playersCount = currentPlayers;
    gameData.players[currentPlayers].id = currentPlayers;
    gameData.players[currentPlayers].x = 5;
    gameData.players[currentPlayers].y = 5;

    // send init data
    sendInitData(newSocket, currentPlayers);
    currentPlayers++;
}

void handleClientData(int clientSocket) {
    Player receivedPlayer;
    int data = read(clientSocket, &receivedPlayer, sizeof(Player));
    if (data < 1) {
        // someone closed client
        closeSocket(clientSocket);
        printf("Someone left the game\n");
        return;
    }

    // handle client key press
    gameData.players[receivedPlayer.id].x = gameData.players[receivedPlayer.id].x + receivedPlayer.x;
    gameData.players[receivedPlayer.id].y = gameData.players[receivedPlayer.id].y + receivedPlayer.y;
    printf("Player %d cords: (%d,%d)\n", receivedPlayer.id, gameData.players[receivedPlayer.id].x,
           gameData.players[receivedPlayer.id].y);
    sendMulticast();
}

void handleData() {
    if (FD_ISSET(serverSocket, &readfds)) {
        // incomming connection
        handleNewClient();
    } else {
        // some player sent data
        for (int i = 0; i < MAX_PLAYERS; i++) {
            int sd = clientSockets[i];
            if (FD_ISSET(sd, &readfds)) {
                handleClientData(sd);
            }
        }
    }
}

int main(int argc, char *argv[]) {
//    if (argc != 3) {
//        perror("kek");
//        return 1;
//    }

    initServerSocket();
    startListening();

    initClientSockets();

    createMulticastSocket();

    while (1) {
        int data = checkData();
        if (data > 0)
            handleData();   // send multicast or accept connection
    }
    return 0;
}