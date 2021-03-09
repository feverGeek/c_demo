#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


int main(int argc, char * argv[])
{
    setlogmask (LOG_UPTO (LOG_NOTICE));

    openlog ("exampleprog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER );

    syslog (LOG_NOTICE, "Program started by User %d", getuid ());
    syslog (LOG_INFO, "A tree falls in a forest");

    closelog ();
    return 0;
}
