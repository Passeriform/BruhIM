#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int sockfd, connect_return;
    int recieved_bytes, sent_bytes;
    struct sockaddr_in server_addr;

    char buff[90];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("\nSocket Creation Error\n");
        return 0;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3388);
    server_addr.sin_addr.s_addr = inet_addr("172.16.53.80");
    connect_return = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (connect_return == -1) {
        printf("\nFailed to Connect to server\n");
        close(sockfd);
        return 0;
    }

    while (1) {
        printf("\nEnter Text\n");
        scanf("%s", buff);

        sent_bytes = send(sockfd, buff, sizeof(buff), 0);
        if (sent_bytes == -1) {
            printf("\nCoudn't send bytes\n");
            return 0;
        }

        recieved_bytes = recv(sockfd, buff, sizeof(buff), 0);
        if (recieved_bytes == -1) {
            printf("\nError in recieving bytes\n");
            return 0;
        }
    }

    printf("%s", buff);
    close(sockfd);
}
