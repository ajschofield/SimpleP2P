#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket.h"
#include "peer.h"

int main(void)
{
    printf("Starting...\n");

    int sockfd = create_socket();
    if (sockfd < 0)
    {
        fprintf(stderr, "Socket creation failed.");
        exit(EXIT_FAILURE);
    }

    return 0;
}