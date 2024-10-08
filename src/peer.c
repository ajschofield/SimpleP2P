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
#include "colours.h"

#define INIT_PORT 9090
#define MIN_PORT 49152
#define MAX_PORT 65535
#define TIMEOUT 10
#define MAX_ATTEMPTS 10

int create_tcp_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror(BRED "TCP socket creation failed" COLOR_RESET);
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
            perror(BRED "Failed to send port proposal" COLOR_RESET);
            return -1;
        }

        memset(buffer, 0, sizeof(buffer));
        if (recv(socket_fd, buffer, sizeof(buffer) - 1, 0) < 0)
        {
            perror(BRED "Failed to receive port proposal response" COLOR_RESET);
            return -1;
        }

        if (strncmp(buffer, "PORT_ACCEPT", 11) == 0)
        {
            printf(BGRN "Port %d accepted\n" COLOR_RESET, proposed_port);
            return proposed_port;
        }

        printf(BYEL "Port %d rejected, trying again...\n" COLOR_RESET, proposed_port);
    }

    printf(BRED "Failed to negotiate a port after %d attempts\n" COLOR_RESET, MAX_ATTEMPTS);
    return -1;
}

void log_err(const char *message)
{
    fprintf(stderr, BRED "Error: %s - %s\n" COLOR_RESET, message, strerror(errno));
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
        log_err("setsockopt(SO_REUSEADDR) failed");
        close(listen_sock);
        return -1;
    }

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(INIT_PORT);

    if (bind(listen_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        log_err("Failed to bind listen socket");
        close(listen_sock);
        return -1;
    }

    if (listen(listen_sock, 1) < 0)
    {
        log_err("Failed to listen on listen socket");
        close(listen_sock);
        return -1;
    }

    printf("Listening for incoming connections on port %d\n", INIT_PORT);

    connect_sock = create_tcp_socket();
    if (connect_sock < 0)
    {
        close(listen_sock);
        return -1;
    }

    make_socket_non_blocking(listen_sock);
    make_socket_non_blocking(connect_sock);

    printf(BMAG "Attempting to connect to peer at %s:%d\n" COLOR_RESET,
           inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    if (connect(connect_sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            log_err("Failed to initiate connection to peer");
            close(listen_sock);
            close(connect_sock);
            return -1;
        }
        printf(BYEL "Connection in progress...\n" COLOR_RESET);
    }
    else
    {
        printf(BGRN "Connection established immediately\n" COLOR_RESET);
        close(listen_sock);
        return connect_sock;
    }

    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++)
    {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_SET(listen_sock, &read_fds);
        FD_SET(connect_sock, &write_fds);
        max_fd = listen_sock > connect_sock ? listen_sock : connect_sock;

        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        result = select(max_fd + 1, &read_fds, &write_fds, NULL, &tv);
        if (result < 0)
        {
            log_err("Select failed");
            break;
        }
        else if (result == 0)
        {
            fprintf(stderr, BRED "Connection attempt %d timed out\n" COLOR_RESET, attempt + 1);
            continue;
        }

        if (FD_ISSET(connect_sock, &write_fds))
        {
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(connect_sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            {
                log_err("getsockopt failed");
                continue;
            }
            printf(BGRN "Outgoing connection established successfully.\n" COLOR_RESET);
            close(listen_sock);
            return connect_sock;
        }

        if (FD_ISSET(listen_sock, &read_fds))
        {
            int new_sock = accept(listen_sock, NULL, NULL);
            if (new_sock < 0)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    log_err("Accept failed");
                    break;
                }
                continue;
            }
            printf(BGRN "Incoming connection accepted.\n" COLOR_RESET);
            close(listen_sock);
            close(connect_sock);
            return new_sock;
        }
    }

    // We shouldn't reach this point!
    printf(BRED "Failed to establish connection after %d attempts\n" COLOR_RESET, MAX_ATTEMPTS);
    close(listen_sock);
    close(connect_sock);
    return -1;
}