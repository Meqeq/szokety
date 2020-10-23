#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "types.h"

int createSocket(struct sockaddr_in* server, char* port);
int createMultiSocket(struct sockaddr_in* multi);
void acceptPlayer(int serverSocket, struct sockaddr_in* client, struct InitGameData* init);
void createPlayer(struct sockaddr_in* client, struct InitGameData* init); 
void sendInitialData(int serverSocket, struct sockaddr_in* client, struct InitGameData* init);
void handleMove(struct PlayerAction* pa, struct InitGameData* init);

int main(int argc, char* argv[]) {

    if(argc != 2) {
        printf("Usage %s <port>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in server = { 0 };
    int serverSocket = createSocket(&server, argv[1]);

    struct sockaddr_in multi = { 0 };
    int multiSocket = createMultiSocket(&multi);

    struct InitGameData init = { MULTICAST, 0 };

    while(1) {
        struct sockaddr_in client = { 0 };
        struct PlayerAction pa = { 0 };

        int size = sizeof client;
        recvfrom(serverSocket, &pa, sizeof(pa), 0, (struct sockaddr*) &client, (socklen_t*) &size);

        switch(pa.type) {
            case 1:
                printf("New player connected\n");
                //printf("%s\n", inet_ntoa(client.sin_addr));
                acceptPlayer(serverSocket, &client, &init);
                break;
            case 2:
                printf("Player moved\n");
                handleMove(&pa, &init);
                break;
        }

        int eke = sendto(multiSocket, &init.gd, sizeof(GameData), 0, (struct sockaddr*) &multi, sizeof(multi));
        if(eke < 0) {
            perror("SEND MULTICAST\n");
        }
    }

    return 0;
}

int createSocket(struct sockaddr_in* server, char* port) {
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(serverSocket == -1) {
        return -1;
    }

    server->sin_family = AF_INET;
	server->sin_addr.s_addr = htonl(INADDR_ANY);
	server->sin_port = htons(strtol(port, NULL, 0));

    if(bind(serverSocket, (struct sockaddr *) server, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind failed");
        return -1;
    }

    return serverSocket;
}

int createMultiSocket(struct sockaddr_in* multi) {
    int multiSock = socket(AF_INET, SOCK_DGRAM, 0);

    if (multiSock < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    multi->sin_family = AF_INET;
    multi->sin_addr.s_addr = inet_addr(MULTICAST);
    multi->sin_port = htons(MULTICAST_PORT);

    //bind(multiSock, (struct sockaddr *) &multi, sizeof(struct sockaddr));   
    struct in_addr localInterface;
    localInterface.s_addr = inet_addr("11.11.11.10");

    if (setsockopt(multiSock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0) {
        perror("setting local interface");
    }

    return multiSock;
}

void acceptPlayer(int serverSocket, struct sockaddr_in* client, struct InitGameData* init) {
    createPlayer(client, init);
    sendInitialData(serverSocket, client, init);
}
void createPlayer(struct sockaddr_in* client, struct InitGameData* init) {
    init->gd.players[init->gd.count].color = init->gd.count + 1;
    init->gd.players[init->gd.count].x = 10;
    init->gd.players[init->gd.count].y = 10;
    init->id = init->gd.count;
    init->gd.count++;
}

void sendInitialData(int serverSocket, struct sockaddr_in* client, struct InitGameData* init) {
    int result = sendto(serverSocket, init, sizeof(struct InitGameData), 0, (struct sockaddr *) client, (socklen_t) sizeof(struct sockaddr));
    if(result < 0) {
        perror("Send init game data\n");
    }
}

void handleMove(struct PlayerAction* pa, struct InitGameData* init) {
    switch(pa->move) {
        case 3:
            init->gd.players[pa->id].x--;
            if(init->gd.players[pa->id].x < 0) init->gd.players[pa->id].x = 0;
            break;
        case 4:
            init->gd.players[pa->id].x++;
            break;
        case 1:
            init->gd.players[pa->id].y--;
            if(init->gd.players[pa->id].y < 0) init->gd.players[pa->id].y = 0;
            break;
        case 2:
            init->gd.players[pa->id].y++;
            break;
    }    
}
