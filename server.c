#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        exit(1);
    }

    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    servaddr.sin_port = htons(5000);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        exit(1);
    }

    if (listen(sockfd, 20)) {
        exit(1);
    }

    // struct sockaddr_in addr;
    // size_t addr_size = sizeof(addr);
    // getsockname(sockfd, (void *)&addr, (void *)&addr_size);

    // printf("%s\n", inet_ntoa(addr.sin_addr));

    // char ip[1024];
    // inet_ntop(AF_INET, &servaddr.sin_addr, ip, sizeof(ip));
    // printf("%s\n", ip);

    while (1) {
        int clientfd = accept(sockfd, NULL, NULL);
        if (clientfd == -1) {
            perror("accept");
            exit(1);
        }

        printf("NEW CLIENT\n");

        char buff[1024];
        int len = read(clientfd, buff, sizeof(buff) - 1);
        if (len == -1) {
            perror("accept");
            exit(1);
        }
        buff[len] = 0;
        printf("messgage from client: %s\n", buff);
    }

    exit(0);
}