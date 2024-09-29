#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "socket.h"
#include "peer.h"
#include "discovery.h"

int main(void)
{
    // printf("Starting...\n");

    // int sockfd = create_socket();
    // if (sockfd < 0)
    // {
    //     fprintf(stderr, "Socket creation failed.");
    //     exit(EXIT_FAILURE);
    // }

    // listen_for_broadcasts();

    // send_broadcast();

    // listen_for_connections(sockfd);

    struct sockaddr_in peer_addr = discover_peer();

    printf("Peer found at %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    return 0;
}