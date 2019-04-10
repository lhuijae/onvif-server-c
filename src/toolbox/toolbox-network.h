
#ifndef __TOOLBOX_NETWORK_H__
#define __TOOLBOX_NETWORK_H__

extern char g_localip[24];

#define LINUX_NETWORK_RESOLV_CONF "/etc/resolv.conf"

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
/* Windows */
#	if defined(WINNT) || defined(_WINNT) || defined(__BORLANDC__) || defined(__MINGW32__) || defined(_WIN32_WCE)
#		define WIN32_LEAN_AND_MEAN
#		define _MSWSOCK_
#		include <winsock2.h>
#		include <ws2tcpip.h>
#	elif TOOLBOX_WIN95_BUILD == 1 // win95 does not support winsock2
#		pragma message("Including WINSOCK v1 for Windows 95 compatibility")
#	else
#		define WIN32_LEAN_AND_MEAN
#		include <winsock2.h>
#		include <ws2tcpip.h>
#	endif
#	include <windows.h>
#	include <string.h>

#	define closeSocket closesocket
#	define socketShutdown(a,b) shutdown(a, b)
#	define EWOULDBLOCK WSAEWOULDBLOCK

#	if defined(_WIN32_WCE)
#		define NO_STRSTREAM 1
#	endif

#	ifdef __cplusplus
#		ifndef _VIRTUAL_ALLOC_H_
		//#include "VirtualAlloc.h"
#		endif
#	endif

/* Windows definitions of types */
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#elif defined(VXWORKS)
/* VxWorks */
#	include <time.h>
#	include <timers.h>
#	include <sys/times.h>
#	include <sockLib.h>
#	include <hostLib.h>
#	include <resolvLib.h>
#	include <ioLib.h>

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#else
/* Unix */
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <sys/time.h>
#	include <netinet/in.h>
#	include <netinet/ip.h>
#	include <netinet/ip_icmp.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <unistd.h>
#	include <string.h>
#	include <stdlib.h>
#	include <errno.h>
#	include <strings.h>
#	include <ctype.h>
#	if defined(_QNX4)
#		include <unix.h>
#	endif
#	define closeSocket close
#	define socketShutdown(a,b) shutdown(a, b)
#endif


int Network_Initialize();

//C_GetAutomaticSocketPort
// written Feb-March 2007
// Limitations: Make sure the guy who actually use this discovered port use SO_REUSEADDR option
int C_GetAutomaticSocketPort(int * port, int even); // apparently unsafe on linux

int TOOLBOX_CalculateBroadcast(const char * const ip, const char * const netmask, char * loadme_broadcast, int loadme_size);
int TOOLBOX_CalculateNetwork(const char * const ip, const char * const netmask, char * loadme_network, int loadme_size);
int TOOLBOX_AddressNetworkOrderToChar(unsigned long ip, char * loadme, int loadme_size);
int TOOLBOX_isAddressPrivate(char * networkAddress);
int TOOLBOX_isIPV4(const char * networkAddress);
int TOOLBOX_validateSubnet(const char * subnet);
int TOOLBOX_validateIpAndGateway(const char * ip, const char * gateway, const char * subnet);

#endif //__TOOLBOX_NETWORK_H__
