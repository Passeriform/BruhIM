#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_SIZE 90

int choose_valid_port(int sockfd, struct sockaddr_in server_addr, int retries)
{
    int bind_return = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (bind_return == -1) {
        if (retries == 0) {
            return -1;
        }
        server_addr.sin_port=htons(ntohs(server_addr.sin_port) + 1);
        return choose_valid_port(sockfd, server_addr, retries - 1);
    } else {
        return server_addr.sin_port;
    }
}

int main()
{
    int sockfd, client_sockfd;
    int recieved_bytes, sent_bytes;
    int bind_return;
    char buff[MAX_SIZE];

    socklen_t client_addr_length;
    struct sockaddr_in server_addr, client_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("\nError in creation of Socket\n");
        return 0;
    }

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(3388);
    server_addr.sin_addr.s_addr=htons(INADDR_ANY);
    bind_return = choose_valid_port(sockfd, server_addr, 10);

    if (bind_return == -1) {
        printf("\nBinding Error\n");
        close(sockfd);
        return 0;
    }

    bind_return = listen(sockfd, 10);
    if (bind_return == -1) {
        printf("\nError during Listening\n");
        close(sockfd);
        return 0;
    }

    client_addr_length = sizeof(client_addr);
    client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_length);
    if (client_sockfd == -1) {
        printf("\nClient Socket couldn't be established\n");
        close(sockfd);
        return 0;
    }

    while (1) {
        recieved_bytes = recv(client_sockfd, buff, sizeof(buff), 0);
        if (recieved_bytes == -1) {
            printf("\nNo Bytes recieved\n");
            close(sockfd);
            close(client_sockfd);
            return 0;
        }

        printf("\n%s\n", buff);
        sent_bytes = send(client_sockfd, buff, sizeof(buff), 0);

        if (sent_bytes == -1) {
            printf("\nNo Bytes sent\n");
            close(sockfd);
            close(client_sockfd);
            return 0;
        }
    }
}
