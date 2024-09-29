#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "discovery.h"

#define BROADCAST_PORT 5000
#define BROADCAST_IP "255.255.255.255"

void listen_for_broadcasts(void)
{
    int sockfd;
    struct sockaddr_in addr;
    char buffer[1024];
    ssize_t recv_len;
    socklen_t addrlen = sizeof(addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("There was an issue creating the UDP socket for broadcasts");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Failed to set SO_REUSEADDR");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
    {
        perror("Failed to set SO_BROADCAST");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(BROADCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("There was an issue binding to the UDP socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening for broadcasts on port %d...\n", BROADCAST_PORT);

    struct timeval tv;
    tv.tv_sec = 5; //
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("There was an error setting the socket timeout");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    recv_len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&addr, &addrlen);
    if (recv_len > 0)
    {
        buffer[recv_len] = '\0';
        printf("Received broadcast from %s: %s\n", inet_ntoa(addr.sin_addr), buffer);
    }
    else if (recv_len == -1)
    {
        perror("recvfrom failed, try again");
    }
    else
    {
        printf("No broadcasts were received within the timeout period.\n");
    }

    close(sockfd);
}

void send_broadcast(void)
{
    int sockfd;
    struct sockaddr_in addr;
    char *message = "Peer available";
    int opt = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("There was an issue creating the UDP socket for broadcasts");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
    {
        perror("Failed to set SO_BROADCAST");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(BROADCAST_PORT);
    addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("There was an issue sending the broadcast message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Broadcast message sent.\n");

    close(sockfd);
}
