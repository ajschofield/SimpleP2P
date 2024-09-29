#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;

    // First, let's create the socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("There was an issue creating the socket.");
        return 1
    }

    // Then, initialise address structure.
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    return 0
}