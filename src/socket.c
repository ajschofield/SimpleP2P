#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MIN_PORT 1024
#define MAX_PORT 65535

int socket()
{
    int sockfd;
    struct sockaddr_in server_addr;
    int rand_port;

    srand(time(NULL));

    rand_port = (rand() % (MAX_PORT - MIN_PORT + 1)) + MIN_PORT;

    // First, let's create the socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("There was an issue creating the socket.");
        return 1;
    }

    // Then, initialise address structure.
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(rand_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("There was an issue binding the socket.");
        return 1;
    }

    printf("Socket is now bound to port %d\n", rand_port);

    close(sockfd);

    return 0;
}