#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <netinet/in.h>

struct peer_info
{
    struct sockaddr_in addr;
    int discovery_socket;
};

void generate_unique_id(void);
int create_broadcast_socket(void);
void send_broadcast(int sockfd);
int listen_for_broadcast(int sockfd, struct sockaddr_in *peer_addr);
struct peer_info discover_peer(void);

#endif