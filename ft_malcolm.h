#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct arp_header {
    uint16_t htype;         // Hardware type (Ethernet is 1)
    uint16_t ptype;         // Protocol type (IPv4 is 0x0800)
    uint8_t hlen;           // Hardware address length (6 for MAC)
    uint8_t plen;           // Protocol address length (4 for IPv4)
    uint16_t opcode;        // Operation (1 for request, 2 for reply)
    uint8_t sender_mac[6];  // Sender MAC address
    uint8_t sender_ip[4];   // Sender IP address
    uint8_t target_mac[6];  // Target MAC address
    uint8_t target_ip[4];   // Target IP address
};

struct arp_options {
    uint8_t target_mac[6];
    uint8_t target_ip[4];
    uint8_t source_mac[6];
    uint8_t source_ip[4];
};

#define ARP_SIZE sizeof(struct ether_header) + sizeof(struct arp_header)
