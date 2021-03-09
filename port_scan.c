#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <signal.h>
//#include <libxml/parser.h>
//#include <libxml/tree.h>
#define TRUE 1
#define FALSE 0
#define UDP "UDP"
#define TCP "TCP"
#define tcp "tcp"
#define udp "udp"
typedef struct _GsSockStru{
    int fd;
    int len;
    struct sockaddr_in addr;
}GsSockStru;

static int tcptest( char ip[32], char port[20]);
static int udptest( char ip[32], char port[20]);
void sig_alrm( int signo );
static GsSockStru test_sock;

int main( int argc, char** argv)
{
    char string[64];
    char port[20];
    char pro[20];
    char ip[32];
    int res;
    int i = 0;
    int k = 0;
    if( argc>2 || argc<2 )
    {
	printf("usage: test protocol:ip\n");
	return ( -1 );
    }
    strcpy( string, argv[1]);
    /* get protocol */
    while( *string )
    {
	if( string[i] == ':' )
	    break;
	pro[k] = string[i];
	k++;
	i++;
    }
    pro[k] = '\0';
    i++;
    k = 0;
    /* get ip */
    while( *string )
    {
	if( string[i] == ':')
	    break;
	ip[k] = string[i];
	k++;
	i++;
    }
    ip[k] = '\0';
    i++;
    k=0;
    /* get port */
    while( *string )
    {
	if( string[i] == '\0')
	    break;
	port[k] = string[i];
	k++;
	i++;
    }
    port[k] = '\0';
    i++;
    memset( &test_sock, 0, sizeof( test_sock ) );
    if ( ( strcmp( TCP, pro) != 0 ) && ( strcmp( UDP, pro) != 0 ) && ( strcmp( tcp, pro) != 0 ) && ( strcmp( udp, pro) != 0 ))
    {
	printf ( "协议错误: 支持tcp/udp\n" );
	return (-1);
    }
    if ( strcmp( TCP, pro) == 0 || strcmp( tcp, pro) == 0 )
	res = tcptest( ip, port );
    if ( strcmp( UDP, pro) == 0 || strcmp( udp, pro) == 0 )
	res = udptest( ip, port );printf("%d\n",res);
    return ( res );
}

int tcptest( char ip[32], char port[20])
{
    int res;
    struct timeval tv;
    test_sock.fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( test_sock.fd < 0 )
    {
	printf( "create socket failed -3 \n" );
	return ( -3 );
    }
    memset( &( test_sock.addr ), 0, sizeof( test_sock.addr ) );
    test_sock.addr.sin_family = AF_INET;
    test_sock.addr.sin_port = htons( atoi( port ) );
    inet_pton( AF_INET, ip, &test_sock.addr.sin_addr );
    test_sock.len = sizeof( struct sockaddr );
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt( test_sock.fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof( tv ) );
    res = connect( test_sock.fd,( struct sockaddr * )( &( test_sock.addr ) ),
    test_sock.len );
    if ( res < 0 )
    {
	fprintf( stderr, "connect failed 0\n" );
	close( test_sock.fd );
	return FALSE;
    }
    close( test_sock.fd );
    return TRUE;
}

int udptest( char ip[32], char port[20])
{
    struct icmphdr *icmp_header;
    struct sockaddr_in target_info;
    int target_info_len;
    fd_set read_fd;
    int scan_port;
    char recvbuf[5000];
    struct sockaddr_in target_addr;
    int icmp_socket;
    int udp_socket;
    struct timeval tv;
    icmp_header = (struct icmphdr *)(recvbuf+sizeof(struct iphdr));
    scan_port = atoi( port );
    target_addr.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &target_addr.sin_addr );
    target_addr.sin_port = htons(scan_port);
    if ((udp_socket=socket(AF_INET,SOCK_DGRAM,0))==-1)
    {
	printf("create socket failed -3\n");
	return -3;
    }
    if ((icmp_socket=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP))==-1)
    {
	printf("Create raw socket failed -3\n");
	return -3;
    }
    sendto(udp_socket,NULL,0,0,(void *)&target_addr,sizeof(target_addr));
    FD_ZERO(&read_fd);
    FD_SET(icmp_socket,&read_fd);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    select(FD_SETSIZE,&read_fd,NULL,NULL,&tv);
    for (;;){
	if (FD_ISSET(icmp_socket,&read_fd))
	{
	    target_info_len = sizeof(target_info);
	    recvfrom(icmp_socket,recvbuf,5000,0,
	    (struct sockaddr *)&target_info,&target_info_len);
	    if (target_info.sin_addr.s_addr == target_addr.sin_addr.s_addr && icmp_header->type == 3 && icmp_header->code<=12)
	    {
		printf("Port %d : Close\n",scan_port);
		return (0);
	    }
	}
	return (1);
    }
}
