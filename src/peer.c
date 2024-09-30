#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "discovery.h"
#include "peer.h"

#define INIT_PORT 5000
#define MIN_PORT 49152
#define MAX_PORT 65535
#define TIMEOUT 10
#define MAX_ATTEMPTS 10

int create_tcp_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket creation failed");
        return -1;
    }
    return sock;
}

int make_socket_non_blocking(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

int negotiate_port(int socket_fd)
{
    srand(time(NULL));
    char buffer[256];
    int proposed_port;

    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++)
    {
        proposed_port = (rand() % (MAX_PORT - MIN_PORT + 1)) + MIN_PORT;

        snprintf(buffer, sizeof(buffer), "PORT_PROPOSAL:%d", proposed_port);
        if (send(socket_fd, buffer, strlen(buffer), 0) < 0)
        {
            perror("Failed to send port proposal");
            return -1;
        }

        memset(buffer, 0, sizeof(buffer));
        if (recv(socket_fd, buffer, sizeof(buffer) - 1, 0) < 0)
        {
            perror("Failed to receive port proposal response");
            return -1;
        }

        if (strncmp(buffer, "PORT_ACCEPT", 11) == 0)
        {
            printf("Port %d accepted\n", proposed_port);
            return proposed_port;
        }

        printf("Port %d rejected, trying again...\n", proposed_port);
    }

    printf("Failed to negotiate a port after %d attempts\n", MAX_ATTEMPTS);
    return -1;
}

int establish_connection(struct sockaddr_in peer_addr, int discovery_socket)
{
    int listen_sock, connect_sock;
    struct sockaddr_in my_addr;
    fd_set read_fds, write_fds;
    struct timeval tv;
    int max_fd, result;

    close(discovery_socket);

    listen_sock = create_tcp_socket();
    if (listen_sock < 0)
        return -1;

    int reuse = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(listen_sock);
        return -1;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(INIT_PORT);

    if (bind(listen_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        perror("bind failed");
        close(listen_sock);
        return -1;
    }

    if (listen(listen_sock, 1) < 0)
    {
        perror("listen failed");
        close(listen_sock);
        return -1;
    }

    connect_sock = create_tcp_socket();
    if (connect_sock < 0)
    {
        close(listen_sock);
        return -1;
    }

    if (make_socket_non_blocking(listen_sock) < 0 ||
        make_socket_non_blocking(connect_sock) < 0)
    {
        perror("failed to set non-blocking");
        close(listen_sock);
        close(connect_sock);
        return -1;
    }

    peer_addr.sin_port = htons(INIT_PORT);
    result = connect(connect_sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    if (result < 0 && errno != EINPROGRESS)
    {
        perror("connect failed");
        close(listen_sock);
        close(connect_sock);
        return -1;
    }

    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_SET(listen_sock, &read_fds);
        FD_SET(connect_sock, &write_fds);
        max_fd = (listen_sock > connect_sock) ? listen_sock : connect_sock;

        result = select(max_fd + 1, &read_fds, &write_fds, NULL, &tv);
        if (result < 0)
        {
            perror("select failed");
            break;
        }
        else if (result == 0)
        {
            printf("Connection timed out\n");
            break;
        }

        if (FD_ISSET(connect_sock, &write_fds))
        {
            close(listen_sock);
            int negotiated_port = negotiate_port(connect_sock);
            if (negotiated_port < 0)
            {
                close(connect_sock);
                return -1;
            }
            close(connect_sock);

            connect_sock = create_tcp_socket();
            if (connect_sock < 0)
                return -1;
            peer_addr.sin_port = htons(negotiated_port);
            if (connect(connect_sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0)
            {
                perror("Failed to connect on negotiated port");
                close(connect_sock);
                return -1;
            }
            printf("Connected on negotiated port %d\n", negotiated_port);
            return connect_sock;
        }

        if (FD_ISSET(listen_sock, &read_fds))
        {
            int new_sock = accept(listen_sock, NULL, NULL);
            if (new_sock < 0)
            {
                perror("accept failed");
                break;
            }
            close(listen_sock);
            close(connect_sock);

            int negotiated_port = negotiate_port(new_sock);
            if (negotiated_port < 0)
            {
                close(new_sock);
                return -1;
            }
            close(new_sock);

            listen_sock = create_tcp_socket();
            if (listen_sock < 0)
                return -1;
            my_addr.sin_port = htons(negotiated_port);
            if (bind(listen_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0 ||
                listen(listen_sock, 1) < 0)
            {
                perror("Failed to listen on negotiated port");
                close(listen_sock);
                return -1;
            }
            new_sock = accept(listen_sock, NULL, NULL);
            if (new_sock < 0)
            {
                perror("accept on negotiated port failed");
                close(listen_sock);
                return -1;
            }
            close(listen_sock);
            printf("Accepted connection on negotiated port %d\n", negotiated_port);
            return new_sock;
        }
    }

    close(listen_sock);
    close(connect_sock);
    return -1;
}