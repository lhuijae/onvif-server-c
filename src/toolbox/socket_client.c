//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// socket_client.c
// 
// Copyright (c) 2006-2008 HaiVision System Inc.	
//
//    Written by : Francois Oligny-Lemieux
//       Created : 11.Dec.2006 (based on http_client.h)
//      Modified : 
//
//  Description: 
//      TCP/IP client.
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

#include "toolbox-config.h"

#if C_TOOLBOX_NETWORK == 1 
#	include "toolbox-network.h"
#	if GNUC_TOOLBOX_SOCKET_CLIENT == 1
#		include "socket_client.h"


#define SOCKET_SERVER_INTERNAL_RESULT_BUFFER_SIZE 512*1024

// private structures
typedef struct _socket_client_config
{
	long timeout_receive;
	long timeout_send;

} socket_client_config;

static socket_client_config scconfig;

int SocketClient_Constructor(SocketClient * root, char * in_address_or_hostname, int port, int blocking, void * opaque, char ** out_result_message)
{
	struct timeval timeout;
	struct sockaddr_in client;
	int sock_local;
	struct hostent * host = NULL;
	unsigned int addr=0;
	int iret;

	if ( root == NULL )
	{
		return -1;
	}

	memset(root, 0, sizeof(SocketClient));
	memset(&client, 0, sizeof(client));
	
	root->result_message[0] = '\0';
	
	if ( out_result_message )
	{
		*out_result_message = NULL;
		*out_result_message = &root->result_message[0];
	}
	
	if ( port <= 0 || port > 0xFFFF )
	{
		snprintf(root->result_message, 255, "[SocketClient_Constructor] Asked port number invalid, port(%d)", port);
		return -3;
	}
	
	Network_Initialize();
	
	root->blocking = blocking;
	
	if( inet_addr(in_address_or_hostname)==INADDR_NONE )
	{
		host=gethostbyname(in_address_or_hostname);
	}
	else
	{
		addr=inet_addr(in_address_or_hostname);
		//host=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	}
	if( host==NULL && addr==0 )
	{
		return -20; // cannot figure out remote host
	}

	root->sock_local = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	sock_local = root->sock_local;
	root->opaque = opaque;
	
	if (sock_local < 0) 
	{
		//error("ERROR opening socket");
		printf("<!-- cannot create socket -->\n");
		snprintf(root->result_message, 255, "[SocketClient_Constructor] Cannot create socket");
		return TOOLBOX_ERROR_CANNOT_CREATE_SOCKET;
	}

	if ( scconfig.timeout_receive != 0 )
	{
		timeout.tv_sec  = 0;
		timeout.tv_usec = 1000 * scconfig.timeout_receive; // 1000 * ms
		setsockopt(sock_local, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));	
	}
	
	if ( scconfig.timeout_send != 0 )
	{
		timeout.tv_sec  = 0;
		timeout.tv_usec = 1000 * scconfig.timeout_send; // 1000 * ms
		setsockopt(sock_local, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	}

	client.sin_family = AF_INET;
	if ( host )
	{
        client.sin_addr.s_addr=*((unsigned long*)host->h_addr);
	}
	else
	{
        client.sin_addr.s_addr=(unsigned long)addr;
	}
	client.sin_family=AF_INET;
	client.sin_port=htons(port);

	//printf("going to connect\n");
	iret = connect(sock_local, (struct sockaddr *)&client, sizeof(client));

	//printf("connect returned iret(%d)\n", iret);
	if (iret == -1)
	{
		closeSocket(sock_local);
		printf("<!-- SocketClient_Constructor connect returned iret(%d) -->\n", iret);
		return -30;
	}
	else if ( iret < 0 )
	{
		closeSocket(sock_local);
		printf("<!-- SocketClient_Constructor connect returned iret(%d) -->\n", iret);
		return -31;
	}
	
	root->initialized = 1;
	root->sockaddr = client;

	return 1;
}


int SocketClient_Destructor(SocketClient * root)
{
	if ( root == NULL )
	{
		return -1;
	}

	SocketClient_SignalTermination(root);

	return 1;

}

int SocketClient_reConnect(SocketClient * root)
{
	struct timeval timeout;
	struct sockaddr_in client;
	int sock_local;
	struct hostent * host = NULL;
	unsigned int addr=0;
	int iret;

	if ( root == NULL )
	{
		return -1;
	}

	if ( root->sock_local == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( root->initialized == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
		
	closeSocket(root->sock_local);
	root->sock_local = 0;

	root->sock_local = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	sock_local = root->sock_local;	
	memcpy(&client, &root->sockaddr, sizeof(client));
	
	if (sock_local < 0) 
	{
		//error("ERROR opening socket");
		printf("<!-- cannot create socket -->\n");
		snprintf(root->result_message, 255, "[SocketClient_reConnect] Cannot create socket");
		return TOOLBOX_ERROR_CANNOT_CREATE_SOCKET;
	}

	if ( scconfig.timeout_receive != 0 )
	{
		timeout.tv_sec  = 0;
		timeout.tv_usec = 1000 * scconfig.timeout_receive; // 1000 * ms
		setsockopt(sock_local, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));	
	}
	
	if ( scconfig.timeout_send != 0 )
	{
		timeout.tv_sec  = 0;
		timeout.tv_usec = 1000 * scconfig.timeout_send; // 1000 * ms
		setsockopt(sock_local, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	}

	//printf("going to connect\n");
	iret = connect(sock_local, (struct sockaddr *)&client, sizeof(client));

	//printf("connect returned iret(%d)\n", iret);
	if(iret == -1)
	{
		closeSocket(sock_local);
		printf("<!-- SocketClient_reConnect connect returned iret(%d) -->\n", iret);
		return -30;
	}
	else if( iret < 0 )
	{
		closeSocket(sock_local);
		printf("<!-- SocketClient_reConnect connect returned iret(%d) -->\n", iret);
		return -31;
	}
	
	root->sockaddr = client;

	return 1;
}


int SocketClient_SignalTermination(SocketClient * root)
{
	if ( root == NULL ) 
	{
		return -1;
	}

	root->endrun = 1;
	return 1;
}


int SocketClient_send(SocketClient * root, callback_client_func callback, char * in_request, char ** out_result_message)
{
	int iret=0;
	int sock_local;
	int sock_remote = 0;
	//int i;
	int ii;
	int length=0;
//	char content[65536];
	//char output_buffer[65536];
	//struct sockaddr_in particular_adr;
	//lineParser parser;
	unsigned int ulength = 0;
	enum SocketClient_CallbackMessages callbackMessage = GNUCSC_EMPTY;
	char * rcvBuffer;
	char * strRet;
	char * rcvBuffer_ptr;
	char * insider = NULL;
	char * insider2 = NULL;
	struct timeval timeout;
	unsigned int total_received=0;

#if defined(_MSC_VER)
//	FD_SET write_handles;
	FD_SET read_handles;
#else
//	fd_set write_handles;
	fd_set read_handles;
#endif

	timeout.tv_sec = 9;
	timeout.tv_usec = 0;

	FD_ZERO(&read_handles);

	if ( root == NULL )
	{
		return -1;
	}

	if ( root->initialized == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( root->sock_local == 0 )
	{
		return TOOLBOX_ERROR_SOCKET_HANDLE_IS_NULL;
	}
	
	if ( in_request == NULL )
	{
		return -3;
	}

	sock_local = root->sock_local;
	rcvBuffer = root->rcvBuffer;
	
	//memset(packet, 0, 5000);
	//memset(r, 0, 5000);
	memset(rcvBuffer, 0, GNUCSC_RCV_BUFFER_SIZE+5000);

	//printf("going to sendto\n");
	
	//iret = sendto(sockfd, packet, strlen(packet), 0, (struct sockaddr *)&server, sizeof(struct sockaddr));
	iret = send(sock_local, in_request, strlen(in_request), 0 );
	if ( iret <= 0 )
	{
		closeSocket(sock_local);
		root->sock_local = NULL;
		return -30;
	}

	FD_SET(sock_local, &read_handles);
	iret = select(80 /*ignored number*/, &read_handles, NULL, NULL, &timeout);

	if ( iret == 0 && FD_ISSET(sock_local, &read_handles) == 0 )
	{
		// timeout.
		closeSocket(sock_local);
		root->sock_local = NULL;
		return -40;
	}
    
	rcvBuffer_ptr = &rcvBuffer[0];

	iret = 0;

	if ( root->keep_alive )
	{	
		ii = recv(sock_local, rcvBuffer_ptr, 512, 0); // frank fixme do a timeout or select()
		iret += ii;
		while (ii > 0 && ii <= 512 )
		{
			rcvBuffer_ptr[ii] = '\0';
			total_received += (unsigned int) ii;

			strRet = strstr(rcvBuffer, "\r\n");
			if ( strRet )
			{
				break;
			}
			
			if ( root->endrun )
			{
				closeSocket(sock_local);
				root->sock_local = NULL;
				return TOOLBOX_ERROR_TERMINATED;
			}
                    
			if ( total_received < GNUCSC_RCV_BUFFER_SIZE )
			{	rcvBuffer_ptr += ii;
			}
			ii = recv(sock_local,rcvBuffer_ptr,512,0);
			iret += ii;
		}
	}
	else
	{
		ii = recv(sock_local, rcvBuffer_ptr, 512, 0); // frank fixme do a timeout or select()
		iret += ii;
		while (ii > 0 && ii <= 512 )
		{
			rcvBuffer_ptr[ii] = '\0';
			//printf("%s\n",r);
			total_received += (unsigned int) ii;
			if ( total_received < GNUCSC_RCV_BUFFER_SIZE )
			{	rcvBuffer_ptr += ii;
			}

			if ( root->endrun )
			{
				closeSocket(sock_local);
				root->sock_local = NULL;
				return TOOLBOX_ERROR_TERMINATED;
			}

			ii = recv(sock_local,rcvBuffer_ptr,512,0);
			iret += ii;
		}
	}

	//printf("total size(%d)\n",iret);

	if ( iret == -1 )
	{
		C_Sleep(400);
		ii = recv(sock_local,rcvBuffer_ptr,512,0);
		if ( ii > 0 )
		{
			printf("<!-- Second chance success : qty_received(%d) -->\n", ii);
		}
		else
		{
			printf("<!-- Second chance failed : recv iret(%d) -->\n", ii);
		}
	}

	if ( out_result_message ) *out_result_message = &rcvBuffer[0];

	iret = (callback)(&rcvBuffer[0], (char*)root->opaque, &callbackMessage, &insider, &insider2);

	if ( root->keep_alive == 0 )
	{
		closeSocket(sock_local);
	}
	else
	{
		root->keep_alive_status = 1;
	}

	return 1;
}

int SocketClient_FileTransfer(SocketClient * root, callback_client_func callback, char * in_filename, char ** out_result_message, FILE * dst_file, unsigned int * kbytes_progress)
{
	int iret=0;
	int sock_local;
	int sock_remote = 0;
	int success = 0;
	int ii;
	int length=0;
	unsigned int ulength = 0;
	enum SocketClient_CallbackMessages callbackMessage = GNUCSC_EMPTY;
	char sndBuffer[1024];
	char rcvBuffer[GNUCSC_RCV_BUFFER_SIZE+5000];
	char * rcvBuffer_ptr;
	char * insider = NULL;
	char * insider2 = NULL;
	struct timeval timeout;
	unsigned int total_received_b = 0;
	unsigned int total_received_kb = 0;
	char * strRet;

	sndBuffer[0] = '\0';
	sndBuffer[1023] = '\0';

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	if ( root == NULL )
	{
		return -1;
	}
	
	if ( root->initialized == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( root->sock_local == 0 )
	{
		return TOOLBOX_ERROR_SOCKET_HANDLE_IS_NULL;
	}
	
	if ( in_filename == NULL )
	{
		return -3;
	}

	if ( strlen(in_filename) >= 1000 )
	{
		return TOOLBOX_ERROR_YOU_PASSED_A_FILENAME_TOO_BIG;
	}

	sock_local = root->sock_local;
	if ( kbytes_progress ) *kbytes_progress = 0;
	
	memset(rcvBuffer, 0, GNUCSC_RCV_BUFFER_SIZE+5000);
	
	strcpy(sndBuffer, "DOWNLOAD -f \"");
	strcat(sndBuffer, in_filename);
	strcat(sndBuffer, "\"");

	iret = send(sock_local, sndBuffer, strlen(sndBuffer), 0);
	if ( iret <= 0 )
	{
		// failed 
		return TOOLBOX_ERROR_SOCKET_SEND;
	}

	rcvBuffer_ptr = &rcvBuffer[0];

	iret = 0;
	ii = recv(sock_local,rcvBuffer_ptr,512,0); // frank fixme do a timeout or select()

	if ( ii > 0 )
	{
		strRet = strstr(rcvBuffer, "\r\n");
		if ( strRet )
		{
			*strRet = '\0';
			if ( rcvBuffer[0] != '\0'
			  && strstr(rcvBuffer, "ERROR")==0 
			  && strstr(rcvBuffer, "Success")!=0
			   )
			{
				// ok, save file
				ii -= strlen(rcvBuffer) + 2;
				total_received_b = ii;
				fwrite(strRet+2, 1, ii, dst_file);
				
				if ( kbytes_progress && ii > 0 ) *kbytes_progress = 1; // in fact it is < 512bytes
			
				iret = 0;
				iret = recv(sock_local, rcvBuffer, 512, 0); // frank fixme do a timeout or select()
				
				while ( iret > 0 )
				{
					//printf("%s\n",r);
					total_received_b += iret;
	
					fwrite(rcvBuffer, 1, iret, dst_file);
					if ( total_received_b > 1024 )
					{
						total_received_kb += total_received_b/1024;
						total_received_b = total_received_b%1024;
						if ( kbytes_progress ) *kbytes_progress = total_received_kb;
					}

					if ( root->endrun )
					{
						closeSocket(sock_local);
						root->sock_local = NULL;
						return TOOLBOX_ERROR_TERMINATED;
					}
                    
					iret = recv(sock_local, rcvBuffer, 512, 0);
				}
			}
		}
	}
	
	// frank fixme do you need to prevent calling callback during termination ??

	iret = (callback)(&rcvBuffer[0], (char*)root->opaque, &callbackMessage, &insider, &insider2);

	if ( root->keep_alive == 0 )
	{
		closeSocket(sock_local);
	}
	else
	{
		root->keep_alive_status = 1;
	}

	return 1;
}


int SocketClient_SetTimeout_Send(long milliseconds)
{
	//if ( scconfig.timeout_receive == 0 )
	//{
	//	return TOOLBOX_ERROR_NOT_INITIALIZED;
	//}

	scconfig.timeout_send = milliseconds;

	return 1;
}

int SocketClient_SetTimeout_Receive(long milliseconds)
{	
	//if ( scconfig.timeout_receive == 0 )
	//{
	//	return TOOLBOX_ERROR_NOT_INITIALIZED;
	//}

	scconfig.timeout_receive = milliseconds;

	return 1;
}


#	endif
#endif
