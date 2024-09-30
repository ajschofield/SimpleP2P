#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "discovery.h"
#include "peer.h"

int main()
{
    struct peer_info peer = discover_peer();

    printf("Peer found at %s:%d\n", inet_ntoa(peer.addr.sin_addr), ntohs(peer.addr.sin_port));

    int connection_fd = establish_connection(peer.addr, peer.discovery_socket);
    if (connection_fd < 0)
    {
        fprintf(stderr, "Failed to establish connection with peer\n");
        return 1;
    }

    printf("Connection established. Ready for communication.\n");

    sleep(10);

    close(connection_fd);
    printf("Connection closed.\n");

    return 0;
}