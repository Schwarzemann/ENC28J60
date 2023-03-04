/*
 * This implementation uses sockets to send and receive Ethernet frames. 
 * The ethernet_driver_init function creates a raw socket and sets up the socket address structure. 
 * The ethernet_driver_send function creates an Ethernet header with a 
 * destination MAC address of 0xff:0xff:0xff:0xff:0xff:0xff (broadcast), a source MAC address of 0xaa:0xbb:0xcc:0xdd:0xee:0xff, 
 * and a type of ETH_P_IP. It then copies the provided data buffer into the Ethernet packet and sends it using sendto. 
 * The ethernet_driver_receive function receives an Ethernet packet using recvfrom function.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include "ethernet_driver.h"

#define ETH_BUF_SIZE 1024

static int sockfd = -1;
static struct sockaddr_ll sock_addr = {0};

int ethernet_driver_init(void) {
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    sock_addr.sll_family = AF_PACKET;
    sock_addr.sll_halen = ETH_ALEN;
    sock_addr.sll_ifindex = if_nametoindex("eth0");
    if (sock_addr.sll_ifindex == 0) {
        perror("if_nametoindex");
        return -1;
    }
    memcpy(sock_addr.sll_addr, "\xff\xff\xff\xff\xff\xff", 6);

    return 0;
}

int ethernet_driver_send(const uint8_t* data, uint32_t len) {
    uint8_t buf[ETH_BUF_SIZE];
    uint32_t buflen = sizeof(ethernet_header_t) + len;
    if (buflen > ETH_BUF_SIZE) {
        fprintf(stderr, "Ethernet packet too large: %u\n", buflen);
        return -1;
    }

    ethernet_header_t* eth_hdr = (ethernet_header_t*)buf;
    memcpy(eth_hdr->dest, sock_addr.sll_addr, ETH_ALEN);
    memcpy(eth_hdr->src, "\xaa\xbb\xcc\xdd\xee\xff", ETH_ALEN);
    eth_hdr->type = htons(ETH_P_IP);

    memcpy(buf + sizeof(ethernet_header_t), data, len);

    if (sendto(sockfd, buf, buflen, 0, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
        perror("sendto");
        return -1;
    }

    return 0;
}

int ethernet_driver_receive(uint8_t* data, uint32_t len) {
    uint8_t buf[ETH_BUF_SIZE];
    ssize_t nbytes = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
    if (nbytes < 0) {
        perror("recvfrom");
        return -1;
    }

    ethernet_header_t* eth_hdr = (ethernet_header_t*)buf;
    if (eth_hdr->type != htons(ETH_P_IP)) {
        return -1;
    }

    uint32_t payload_len = nbytes - sizeof(ethernet_header_t);
    if (payload_len > len) {
        fprintf(stderr, "Ethernet packet too large: %u\n", payload_len);
        return -1;
    }

    memcpy(data, buf + sizeof(ethernet_header_t), payload_len);

    return payload_len;
}
