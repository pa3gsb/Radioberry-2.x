#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>


#include "udp.h"

// For little endian
struct pseudo_iphdr
{
    uint32_t source_addr;
    uint32_t dest_addr;
    uint8_t zeros;
    uint8_t prot;
    uint16_t length;
};


uint16_t checksum(uint8_t *data, unsigned int size)
{
    int i;
    int sum = 0;
    uint16_t *p = (uint16_t *)data;

    for(i = 0; i < size; i += 2){
        sum += *(p++);
    }

    uint16_t carry = sum >> 16;
    uint16_t tmp = 0x0000ffff & sum;
    uint16_t res = ~(tmp + carry);

    return res;
}

unsigned int build_ip_packet(struct in_addr src_addr, struct in_addr dst_addr, uint8_t protocol, uint8_t *ip_packet, uint8_t *data, unsigned int data_size)
{
    struct iphdr *ip_header;

    ip_header = (struct iphdr *)ip_packet;
    ip_header->version = 4;
    ip_header->ihl = INET_HDR_LEN;
    ip_header->tos = 0;
    ip_header->tot_len = htons(INET_HDR_LEN * 4 + data_size);
    ip_header->id = 0; // Filled in automatically
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = protocol;
    ip_header->check = 0; // Filled in automatically
    ip_header->saddr = src_addr.s_addr;
    ip_header->daddr = dst_addr.s_addr;

    memcpy(ip_packet + sizeof(struct iphdr), data, data_size); 

    return sizeof(struct iphdr) + data_size;
}

#define MAX_PSEUDO_PKT_SIZE 4096

unsigned int build_udp_packet(struct sockaddr_in src_addr, struct sockaddr_in dst_addr, uint8_t *udp_packet, uint8_t *data, unsigned int data_size)
{
    uint8_t pseudo_packet[MAX_PSEUDO_PKT_SIZE];
    uint16_t length;
    struct udphdr *udph;
    struct pseudo_iphdr *p_iphdr = (struct pseudo_iphdr *)pseudo_packet;

    length = UDP_HDR_SIZE + data_size;
    udph = (struct udphdr *)udp_packet;
    udph->source = src_addr.sin_port;
    udph->dest = dst_addr.sin_port;
    udph->len = htons(length);
    memcpy(udp_packet + UDP_HDR_SIZE, data, data_size);

    if(length + sizeof(struct pseudo_iphdr) > MAX_PSEUDO_PKT_SIZE){
        fprintf(stderr, "Buffer size not enough");
        exit(1);
    }

    // Calculate checksum with pseudo ip header
    p_iphdr->source_addr = src_addr.sin_addr.s_addr;
    p_iphdr->dest_addr = dst_addr.sin_addr.s_addr;
    p_iphdr->zeros = 0;
    p_iphdr->prot = IPPROTO_UDP; //udp
    p_iphdr->length = udph->len;

    // Do NOT use udph->len instead of length.
    // udph->len is in big endian
    memcpy(pseudo_packet + sizeof(struct pseudo_iphdr), udph, length);
    udph->check = checksum(pseudo_packet, sizeof(struct pseudo_iphdr) + length);

    return length;
}

void send_udp_packet(int raw_sock, struct sockaddr_in src_addr, struct sockaddr_in dst_addr, uint8_t *data, unsigned int data_size)
{
    int flag = 1;
    unsigned int packet_size;
    unsigned int ip_payload_size;
    uint8_t packet[ETH_DATA_LEN]; 

    memset(packet, 0, ETH_DATA_LEN);
    ip_payload_size = build_udp_packet(src_addr, dst_addr, packet + sizeof(struct iphdr), data, data_size);

    packet_size = build_ip_packet(src_addr.sin_addr, dst_addr.sin_addr, IPPROTO_UDP, packet, packet + sizeof(struct iphdr), ip_payload_size);
	
    if(sendto(raw_sock, packet, packet_size, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr)) < 0){
        perror("send_udp_packet");
        exit(1);
    }
}
