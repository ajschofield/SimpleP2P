#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "discovery.h"
#include "peer.h"
#include "colours.h"

int main()
{
    printf("------------------------\n");
    printf("Name: SimpleP2P\n");
    printf("Author: Alex Schofield\n");
    printf("------------------------\n");

    printf(BYEL "Starting peer discovery...\n" COLOR_RESET);
    struct peer_info peer = discover_peer();

    printf(BGRN "Peer found at %s:%d\n" COLOR_RESET, inet_ntoa(peer.addr.sin_addr), ntohs(peer.addr.sin_port));

    printf(CYN "Attempting to establish connection...\n" COLOR_RESET);
    int connection_fd = establish_connection(peer.addr, peer.discovery_socket);
    if (connection_fd < 0)
    {
        fprintf(stderr, BRED "Failed to establish connection with peer\n" COLOR_RESET);
        return 1;
    }

    printf(BGRN "Connection established successfully. Socket FD: %d\n" COLOR_RESET, connection_fd);
    printf("Keeping connection open for 10 seconds...\n");

    sleep(10);

    close(connection_fd);
    printf(BRED "Connection closed.\n" COLOR_RESET);

    return 0;
}