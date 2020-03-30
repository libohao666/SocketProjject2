/*************************************************************************
	> File Name: client.c
	> Author: suyelu
	> Mail: suyelu@haizeix.com
	> Created Time: 六  3/28 14:51:21 2020
 ************************************************************************/


#include "head.h"
#include "tcp_client.h"
#include "common.h"

int main(int argc, char **argv) {
    char msg[512] = {0};
    int sockfd;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s ip port\n", argv[0]);
        return 1;
    }

    if ((sockfd = socket_connect(argv[1], atoi(argv[2]))) < 0) {
        perror("socket_connect");
        return 2;
    }
    make_nonblock_ioctl(sockfd);
    recv(sockfd, msg, 512, 0);
    printf("recv : %s\n", msg);
    close(sockfd);
    return 0;
} 
