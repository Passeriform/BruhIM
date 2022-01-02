#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int clientRun(int argc, char **argv)
{
    int sockfd, connect_return, opt=-1;
    char feed_addr[200];
    int recieved_bytes, sent_bytes;
    struct sockaddr_in server_addr;

    config connConfig;

    char buff[90];


    while ((opt = getopt(argc, argv, "m:a:")) != -1) {
        switch (opt) {
            case 'm':
                sprintf( buff, "%s", optarg );
                break;

            case 'a':
                sprintf( feed_addr, "%s", optarg );
                break;

            case '':
            	printf("False params");

        }
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("\n[B_SOCKET:000] Socket creation error\n");
        return 0;
    }

    printf("%s\n", buff);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    server_addr.sin_addr.s_addr = inet_addr(feed_addr);
    connect_return = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (connect_return == -1) {
        printf("\n[B_SOCKET:001] Failed to connect to server\n");
        close(sockfd);
        return 0;
    }

    if (initiated) {
        sent_bytes = send(sockfd, buff, sizeof(buff), 0);
        if (sent_bytes == -1) {
            printf("\n[B_TRANS:000] Coudn't send bytes\n");
            return 0;
        }

        recieved_bytes = recv(sockfd, buff, sizeof(buff), 0);
        if (recieved_bytes == -1) {
            printf("\n[B_TRANS:001] Error in recieving bytes\n");
            return 0;
        }
    }
    else {
        while (1) {
            sent_bytes = send(sockfd, buff, sizeof(buff), 0);
            if (sent_bytes == -1) {
                printf("\n[B_TRANS:000] Coudn't send bytes\n");
                return 0;
            }

            recieved_bytes = recv(sockfd, buff, sizeof(buff), 0);
            if (recieved_bytes == -1) {
                printf("\n[B_TRANS:001] Error in recieving bytes\n");
                return 0;
            }
        }
    }

    printf("%s", buff);
    close(sockfd);
}
