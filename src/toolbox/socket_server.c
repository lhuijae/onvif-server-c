//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// socket_server.c
// 
// Copyright (c) 2006-2008 HaiVision System Inc.	
//
//    Written by : Francois Oligny-Lemieux
//       Created : 17.Nov.2006
//      Modified : (see .h)
//
//  Description: 
//      TCP/IP server. Has been used to implement a RTSP server, a telnet server, a HTTP server and for
//      inter-process communications.
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
#include "../filelog.h"
#include "toolbox-line-parser.h"
#include "toolbox-basic-types.h"

#if C_TOOLBOX_NETWORK == 1 
#	include "toolbox-network.h"
#	if GNUC_TOOLBOX_SOCKET_SERVER == 1
#		include "socket_server.h"



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#	include <io.h> // instead of unistd.h
#	define C_Sleep(x) Sleep(x)
#else
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <sys/time.h>
#	include <sys/ioctl.h>
#	include <sys/select.h>
//#	include "busybox.h"
#	ifndef SOCKET_ERROR
#		define SOCKET_ERROR -1
#	endif
#	define C_Sleep(x) usleep(x*1000)
#endif

#define SOCKET_SERVER_INTERNAL_RESULT_BUFFER_SIZE 512*1024

// private structures
typedef struct _socket_server_config
{
	long timeout_receive;
	long timeout_send;
	long use_keepalive; // 0 or 1

} socket_server_config;

static socket_server_config ssconfig;

int socketServer_Constructor(socketServer_T * root, int port, int blocking, void * opaque, char ** out_result_message)
{
	struct timeval timeout;
	struct sockaddr_in server;
	int sock_listen;
	int value = 0;
	socklen_t length = sizeof(int);
	int iret;
	const int one = 1;

	if ( root == NULL )
	{
		return -1;
	}

	memset(root, 0, sizeof(socketServer_T));
	memset(&server, 0, sizeof(server));
	
	root->result_message[0] = '\0';
	
	if ( out_result_message )
	{
		*out_result_message = NULL;
		*out_result_message = &root->result_message[0];
	}
		
	Network_Initialize();
	
	root->blocking = blocking;

	root->sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	sock_listen = root->sock_listen;
	root->opaque = opaque;
	
	if (sock_listen < 0) 
	{
		// ERROR opening socket
		printf("<!-- ERROR cannot create socket -->\n");
		return -20;
	}

	if ( ssconfig.timeout_receive != 0 )
	{
		timeout.tv_sec  = 0;
		timeout.tv_usec = 1000 * ssconfig.timeout_receive; // 1000 * ms
		setsockopt(sock_listen, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));	
	}
	
	if ( ssconfig.timeout_send != 0 )
	{
		timeout.tv_sec  = 0;
		timeout.tv_usec = 1000 * ssconfig.timeout_send; // 1000 * ms
		setsockopt(sock_listen, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	}

	if ( ssconfig.use_keepalive != 0 )
	{
		setsockopt(sock_listen, SOL_SOCKET, SO_KEEPALIVE, (char*)&one, sizeof(one));
	
		if ( getsockopt(sock_listen, SOL_SOCKET, SO_KEEPALIVE, (char*)&value, &length) != SOCKET_ERROR )
		{
			printf("SO_KEEPALIVE value: %ld.\n", value);
		}
	}

	
	// dont reuse, we are a server, we need absolute port control
	//setsockopt(sock_listen,SOL_SOCKET,SO_REUSEADDR, (char*)&one, sizeof(one));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	iret = bind(sock_listen, (struct sockaddr *)&server, sizeof(server));

	if(iret < 0)
	{
		closeSocket(sock_listen);
		printf("<!-- socketServer_Constructor bind returned iret(%d) -->\n", iret);
		return -31;
	}

	root->initialized = 1;
	root->sockaddr = server;

	return 1;
}

int socketServer_Destrutor(socketServer_T * root)
{
	if ( root == NULL )
	{
		return -1;
	}
	
	return 1;
}

// This function does not return
int socketServer_listen(socketServer_T * root, callback_func callback, idle_callback_func callback_idle, close_callback_func callback_close, int timeout_ms, char ** out_result_message)
{
	int iret=0;
	ss_client_T * clients = &root->clients[0];
	int sock_listen;
	int i;
	int amount_served;
	int ival;
	socklen_t length = 0;
	char content[65536];
	struct sockaddr_in particular_adr;
	lineParser parser;
	unsigned int blength = 0;
	int callbackMessage = GNUCSS_EMPTY;
	const char * insider;
	const char * insider2;
	const char * insider3;
	struct timeval timeout;
	char temp[256];

#if defined(_MSC_VER)
	FD_SET write_handles;
	FD_SET read_handles;
#else
	fd_set write_handles;
	fd_set read_handles;
#endif

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	temp[255] = '\0';

	if ( timeout_ms > 0 )
	{
		timeout.tv_sec = timeout_ms/1000;
		timeout.tv_usec = (timeout_ms%1000)*10;
	}

	if ( root == NULL )
	{
		return -1;
	}
		
	if ( root->initialized == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( root->sock_listen == 0 )
	{
		return TOOLBOX_ERROR_SOCKET_HANDLE_IS_NULL;
	}
	
	sock_listen = root->sock_listen;

	FD_ZERO(&read_handles);
	FD_ZERO(&write_handles);
	memset(&particular_adr,0,sizeof(particular_adr));
	memset(&content[0], 0, 65536); 
	iret = lineParser_Constructor(&parser);
	if ( iret <= 0 )
	{
		printf("[SocketServer] (listen) lineParser_Constructor failed, iret(%d)\n", iret);
		return -11;
	}
	
	FD_SET(sock_listen, &read_handles);
	
	if ( listen(sock_listen, 5) )
	{
		#if ( defined(_MSC_VER) )
		printf("[SocketServer] (listen) listen failed WSAGetLastError(%d)\n", WSAGetLastError());
		#else
		printf("[SocketServer] (listen) listen failed\n");
		#endif
		lineParser_Destructor(&parser);
		return -12;
	}

	if ( root->blocking == 0 )
	{
#if defined(_MSC_VER)		
		unsigned long nonblocking = 1;
		if ( ioctlsocket(sock_listen, FIONBIO, &nonblocking) == SOCKET_ERROR )
		{
			printf("<!-- ioctlsocket() failed -->\n");
			lineParser_Destructor(&parser);
			return -32;
		}
#else		
		lineParser_Destructor(&parser);
		return 0; //not implemented
#endif
	}

	root->callback = callback;
	root->callback_idle = callback_idle;
	root->callback_close = callback_close;

	while(1)
	{
first_while_loop:
		FD_ZERO(&read_handles);

		if ( FD_ISSET(sock_listen, &read_handles)==0 )
		{
			if ( sock_listen )
			{
                FD_SET(sock_listen, &read_handles);
			}
			else
			{
				printf("[SocketServer] (listen) ERROR sock_listen is NULL, exiting.\n");
				break;
			}
		}
		
		if ( root->rcv_from_this_guy_again == NULL )
		{
			i=0;
			while (i<GNUCSS_MAX_CLIENT)
			{
				if ( clients[i].socket )
				{
					FD_SET(clients[i].socket, &read_handles);
				}
				i++;
			}
		}
		else
		{
			if ( root->rcv_from_this_guy_again->socket )
			{
				FD_SET(root->rcv_from_this_guy_again->socket, &read_handles);
			}
			else
			{
				root->rcv_from_this_guy_again = NULL;
				continue; // don't waste time doing a timeout, restart listening from everybody
			}
		}

		ival = select(80 /*ignored number*/, &read_handles, NULL, NULL, &timeout);

#if ( defined(_MSC_VER) )
		if ( ival<0 && WSAEINPROGRESS==WSAGetLastError() )
		{
			C_Sleep(1);
			continue;
		}
#else
		if (0)
		{
		}
#endif
		else if ( ival<0 )
		{
			printf("[SocketServer] (listen) select returned negative (%d), exiting.\n", ival);
			break;
		}

		if ( root->endrun )
		{
			printf("[SocketServer] (listen) detected endrun.\n");
			break;
		}

		if ( FD_ISSET(sock_listen, &read_handles) )
		{
			int new_socket = 0;
			// accept connection from client
			length = sizeof(struct sockaddr);
			memset(&particular_adr, 0, sizeof(struct sockaddr) );
			
			i=0;
			while (i<GNUCSS_MAX_CLIENT)
			{
				if ( clients[i].socket == 0 )
				{
					break;
				}
				i++;
			}
			if ( i == GNUCSS_MAX_CLIENT )
			{
				// failure
			}

			new_socket = accept(sock_listen, (struct sockaddr *) &particular_adr, &length);

			if ( i == GNUCSS_MAX_CLIENT )
			{
				closeSocket(new_socket); // reject incoming call
			}
			else if ( new_socket > 0 )
			{
				// clear sock_listen to prevent handling new connections.
				// no dont FD_CLR(sock_listen, &read_handles);
				clients[i].socket = new_socket;
				clients[i].sockaddr = particular_adr;
				FD_SET(clients[i].socket, &read_handles);

				blength = sizeof( root->sockaddr );
				// get my identification

				iret = getsockname(clients[i].socket, (struct sockaddr *) &root->sockaddr, &blength);

				// get client info
				iret = getpeername(clients[i].socket, (struct sockaddr *) &root->sockaddr, &blength);
			}
			else
			{
				// frank fixme accept failed.
			}

			continue;
		}

		if ( root->async_message_required == 1 )
		{
			root->async_message_required = 2;
			
			i=0;
			while (i<GNUCSS_MAX_CLIENT)
			{
				if ( clients[i].socket )
				{
					FD_SET(clients[i].socket, &read_handles);
					clients[i].async_needed = 1;
				}
				i++;
			}
		}

		amount_served = 0;
		i=0;
		while (i<GNUCSS_MAX_CLIENT)
		{
			if ( FD_ISSET(clients[i].socket, &read_handles) )
			{
				memset(&content[0], 0, 65536);

				if ( clients[i].async_needed == 1 )
				{
					 clients[i].async_needed = 0;
					 goto sendto_tag;
				}

				length = sizeof(struct sockaddr);
				iret = recvfrom(clients[i].socket, &content[0], 65536 - 1, 0, (struct sockaddr *)&particular_adr, &length);
			
				if ( iret == 0 )
				{
					if ( callback_close )
					{
						(callback_close)(root->opaque, &clients[i], &callbackMessage);
					}

					// connection gracefully closed
					FD_CLR(clients[i].socket, &read_handles);
					closeSocket(clients[i].socket);
					clients[i].socket = 0;
					memset(&clients[i].sockaddr, 0, sizeof(clients[i].sockaddr));
					snprintf(temp, 255, "[SocketServer] (listen) Connection closed by remote party\n");
					fputs(temp, stdout);
					Log_C(temp, 0);
					goto loop_recv_clients_continue;
				}
				
				if ( iret < 0 )
				{
					if ( callback_close )
					{
						(callback_close)(root->opaque, &clients[i], &callbackMessage);
					}
					// error
#if ( defined(_MSC_VER) )
					iret = WSAGetLastError();
					//if ( iret == WSAEINPROGRESS )
					//{
						// not a error from us, try again? no. lets fail.
					//}
					//else
					{
						printf("[SocketServer] (listen) recvfrom got length(%d) and WSAGetLastError == (%d)\n", length, iret);
					}
#else
					printf("[SocketServer] (listen) recvfrom got length(%d) and returned iret(%d)\n", length, iret);
#endif
					FD_CLR(clients[i].socket, &read_handles);
					closeSocket(clients[i].socket);
					clients[i].socket = 0;
					
					goto loop_recv_clients_continue;
				}

				snprintf(temp, 255, "[SocketServer] (listen) recvfrom got messageLength(%u), returned iret(%d) and got:\n", strlen(content), iret);
				Log_C(temp, 0);
				Log_C(content, 0);

				// Chapter 1.2 -- reading response.
				// send answer to client, do I send a response ?

sendto_tag:
				if ( root->callback )
				{
					insider = NULL;
					insider2 = NULL;
					insider3 = NULL;
					callbackMessage = 0;
					if ( root->async_message_required == 2 )
					{	
						callbackMessage = GNUCSS_SEND_ASYNC_MESSAGE; 
					}
					iret = (root->callback)(root->opaque, &content[0], iret, &clients[i], &callbackMessage, &insider, &insider2, &insider3 /*filename*/);

					if ( callbackMessage&GNUCSS_REQUEST_TO_SEND_ASYNC_MESSAGE )
					{
						root->async_message_required = 1;
					}
					if ( callbackMessage&GNUCSS_RECV_FROM_THIS_GUY_AGAIN )
					{
						root->rcv_from_this_guy_again = &clients[i];
						// receive again from this guy
						timeout.tv_sec = 10;
						timeout.tv_usec = 500*1000; //ms*1000;
						goto first_while_loop;
					}

					if ( iret > 0 )
					{
						if ( IS_GNUCSS_SEND_RESPONSE(callbackMessage) )
						{
							if ( insider )
							{
								ival = sendto(clients[i].socket, insider, strlen(insider), 0,
									(struct sockaddr *)&particular_adr,
									sizeof(struct sockaddr));

								snprintf(temp, 255, "Sending response iret(%d):\n%s", iret, insider);
								Log_C(temp, 0);
							}

							if ( insider2 )
							{
								ival = sendto(clients[i].socket, insider2, strlen(insider2), 0,
									(struct sockaddr *)&particular_adr,
									sizeof(struct sockaddr));
							}
							else if ( insider3 )
							{
								FILE * source;
								unsigned char buffer[1024];

								source = fopen(insider3, "rb");
								if ( source )
								{
									while ( 1 )
									{
										iret = fread(buffer, 1, 1024, source);
										if ( iret > 0 )
										{
											ival = sendto(clients[i].socket, buffer, iret, 0,
												(struct sockaddr *)&particular_adr,
												sizeof(struct sockaddr));
										}
										else
										{
											break;
										}
									}
									fclose(source);
								}
								else if ( ! IS_GNUCSS_CLOSE_SOCKET(callbackMessage) )
								{
									// we are probably going to inform a Content-Length bigger than actual which will result in blocking on the remote(client) side.
									callbackMessage = GNUCSS_SEND_RESPONSE | GNUCSS_CLOSE_SOCKET;
								}
							}
						}
						if ( IS_GNUCSS_CLOSE_SOCKET(callbackMessage) )
						{
							FD_CLR(clients[i].socket, &read_handles);
							closeSocket(clients[i].socket);
							clients[i].socket = 0;
							timeout.tv_sec = 1;
							timeout.tv_usec = 0;
						}
						else
						{	// receive again from this guy
							timeout.tv_sec = 0;
							timeout.tv_usec = 500*1000; //ms*1000;
						}
					}
					else
					{
						// app failed, so we must handle it
						FD_CLR(clients[i].socket, &read_handles);
						closeSocket(clients[i].socket);
						clients[i].socket = 0;
						timeout.tv_sec = 1;
						timeout.tv_usec = 0;
					}
				}
			}//FD_ISSET( sock_client )
loop_recv_clients_continue:;
			i++;
		}//while (i<GNUCSS_MAX_CLIENT)

		if ( root->async_message_required == 2 )
		{
			root->async_message_required = 0;
			if ( root->callback )
			{
				insider = NULL;
				insider2 = NULL;
				insider3 = NULL;
				callbackMessage = GNUCSS_ASYNC_COMPLETED;
				content[0] = '\0';
				iret = (root->callback)(root->opaque, &content[0], 0, &clients[i], &callbackMessage, &insider, &insider2, &insider3 /*filename*/);
			}
		}

		if ( amount_served == 0 )
		{
			// timeout

			if ( root->rcv_from_this_guy_again )
			{	root->rcv_from_this_guy_again = NULL; // last chance
			}

			if ( root->callback_idle )
			{
				callbackMessage = 0;
				iret = (root->callback_idle)(root->opaque, &callbackMessage);
				
				if ( callbackMessage&GNUCSS_REQUEST_TO_SEND_ASYNC_MESSAGE )
				{
					root->async_message_required = 1;
				}

				if ( iret > 0 )
				{
				}
			}
		}
	}//while(1)

	lineParser_Destructor(&parser);
	
	return 1;
}


int socketServer_SetTimeout_Send(long milliseconds)
{
	ssconfig.timeout_send = milliseconds;

	return 1;
}

int socketServer_SetTimeout_Receive(long milliseconds)
{
	ssconfig.timeout_receive = milliseconds;

	return 1;
}

int socketServer_Enable_KeepAlive()
{
	ssconfig.use_keepalive = 1;

	return 1;
}


#	endif
#endif
