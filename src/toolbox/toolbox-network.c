#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "sys/stat.h"

#if ( defined(_MSC_VER) )
#	include <io.h>
#else
#	include "unistd.h"
#endif

#include "toolbox.h"
#include "toolbox-basic-types.h" // for uint16_t

#if C_TOOLBOX_NETWORK == 1 
#	include "toolbox-network.h"

char g_localip[24] = "";

// private structures
typedef struct _Network_Config
{
	long timeout_receive;
	long timeout_send;

} Network_Config;

static Network_Config network_config;

// private function
static int initializeWinsockIfNecessary(void);

// TI defines BIG_ENDIAN AND BYTE_ORDER
#if ( defined(BIG_ENDIAN) && defined(BYTE_ORDER) )
#	if ( BIG_ENDIAN != BYTE_ORDER )
#		undef BIG_ENDIAN
#	endif
#endif


int Network_Initialize()
{
	static int network_initialized = 0;

	if ( network_initialized == 0 )
	{
		memset(&network_config, 0, sizeof(Network_Config) );
		network_config.timeout_receive=30000;
		network_config.timeout_send=30000;
		network_initialized = 1;
	}
	
	initializeWinsockIfNecessary();

	return 1;
}

int Network_SetTimeout_Send(long milliseconds)
{
	Network_Initialize();

	network_config.timeout_send = milliseconds;

	return 1;
}
int Network_SetTimeout_Receive(long milliseconds)
{	
	Network_Initialize();

	network_config.timeout_receive = milliseconds;

	return 1;
}


// frank added _MSC_VER 15.Aug.2007 cause on windows XP, visual studio 7.1, WIN32, _WIN32, __WIN32__, _WINNT, WINNT were not defined.
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE) || defined(_MSC_VER)
#	ifndef IMN_PIM
#		define WS_VERSION_CHOICE1 0x202/*MAKEWORD(2,2)*/
#		define WS_VERSION_CHOICE2 0x101/*MAKEWORD(1,1)*/
static int initializeWinsockIfNecessary(void) 
{
	/* We need to call an initialization routine before
	* we can do anything with winsock.  (How fucking lame!):
	*/
	static int _haveInitializedWinsock = 0; // frank fixme do an interlock exchange or proper mutex singleton
	// frank, 13 mar 2006, actually I just use a semaphore around the calls to createNew() network sources.
	WSADATA	wsadata;

	if (!_haveInitializedWinsock)
	{
		if ((WSAStartup(WS_VERSION_CHOICE1, &wsadata) != 0)
			&& ((WSAStartup(WS_VERSION_CHOICE2, &wsadata)) != 0))
		{
			return 0; /* error in initialization */
		}
		if ((wsadata.wVersion != WS_VERSION_CHOICE1)
			&& (wsadata.wVersion != WS_VERSION_CHOICE2))
		{
			WSACleanup();
			return 0; /* desired Winsock version was not available */
		}
		_haveInitializedWinsock = 1;
	}

	return 1;
}
#	else
static int initializeWinsockIfNecessary(void) 
{ 
	return 1;
}
#	endif
#else
static int initializeWinsockIfNecessary(void) 
{ 
	return 1;
}
#endif


int C_GetAutomaticSocketPort(int * port, int even)
{
	int iret;
	struct sockaddr_in receiver;
	uint16_t portNumber = 0;
	//struct sockaddr socketAddr;
	struct sockaddr_in socketAddrIn;
	socklen_t length;
	int socketNumber;
	static int deep = 0;
	if ( port == NULL )
	{
		return -1;
	}

	Network_Initialize();

	socketNumber = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
	if (socketNumber < 0) 
	{
		//error("ERROR opening socket");
		printf("[C_GetAutomaticSocketPort] ERROR -- Cannot create socket.\n");
		return -20;
	}

	receiver.sin_family = AF_INET;
	receiver.sin_addr.s_addr = htonl(INADDR_ANY);
	receiver.sin_port=htons(0);

	iret = bind( socketNumber, (struct sockaddr *) &receiver, sizeof(receiver) );
	if ( iret < 0 )
	{
		// ouch frank fixme
		socketShutdown(socketNumber, 2);
		closeSocket(socketNumber);
		return -21;
	}

	socketAddrIn.sin_port = 0;	
	length = sizeof(socketAddrIn);
	iret = getsockname(socketNumber, (struct sockaddr *) &socketAddrIn, &length);
	if ( iret >=0 ) 
	{
		portNumber = ntohs(socketAddrIn.sin_port);
		if ( even
			&& portNumber%2 == 1 
			&& deep < 1000 )
		{
			deep++;
			//printf("Getting deeper C_GetAutomaticSocketPort\n");
			iret = C_GetAutomaticSocketPort(port, 1);
			if ( iret > 0 )
			{
				//printf("Finally got digging deeper port(%d)\n", *port);
				portNumber = *port;
			}
		}
		deep = 0;
		*port = portNumber;
		socketShutdown(socketNumber, 2);
		closeSocket(socketNumber);
		return 1;
	}
	
	socketShutdown(socketNumber, 2);
	closeSocket(socketNumber);
	return 0;
}

// written 12.Jun.2007
int TOOLBOX_CalculateBroadcast(const char * const ip, const char * const netmask, char * loadme_broadcast, int loadme_size)
{
	int iret;
	unsigned long ip_num;
	unsigned long netmask_num;
	unsigned long broadcast_num;

	if ( ip == NULL )
	{
		return -1;
	}
	if ( netmask == NULL )
	{
		return -2;
	}
	if ( loadme_broadcast == NULL )
	{
		return -3;
	}

	ip_num = inet_addr(ip);
	if ( ip_num==INADDR_NONE )
	{
		return -10;
	}
	netmask_num = inet_addr(netmask);
	if ( netmask_num==INADDR_NONE )
	{
		return -11;
	}

	broadcast_num = ip_num & netmask_num;
	broadcast_num |= ~netmask_num;

	iret = TOOLBOX_AddressNetworkOrderToChar(broadcast_num, loadme_broadcast, loadme_size);

	//printf("calculated %s from 0x%lX\n", loadme_broadcast, broadcast_num);
	if ( iret > 0 )
	{
		return 1;
	}
	return 0;
}

// written 07.May.2009 (based on TOOLBOX_CalculateBroadcast)
int TOOLBOX_CalculateNetwork(const char * const ip, const char * const netmask, char * loadme_network, int loadme_size)
{
	int iret;
	unsigned long ip_num;
	unsigned long netmask_num;
	unsigned long network_num;

	if ( ip == NULL )
	{
		return -1;
	}
	if ( netmask == NULL )
	{
		return -2;
	}
	if ( loadme_network == NULL )
	{
		return -3;
	}

	ip_num = inet_addr(ip);
	if ( ip_num==INADDR_NONE )
	{
		return -10;
	}
	netmask_num = inet_addr(netmask);
	if ( netmask_num==INADDR_NONE )
	{
		return -11;
	}

	network_num = ip_num & netmask_num;
	network_num |= netmask_num;

	iret = TOOLBOX_AddressNetworkOrderToChar(network_num, loadme_network, loadme_size);

	//printf("calculated %s from 0x%lX\n", loadme_network, network_num);
	if ( iret > 0 )
	{
		return 1;
	}
	return 0;
}


int TOOLBOX_AddressNetworkOrderToChar(unsigned long ip, char * loadme, int loadme_size)
{
	struct in_addr addr;
	char * address;
	if ( ip == 0 )
	{
		return -1;
	}
	if ( loadme == NULL )
	{
		return -2;
	}
	if ( loadme_size < 16 )
	{
		return -3;
	}

#if defined(_MSC_VER)
	addr.S_un.S_addr = ip; // not portable 
#else
	addr.s_addr = ip; // not portable 
#endif
	address = inet_ntoa(addr);
	if ( address && address[0] != '\0' )
	{
		strncpy(loadme, address, 15);
		loadme[15]='\0';
		return 1;
	}
	return -10; //not done
}

// written 16.May.2007
int TOOLBOX_isAddressPrivate(char * networkAddress)
{
	unsigned long addressLong;
	if ( networkAddress==NULL )
	{
		return -1;
	}
	
	if ( networkAddress[0] == '\0' )
	{
		return -10; // empty input
	}

	addressLong = inet_addr(networkAddress);

	if ( addressLong==INADDR_NONE )
	{
		struct hostent * host = NULL;
		host=gethostbyname(networkAddress);
		if ( host )
		{
			addressLong = *((unsigned long*)host->h_addr); // frank fixme test with BIG and LITTLE endian.
			//printf("Got address(%u) from hostname(%s)\n", addressLong, networkAddress);
		}
		else
		{
            return -20; // cannot resolve host
		}
	}

#ifdef BIG_ENDIAN
	if ( (addressLong&0xFF000000)>>24 == 10 )
	{
		return 1;
	}
	if ( (addressLong&0xFF000000)>>24 == 127 
	  && (addressLong&0x00FF0000)>>16 >= 16  
	  && (addressLong&0x00FF0000>>16) <= 31 )
	{
		return 1;
	}
	if ( (addressLong&0xFF000000)>>24 == 192 
	  && (addressLong&0x00FF0000)>>16 == 168 )
	{
		return 1;
	}
#else
	if ( (addressLong&0x000000FF) == 10 )
	{
		return 1;
	}
	if ( (addressLong&0x000000FF) == 127 
	  && (addressLong&0x0000FF00)>>8 >= 16  
	  && (addressLong&0x0000FF00)>>8 <= 31 )
	{
		return 1;
	}
	if ( (addressLong&0x000000FF) == 192 
	  && (addressLong&0x0000FF00)>>8 == 168 )
	{
		return 1;
	}
#endif
	
	return 0;
}


// written 25.May.2009
int TOOLBOX_isIPV4(const char * networkAddress)
{
	unsigned long addressLong;
	if ( networkAddress==NULL )
	{
		return -1;
	}
	
	if ( networkAddress[0] == '\0' )
	{
		return -10; // empty input
	}

	addressLong = inet_addr(networkAddress);

	if ( addressLong==INADDR_NONE )
	{
		return -20; // not an ip
	}
	
#ifdef BIG_ENDIAN
	if ( (addressLong&0xFF000000) == 0 )
	{
		return -30;
	}
#else
	if ( (addressLong&0x000000FF) == 0 )
	{
		return -30;
	}
#endif
	
	return 1;
}


// check that netmask is contiguous
// check that netmask is not 32-bit
int TOOLBOX_validateSubnet(const char * subnet)
{
	unsigned long netmask_num;
	int zero_seen = 0;
	int error = 0;
	unsigned int i;

	if ( subnet == NULL )
	{
		return -1;
	}

	netmask_num = ntohl(inet_addr(subnet));

//#ifndef BIG_ENDIAN
//	netmask_num = (netmask_num&0xFF000000)>>24 | (netmask_num&0x00FF0000)>>8 | (netmask_num&0x0000FF00)<<8 | (netmask_num&0x000000FF)<<24;
//#endif

	if ( netmask_num == 0xFFFFFFFF )
	{
		return -10; // don't accept 32 bit netmask
	}

	for (i=31; i>0; i--)
	{
		if ( zero_seen && ((netmask_num&(1<<i)) != 0) )
		{
			// error netmask_num is not contiguous
			error = 1;
		}
		if ( (netmask_num & (1<<i)) == 0 )
		{
			zero_seen = 1;
		}
	}

	if ( error )
	{
		return -11; //netmask is not contiguous
	}

	return 1;
}

int TOOLBOX_validateIpAndGateway(const char * ip, const char * gateway, const char * subnet)
{
	unsigned int ip_num;
	unsigned int gateway_num;
	unsigned int subnet_num;

	if ( ip == NULL ) return -1;
	if ( gateway == NULL ) return -2;
	if ( subnet == NULL ) return -3;

	ip_num = inet_addr(ip);
	gateway_num = inet_addr(gateway);
	subnet_num = inet_addr(subnet);

	if ( ip_num == 0 )
	{
		return -10;
	}
	if ( gateway_num == 0 )
	{
		return -11;
	}

	if ( (ip_num & subnet_num) != (gateway_num & subnet_num) )
	{
		return -20; // both are not on same subnet
	}	

	if ( ip_num == gateway_num )
	{
		return -21; // both are the same
	}

	return 1;
}


#endif //C_TOOLBOX_NETWORK

