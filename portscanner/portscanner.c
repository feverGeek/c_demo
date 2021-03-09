
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int port_scanner(char *ip, int first_port, int last_port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *record;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int total = last_port - first_port + 1;
	int notopen = total;
	int ret, status;
	char port[6] = { 0 };

	struct timeval timeout;
	socklen_t len = sizeof(timeout);

	timeout.tv_sec = 0;
	timeout.tv_usec = 50000; // 0.05 sec

	printf("Scanning host %s tcp port from %d to %d...\n\n", ip, first_port, last_port);
	for (int i = first_port; i <= last_port; i++)
	{
		sprintf(port, "%d", i);
		if ((ret = getaddrinfo(ip, port, &hints, &res)) != 0)
		{
			fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret));
			return -1;
		}

		for (record = res; record != NULL; record = record->ai_next)
		{
			status = socket(record->ai_family, record->ai_socktype, record->ai_protocol);
			if (status == -1) continue;

			setsockopt(status, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
			if (connect(status, record->ai_addr, record->ai_addrlen) != -1)
			{
				printf("[+] %d open\n", i);
				notopen--;
				close(status);
				break;
			}
			close(status);
		}
	}
	printf("\nScanning %d ports done. %d ports are not open.\n", total, notopen);

	freeaddrinfo(res);
	return 0;
}

int isdigit_str(char *str)
{
	for (int i = 0; str[i] != '\0'; i++)
		if (str[i] < '0' || str[i] > '9') return 0;
	return 1;
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: portscanner <target ip/domain name> <first port> <last port>\n");
		exit(-1);
	}

	if (!isdigit_str(argv[2]) || !isdigit_str(argv[3]))
	{
		fprintf(stderr, "Port number must be an integer.\n");
		exit(-1);
	}

	int first_port = atoi(argv[2]);
	int last_port = atoi(argv[3]);
	if (first_port > last_port)
	{
		fprintf(stderr, "Last port number must be larger than first port number.\n");
		exit(-1);
	}
	port_scanner(argv[1], first_port, last_port);

	return 0;
}
