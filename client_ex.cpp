#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
using namespace std;

char message[] = "Hello there!\n";
char buf[sizeof(message)];
char mes[80];

int main()
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425); // или любой другой порт...
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }

    
    while (1)
    {
        cin >> mes;
        send(sock, mes, strlen(mes) + 1, 0);
        //bzero(buf, sizeof(message));
        //recv(sock, buf, strlen(mes) + 1, 0);
        //printf(buf);
        //printf("\n");
    }
    close(sock);

    return 0;
}