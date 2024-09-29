#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket.h"
#include "peer.h"
#include "discovery.h"

int main(void)
{
    printf("Starting...\n");

    int sockfd = create_socket();
    if (sockfd < 0)
    {
        fprintf(stderr, "Socket creation failed.");
        exit(EXIT_FAILURE);
    }

    listen_for_broadcasts();

    send_broadcast();

    listen_for_connections(sockfd);

    return 0;
}