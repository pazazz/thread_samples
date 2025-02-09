#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char** argv)
{
    int    listenfd, connfd;
    struct sockaddr_in     servaddr;
    char    buff[4096];
    int     n;
    int     port = 0;

    if (argc < 2) {
        printf("exit");
        return -1;
    }

    port = atoi(argv[1]);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        exit(0);
    }

    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    printf("======waiting for client's request======\n");
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1) {
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            continue;
        }

        n = recv(connfd, buff, MAXLINE, 0);
        buff[n] = '\0';
        printf("recv msg from client: %s\n", buff);

        /* wait closed by client */
        sleep(130);
        close(connfd);
        printf("close socket %d\n", connfd);
    }

    close(listenfd);
}
