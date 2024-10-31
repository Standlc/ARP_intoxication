#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
// #include <linux/if_link.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sockfd == -1) {
        perror("socket"), exit(1);
    }

    printf("Sniffing packets...\n");

    while (1) {
        char buff[1024];
        struct sockaddr_ll addr;
        socklen_t addr_size = sizeof(addr);
        int len = recvfrom(sockfd, buff, sizeof(buff), 0, (void *)&addr, &addr_size);
        if (len == -1) {
            perror("recvfrom"), exit(1);
        }

        if (len < (int)(sizeof(struct ether_header) + sizeof(struct ip))) {
            printf("\nReceived %d bytes in packet malformed\n", len);
            continue;
        }

        // ETHERNET HEADER
        struct ether_header *eth = (struct ether_header *)buff;
        char interface[1024];
        printf("\nReceived %d bytes on interface %s\n", len, if_indextoname(addr.sll_ifindex, interface));
        printf("\tEthernet header\n");
        printf("\t\tSource MAC address: %s\n", ether_ntoa((void *)&eth->ether_shost));
        printf("\t\tDestination MAC address: %s\n", ether_ntoa((void *)&eth->ether_dhost));

        // IP HEADER
        struct ip *iphdr = (void *)(buff + sizeof(*eth));
        printf("\tIP header\n");
        printf("\t\tSource IP address: %s\n", inet_ntoa(iphdr->ip_src));
        printf("\t\tDestination IP address: %s\n", inet_ntoa(iphdr->ip_dst));
        printf("\t\tProtocol: %s\n", iphdr->ip_p == IPPROTO_TCP    ? "TCP"
                                   : iphdr->ip_p == IPPROTO_ICMP ? "ICMP"
                                   : iphdr->ip_p == IPPROTO_UDP  ? "UDP"
                                                                 : "???");
    }

    exit(0);
}