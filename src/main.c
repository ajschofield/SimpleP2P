#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "discovery.h"
#include "peer.h"

int main()
{
    printf("------------------------\n");
    printf("Name: SimpleP2P\n");
    printf("Author: Alex Schofield\n");
    printf("------------------------\n");
    printf("Starting peer discovery...\n");
    struct peer_info peer = discover_peer();

    printf("Peer found at %s:%d\n", inet_ntoa(peer.addr.sin_addr), ntohs(peer.addr.sin_port));

    printf("Attempting to establish connection...\n");
    int connection_fd = establish_connection(peer.addr, peer.discovery_socket);
    if (connection_fd < 0)
    {
        fprintf(stderr, "Failed to establish connection with peer\n");
        return 1;
    }

    printf("Connection established successfully. Socket FD: %d\n", connection_fd);
    printf("Keeping connection open for 10 seconds...\n");

    sleep(10);

    close(connection_fd);
    printf("Connection closed.\n");

    return 0;
}