#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include "socket.h"

#define MIN_PORT 1024
#define MAX_PORT 65535
#define MAX_ATTEMPTS 10

int negotiate_port(int socket_fd)
{
    srand(time(NULL));
    char buffer[256];
    int negotiated_port;

    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++)
    {
        negotiated_port = (rand() % (MAX_PORT - MIN_PORT + 1)) + MIN_PORT;
        // Send
        snprintf(buffer, sizeof(buffer), "PORT_PROPOSAL:%d", negotiated_port);
        if (send(socket_fd, buffer, strlen(buffer), 0) < 0)
        {
            perror("Failed to send port proposal");
            return -1;
        }

        // Receive
        memset(buffer, 0, sizeof(buffer));
        if (recv(socket_fd, buffer, sizeof(buffer) - 1, 0) < 0)
        {
            perror("Failed to receive port confirmation");
            return -1;
        }

        if (strncmp(buffer, "PORT_ACCEPT", 11) == 0)
        {
            printf("Port %d accepted\n", negotiated_port);
            return negotiated_port;
        }
    }

    printf("Failed to negotiatae a port after %d attempts\n", MAX_ATTEMPTS);
    return -1;
}

int establish_connection(struct sockaddr_in peer_addr)
{
    int socket_fd;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create the TCP socket required for communication");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0)
    {
        perror("Failed to connect to the peer");
        close(socket_fd);
        return -1;
    }

    printf("Connection with peer established. Negotiating port...\n");

    int negotiated_port = negotiate_port(socket_fd);
    if (negotiated_port < 0)
    {
        close(socket_fd);
        return -1;
    }

    close(socket_fd);

    peer_addr.sin_port = htons(negotiated_port);
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket for the negotiated port");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0)
    {
        perror("Failed to connect to the peer on the negotiated port");
        close(socket_fd);
        return -1;
    }

    printf("Connection established on port %d\n", negotiated_port);
    return socket_fd;
}