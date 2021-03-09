# portscanner

Simple port scanner written by c in linux to scan tcp port of target ip/domain name.

# Usage

```
Usage: portscanner <target ip/domain name> <first port> <last port>
```

# Test

```
$ ./portscanner www.iqiyi.com 1 1000
Scanning host www.iqiyi.com tcp port from 1 to 1000...

[+] 80 open
[+] 443 open

Scanning 1000 ports done. 998 ports are not open.
```
