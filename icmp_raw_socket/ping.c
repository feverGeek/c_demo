#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <setjmp.h>
#include <error.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

#define PACKET_SIZE 4096
#define MAX_WAIT_TIME 5
#define MAX_NO_PACKETS 3

char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

int sockfd, datalen = 56;
int nsend = 0, nreceived = 0;
struct sockaddr_in dest_addr;
pid_t pid;
struct sockaddr_in from;
struct timeval tvrecv;

void statistics(int signo);
unsigned short cal_chksum(unsigned short *addr, int len);
int pack(int pack_no);
void send_pack(void);
void recv_pack(void);
int unpack(char *buf, int len);
void tv_sub(struct timeval *out, struct timeval *in);

void statistics(int signo)
{
    printf("\n-----------ping statistics----------\n");
    printf("%d packtes trasmitted, %d received, %%%d lost\n", nsend, nreceived, (nsend-nreceived)/nsend*100);
    close(sockfd);
    exit(1);
}

unsigned short cal_chksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    /* 把ICMP报头以2字节为单位累加起来 */
    while(nleft > 1)
    {
	sum += *w++;
	nleft -= 2;
    }

    /* 若icmp报头位奇数个字节，会剩下一个字节，把最后一个字节视为一个2字节数据的高字节,这2个字节数据的低字节为0, 继续累加 */
    if(nleft == 1)
    {
	*(unsigned char *)(&answer) = *(unsigned char *)w;
	sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}

int pack(int pack_no)
{
    int i, packsize;
    struct icmp *icmp;
    struct timeval *tval;

    icmp = (struct icmp *)sendpacket;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = pack_no;
    icmp->icmp_id = pid;
    packsize = 8 + datalen;
    tval = (struct timeval *)icmp->icmp_data;
    gettimeofday(tval, NULL); /* 记录发送时间 */
    icmp->icmp_cksum = cal_chksum( (unsigned short *)icmp, packsize);
    return packsize;
}

void send_pack()
{
    int packetsize;
    while(nsend < MAX_NO_PACKETS)
    {
	nsend++;
	packetsize = pack(nsend);
	if(sendto(sockfd, sendpacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
	{
	    perror("sendto error");
	    continue;
	}
	sleep(1);
    }
}

void recv_pack()
{
    int n, fromlen;
    extern int errno;

    signal(SIGALRM, statistics);
    fromlen = sizeof(from);
    while( nreceived < 10 )
    {
	alarm(MAX_WAIT_TIME);
	if((n=recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, &fromlen)) < 0)
	{
	    if(errno == EINTR)
		continue;
	    perror("recvfrom error");
	    continue;
	}
	gettimeofday(&tvrecv, NULL);
	if(unpack(recvpacket, n) == -1)
	    continue;
	nreceived++;
    }
}

int unpack(char *buf, int len)
{
    int i, iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    double rtt;
    ip = (struct ip *)buf;
    iphdrlen = (ip->ip_hl) * 4; /* 求ip报头长度，即ip报头的长度标志乘4 */
    icmp = (struct icmp *)(buf + iphdrlen); /* 越过ip报头，指向icmp报头 */
    len -= iphdrlen; /* icmp报头及icmp数据报的总长度 */
    if(len < 8)
    {
	printf("icmp packets\'s length is less than 8\n");
	return -1;
    }

    /* 确保所接受的是我所发送icmp的回应 */
    if( (icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid) )
    {
	tvsend = (struct timeval *)icmp->icmp_data;
	tv_sub(&tvrecv, tvsend); /* 接受和发送的时间差 */
	rtt = tvrecv.tv_sec * 1000 + tvrecv.tv_usec / 1000; /* 以毫秒为单位 */
	/* 显示相关信息 */
	printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n",
		len, inet_ntoa(from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
    }
    else 
	return -1;
}

int main(int argc, char *argv[])
{
    struct hostent *host;
    struct protoent *protocol;
    unsigned long inaddr = 0l;
    int waittime = MAX_WAIT_TIME;
    int size = 50*1024;

    if(argc < 2)
    {
	printf("usage: %s hostname/IP address\n", argv[0]);
	exit(1);
    }

    if((protocol=getprotobyname("icmp")) == NULL)
    {
	perror("getprotobyname");
	exit(1);
    }

    /* 生成使用icmp的原始套接字 */
    if((sockfd=socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
    {
	perror("socket error");
	exit(1);
    }

    setuid(getuid());
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if((host=gethostbyname(argv[1])) == NULL)
    {
	perror("gethostbyname error");
	exit(1);
    }
    dest_addr.sin_addr = *((struct in_addr *)host->h_addr);

    pid = getpid();
    printf("PING %s(%s): %d types data in ICMP packets.\n", argv[1], inet_ntoa(dest_addr.sin_addr), datalen);
    send_pack();
    recv_pack();
    statistics(SIGALRM);
    return 0;
}

void tv_sub(struct timeval *out, struct timeval *in)
{
    if((out->tv_usec -= in->tv_usec)<0)
    {
	--out->tv_sec;
	out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}
