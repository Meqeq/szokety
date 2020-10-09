#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <time.h>

#define SERVER_PORT 1235
#define MULTICAST_PORT 1234
#define MAX_PLAYERS 4

typedef struct Player {
    int x;
    int y;
    int color;
} Player;
Player players[MAX_PLAYERS];

int playerSockets[MAX_PLAYERS], max_sd;

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

const char *MULTICAST_ADDR = "239.4.2.224";

struct sockaddr_in server, client, multiAddr;

fd_set readfds;

struct timeval selectTimeout = { 0, 30000 };

int initSocket(char* address);
int initMultiSocket();
void sendGameData(int multiSocket);
void sendInitData(int clientSocket);
void createPlayer();
void acceptPlayer(int serverSocket);
void prepareClientSockets();
void handleClientData(int clientSocket, int player);

int main(int argc , char *argv[])
{
    if(argc != 3) {
        perror("kek");
        return 1;
    }

    int serverSocket = initSocket(argv[2]);
    int multiSocket = initMultiSocket();


    listen(serverSocket, 1000);

    while(1) {

        prepareClientSockets(serverSocket);
        int data = select(max_sd + 1, &readfds, NULL, NULL, &selectTimeout);
        
        if (FD_ISSET(serverSocket, &readfds)) {
            acceptPlayer(serverSocket);
        } else {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                int sd = playerSockets[i];
                if (FD_ISSET(sd, &readfds)) {
                    handleClientData(sd, i);
                }
            }
        }

        sendGameData(multiSocket);
    }

	return 0;
}

void prepareClientSockets(int serverSocket) {
    //clear the client socket set
    FD_ZERO(&readfds);

    //add server socket to set
    FD_SET(serverSocket, &readfds);

    max_sd = serverSocket;
    //add child sockets to set
    for (int i = 0; i < MAX_PLAYERS; i++) {
        int sd = playerSockets[i];
        if (sd > 0)
            FD_SET(sd, &readfds);

        if (sd > max_sd)
            max_sd = sd;
    }
}

void handleClientData(int clientSocket, int player) {
    Player receivedPlayer;
    int data = read(clientSocket, &receivedPlayer, sizeof(Player));
    if (data < 1) {
        // someone closed client
        //closeSocket(clientSocket); .///////////////////////////////////////////////////
        printf("Someone left the game\n");
        return;
    }

    // handle client key press
    initGameData.gameData.players[player].x += receivedPlayer.x;
    initGameData.gameData.players[player].y += receivedPlayer.y;
    printf("Player %d cords: (%d,%d)\n", player, initGameData.gameData.players[player].x,
           initGameData.gameData.players[player].y);
}

void acceptPlayer(int serverSocket) {
    int c = sizeof(struct sockaddr_in);

    playerSockets[initGameData.gameData.playersCount] = accept(serverSocket, (struct sockaddr *)&client, (socklen_t*)&c);

    if(serverSocket == -1) {
        perror("Could not create socket");
    }

    printf("Accept player\n");

    createPlayer();
    sendInitData(playerSockets[initGameData.gameData.playersCount-1]);
}

int initSocket(char* address) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1) {
        return -1;
    }

    server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(strtol(address, NULL, 0));

    if(bind(serverSocket,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Bind failed");
        return -1;
    }

    return serverSocket;
}

int initMultiSocket() {
    int multiSock = socket(AF_INET, SOCK_DGRAM, 0);

    if (multiSock < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // set up destination address
    multiAddr.sin_family = AF_INET;
    multiAddr.sin_port = htons(MULTICAST_PORT);
    if (inet_pton(AF_INET, MULTICAST_ADDR, &multiAddr.sin_addr) <= 0)  // multiAddr.sin_addr.s_addr = inet_addr(group);
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    bind(multiSock, (struct sockaddr *) &multiAddr, sizeof(struct sockaddr));   

    return multiSock;
}

void sendGameData(int multiSocket) {
    int nbytes = sendto(multiSocket, &initGameData.gameData, sizeof(GameData), 0, (struct sockaddr *) &multiAddr, sizeof(multiAddr));

    if (nbytes < 0) {
        perror("sendto");
        return;
    }
}

void sendInitData(int clientSocket) {

    int nbytes = sendto(clientSocket, &initGameData, sizeof(InitGameData), 0, (struct sockaddr *) &multiAddr, sizeof(multiAddr));

    if (nbytes < 0) {
        perror("sendto");
        return;
    }
}

void createPlayer() {
    initGameData.gameData.players[initGameData.gameData.playersCount].x = 10;
    initGameData.gameData.players[initGameData.gameData.playersCount].y = 10;
    initGameData.gameData.players[initGameData.gameData.playersCount].color = initGameData.gameData.playersCount+1;

    initGameData.gameData.playersCount++;
}