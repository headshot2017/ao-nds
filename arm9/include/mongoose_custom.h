#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <netdb.h>


#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
typedef int socklen_t;
#define TCP_ACKDELAYTIME   0x2001
#define TCP_NOACKDELAY     0x2002
#define TCP_MAXSEG         0x2003
#define TCP_NODELAY        0x2004

// See https://mongoose.ws/documentation/#build-options
#define MG_ARCH MG_ARCH_CUSTOM
#define MG_ENABLE_TCPIP 0
#define MG_ENABLE_SOCKET 1
#define MG_ENABLE_FILE 0
#define MG_ENABLE_DIRLIST 0
#define MG_ENABLE_FATFS 0
#define MG_IO_SIZE 256
#define MG_ENABLE_SSI 0

#define MG_CUSTOM_NONBLOCK(fd)
