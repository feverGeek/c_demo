#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {FALSE = 0, TRUE = 1} bool;

char *bind_addr = NULL;
char *remote_host = NULL;
char *cmd_in = NULL;
char *cmd_out = NULL;
int remote_port = 0;
bool foreground = FALSE;
bool use_syslog = FALSE;

#define SYNTAX_ERROR -10

int parse_options(int argc, char *argv[]) {
    int c, local_port = 0;

    while ((c = getopt(argc, argv, "b:l:h:p:i:o:fs")) != -1) {
        switch(c) {
            case 'l':
                local_port = atoi(optarg);
                printf("l");
                break;
            case 'b':
                bind_addr = optarg;
                printf("b");
                break;
            case 'h':
                remote_host = optarg;
                printf("h");
                break;
            case 'p':
                remote_port = atoi(optarg);
                printf("p");
                break;
            case 'i':
                cmd_in = optarg;
                printf("i");
                break;
            case 'o':
                cmd_out = optarg;
                printf("o");
                break;
            case 'f':
                foreground = TRUE;
                printf("f");
                break;
            case 's':
                use_syslog = TRUE;
                printf("s");
                break;
        }
    }

    if (local_port && remote_host && remote_port) {
        return local_port;
    } else {
        return SYNTAX_ERROR;
    }
}


int main(int argc, char * argv[])
{
    parse_options(argc, argv);
    return 0;
}
