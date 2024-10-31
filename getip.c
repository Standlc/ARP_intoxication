#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    char hostname[65000];
    if (gethostname(hostname, sizeof(hostname))) {
        perror("gethostname");
        exit(1);
    }

    printf("host: %s\n", hostname);

    struct hostent *host = gethostbyname(hostname);
    if (host == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
    for (int i = 0; addr_list[i]; i++) {
        printf("%s\n", inet_ntoa(*addr_list[i]));
    }
    return 0;
}