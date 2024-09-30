#ifndef PEER_H
#define PEER_H

#include <netinet/in.h>

int create_tcp_socket(void);
int make_socket_non_blocking(int sock);
int establish_connection(struct sockaddr_in peer_addr, int discovery_socket);

#endif