#ifndef UDP_H
#define UDP_H

#include <stdint.h>
#include <arpa/inet.h>

#define UDP_HDR_SIZE 8

#define INET_HDR_LEN 5

unsigned int build_udp_packet(struct sockaddr_in src_addr, struct sockaddr_in dst_addr, uint8_t *udp_packet, uint8_t *data, unsigned int data_size);

unsigned int build_ip_packet(struct in_addr src_addr, struct in_addr dst_addr, uint8_t protocol, uint8_t *ip_packet, uint8_t *data, unsigned int data_size);

void send_udp_packet(int raw_sock, struct sockaddr_in src_addr, struct sockaddr_in dst_addr, uint8_t *data, unsigned int data_size);

#endif

