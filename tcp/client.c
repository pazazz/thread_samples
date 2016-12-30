#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <errno.h>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <assert.h>
#include    <arpa/inet.h>
#include    <unistd.h>

#define MAXLINE 4096

#define TEST_MSG    "test_msg"

static int test_1(const char *host, const char *port);
static int test_2(const char *host, const char *port);
static int test_3(const char *host, const char *port);

int main(int argc, char** argv)
{
    int type = 0;

    /* choise test case */
    if (argc < 2)
    {
        printf("option is invalid");
        return -1;
    }

    switch (type) {
        case 0:
        {
            test_1(argv[1], argv[2]);
            break;
        }

        case 1:
        {
            test_2(argv[1], argv[2]);
            break;
        }

        case 2:
        {
            test_3(argv[1], argv[2]);
            break; 
        }

        default:
            printf("other case %d", type);
    }
    return 0;
}

/*
 * test
 *   connect 3 handshark
 *   send data
 *   close 4 handshark
 */
static int
test_1(const char *host, const char *port)
{
    int     sockfd = -1;
    struct  sockaddr_in servaddr;
    int     p = 0;
    char    *endptr = NULL;

    assert(host != NULL && port != NULL);

    p  = strtol(port,  &endptr, 10);
    if ((endptr != NULL && *endptr != '\0') ||
            (errno == ERANGE) || endptr == port) {
        return -1;
    }
    

    printf("test_1\n");

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(p);

    if (inet_pton(AF_INET, host, &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", host);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    } 

    if (send(sockfd, TEST_MSG, strlen(TEST_MSG), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    close(sockfd);

    return 0;
}

static int
test_2(const char *host, const char *port)
{
    return 0;
}

static int
test_3(const char *host, const char *port)
{
    return 0;
}
