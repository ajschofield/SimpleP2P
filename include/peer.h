#ifndef PEER_H
#define PEER_H

#include <netinet/in.h>

int establish_connection(struct sockaddr_in peer_addr);

#endif