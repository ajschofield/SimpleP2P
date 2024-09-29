#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket.h"

// Set a max size for the messages so that we don't overflow the buffer.
#define MAX_BUFFER 1024

void listen_for_connections(int sockfd)
{
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    char buffer[MAX_BUFFER] = {0};

    // Listen for any incoming connections.
    if (listen(sockfd, 3) != 0)
    {
        perror("There was an error whilst listening for connections.");
        exit(EXIT_FAILURE);
    }

    printf("Listening for incoming connections...\n");

    int incoming = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);

    if (incoming < 0)
    {
        perror("There was an error whilst accepting the connection.");
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted from %s:%d\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
}