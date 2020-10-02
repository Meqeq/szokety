#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr

struct {
    int x;
    int y;
    int color;
    
}

int main(int argc , char *argv[])
{
    if(argc != 3) {
        perror("kek");
        return 1;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(serverSocket == -1) {
        perror("Could not create socket");
        return 2;
    }

    struct sockaddr_in server , client;

    server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(strtol(argv[2], NULL, 0));

    if( bind(serverSocket,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Bind failed");
        return 3;
    }

    listen(serverSocket, 1000);
    int c = sizeof(struct sockaddr_in);
    int newSocket = accept(serverSocket, (struct sockaddr *)&client, (socklen_t*)&c);
    if(serverSocket == -1) {
        perror("Could not create socket");
        return 2;
    }

    char reply[2000];
    if( recv(newSocket, reply , 2000 , 0) < 0) {
		puts("recv failed");
	}

	puts("Reply received\n");
	puts(reply);

	return 0;
}