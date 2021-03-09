#ifndef __IP_H
#define __IP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#define PACKET_SIZE 4096
#define MAX_WAIT_TIME 5
#define DEST_ADDR "10.0.0.62"

extern int errno;
char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

int sockfd, datalen = 56;
struct sockaddr_in dest_addr;

void send_packet();
void recv_packet();

unsigned short cal_chksum(unsigned short *addr, int len);
void showiphdr(struct ip *ip);
void onTerm();

#endif
