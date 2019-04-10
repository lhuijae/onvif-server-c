//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// socket_client.h
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

#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "toolbox.h"

#ifdef __cplusplus
extern "C" {
#endif

// Two case scenario of usage:
//  (i) one is a client opens a socket TCP/IP to a server, and in between request he keeps the socket alive
//  (ii) the second case if when a client send a request, reads the response then the callback function says to close the socket.
//  This leads to:
//    - having to keep the last socket number in the client object.
//    - having a state which says if the socket is keep_alive
// public enum
enum SocketClient_CallbackMessages
{
	GNUCSC_EMPTY = 0,
	GNUCSC_DONT_SEND_RESPONSE,
	GNUCSC_SEND_RESPONSE_CLOSE_SOCKET,
	GNUCSC_SEND_RESPONSE_KEEP_ALIVE
};

#define IS_GNUCSC_SEND_RESPONSE(a) (a==GNUCSC_SEND_RESPONSE_CLOSE_SOCKET||a==GNUCSC_SEND_RESPONSE_KEEP_ALIVE)
#define IS_GNUCSC_CLOSE_SOCKET(a) (a==GNUCSC_SEND_RESPONSE_CLOSE_SOCKET||a==GNUCSC_DONT_SEND_RESPONSE)

typedef int (*callback_client_func)(char * in_response, char * in_opaque, enum SocketClient_CallbackMessages * out_message, char ** out_content_part1, char ** out_content_part2);

#define GNUCSC_RCV_BUFFER_SIZE 65536

// public structures

typedef struct _SocketClient
{
	int initialized;
	int sock_local;
	int sock_remote;
	int keep_alive; // 0 means no keeping alive
					// 1 means user wants to keep alive
	int keep_alive_status;	// 0 means current socket not alive
							// 1 means current socket is still alive
	struct sockaddr_in sockaddr;
	callback_client_func callback;
	int blocking;
	void * opaque;
	int endrun; // set to 1 and above to signal program termination
	char rcvBuffer[GNUCSC_RCV_BUFFER_SIZE+5000];
	char result_message[256]; // for writing error message when return value is negative.
	int beefeater;
} SocketClient;

extern int g_debug_level;

int SocketClient_SetTimeout_Send(long milliseconds);
int SocketClient_SetTimeout_Receive(long milliseconds);

int SocketClient_Constructor(SocketClient * root, char * in_address_or_hostname, int port, int blocking, void * opaque, char ** out_result_message);
int SocketClient_Destructor(SocketClient * root);
int SocketClient_reConnect(SocketClient * root);
int SocketClient_SignalTermination(SocketClient * root);

// opaque is sent to client when callback_func is called.
int SocketClient_send(SocketClient * root, callback_client_func callback, char * in_request, char ** out_result_message);
int SocketClient_FileTransfer(SocketClient * root, callback_client_func callback, char * in_request, char ** out_result_message, FILE * dst_file, unsigned int * kbytes_progress); // frank warning not 64 bit os compliant.

#ifdef __cplusplus
}
#endif

#endif // __SOCKET_CLIENT_H__

