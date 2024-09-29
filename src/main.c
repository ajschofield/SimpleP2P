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
    struct sockaddr_in peer_addr = discover_peer();

    printf("Peer found at %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    int connection_fd = establish_connection(peer_addr);
    if (connection_fd < 0)
    {
        fprintf(stderr, "Failed to establish connection with peer\n");
        return -1;
    }

    printf("Connection established!\n");

    // TESTING: Keep alive for ten seconds - to test under different network conditions
    sleep(10);

    close(connection_fd);
    printf("Connection closed\n");

    return 0;
}