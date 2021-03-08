// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef WIN32  // Windows
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <winsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <mswsock.h>
#include<errno.h>
#include "getopt.h"
#endif

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SoCKEADDR sockaddr
#define WSAGetLastError() (errno)
#define closesocket(s) close(s)
#define ioctlsocket ioctl
#define WSAEWOULDBLOCK EWOULDBLOCK
#define Sleep(s) usleep(s*1000)
#endif

#define DEFAULT_SERVER "192.168.199.129"
#define BUFLEN 65536
#define PORT 4180	//The port on which to send data

/* DEFAULT define of send & recv */
#define DEFAULT_UPDATE 500                 
#define DEFAULT_PROTO "udp"
#define DEFAULT_BSIZE 1000

/* DEFAULT define of send */
#define DEFAULT_HOST_NAME "localhost"
#define DEFAULT_REMOTE_PORT_NUMBER 4180
#define DEFAULT_TXRATE 1000
#define DEFAULT_TOTAL_NUM_MESSAGES 0
#define DEFAULT_SEND_BUFFER 65536

// TODO: reference additional headers your program requires here

int SEND(int argc, char* argv[]);

int RECV(int argc, char* argv[]);

// TODO: reference additional headers your program requires here