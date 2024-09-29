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
#define MAX_BUFFER 1024
#define LISTEN_TIMEOUT 5
#define BROADCAST_INTERVAL 1

static char my_id[37];

void generate_unique_id()
{
    snprintf(my_id, sizeof(my_id), "%08x-%04x-%04x-%04x-%012x",
             rand(), rand(), rand(), rand(), rand());
}

int create_broadcast_socket()
{
    int sockfd;
    struct sockaddr_in addr;
    int broadcast = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
    {
        perror("Failed to set SO_BROADCAST");
        close(sockfd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BROADCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void send_broadcast(int sockfd)
{
    struct sockaddr_in addr;
    char message[MAX_BUFFER];

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BROADCAST_PORT);
    addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    snprintf(message, sizeof(message), "DISCOVER:%s", my_id);

    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Broadcast failed");
    }
}

int listen_for_broadcast(int sockfd, struct sockaddr_in *peer_addr)
{
    char buffer[MAX_BUFFER];
    socklen_t addr_len = sizeof(struct sockaddr_in);
    ssize_t recv_len;

    recv_len = recvfrom(sockfd, buffer, MAX_BUFFER - 1, 0, (struct sockaddr *)peer_addr, &addr_len);

    if (recv_len > 0)
    {
        buffer[recv_len] = '\0';
        char received_id[37];
        if (sscanf(buffer, "DISCOVER:%36s", received_id) == 1)
        {
            if (strcmp(received_id, my_id) != 0)
            {
                printf("Discovered peer: %s\n", inet_ntoa(peer_addr->sin_addr));
                return 1;
            }
        }
    }

    return 0;
}

struct sockaddr_in discover_peer()
{
    int sockfd = create_broadcast_socket();
    struct sockaddr_in peer_addr;
    struct timeval tv;

    if (sockfd < 0)
    {
        exit(EXIT_FAILURE);
    }

    generate_unique_id();
    printf("My ID: %s\n", my_id);

    while (1)
    {
        send_broadcast(sockfd);
        printf("Broadcast sent, listening for responses...\n");

        tv.tv_sec = LISTEN_TIMEOUT;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (listen_for_broadcast(sockfd, &peer_addr))
        {
            close(sockfd);
            return peer_addr;
        }

        sleep(BROADCAST_INTERVAL);
    }

    close(sockfd);
    exit(EXIT_FAILURE);
}