//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// socket_server.h
// 
// Copyright (c) 2006-2008 HaiVision System Inc.	
//
//    Written by : Francois Oligny-Lemieux
//       Created : 17.Nov.2006 (in NightFlight)
//      Modified : 28.Jan.2008 (fixed a fclose on insider3 which resulting in exhausting filesystem resources)
//
//  Description: 
//      TCP/IP server. Has been used to implement a RTSP server, a telnet server, a HTTP server and for
//      inter-process communications.
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

#include "toolbox-config.h"

#if C_TOOLBOX_SOCKET_SERVER == 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "toolbox-network.h"

// public structures
typedef struct ss_client_S
{
	int socket;
	int async_needed;
	struct sockaddr_in sockaddr;
} ss_client_T;

// public enum
enum socketServer_CallbackMessages
{
	GNUCSS_EMPTY = 0,
	GNUCSS_DONT_SEND_RESPONSE = 1,
	GNUCSS_SEND_RESPONSE = 2,
	GNUCSS_CLOSE_SOCKET = 4,
	GNUCSS_KEEP_ALIVE = 8,
	GNUCSS_REQUEST_TO_SEND_ASYNC_MESSAGE = 16, // will call every client's callback_func with empty message to allow them to send something on the socket.
	GNUCSS_SEND_ASYNC_MESSAGE = 32, // will call every client's callback_func with empty message to allow them to send something on the socket.
	GNUCSS_ASYNC_COMPLETED = 64, // socket_server will send this to callback
	GNUCSS_RECV_FROM_THIS_GUY_AGAIN = 128
};

#define GNUCSS_MAX_CLIENT 31

#define IS_GNUCSS_SEND_RESPONSE(a) ( a&GNUCSS_SEND_RESPONSE )
#define IS_GNUCSS_CLOSE_SOCKET(a)  ( a&GNUCSS_CLOSE_SOCKET )
#define IS_GNUCSS_KEEPALIVE(a)     ( a&GNUCSS_KEEP_ALIVE )


typedef int (*callback_func)(void * in_opaque, char * in_request, int in_length, ss_client_T * client, int * out_message, const char ** out_content_part1, const char ** out_content_part2, const char ** out_filename);
typedef int (*idle_callback_func)(void * in_opaque, int * out_message);
typedef int (*close_callback_func)(void * in_opaque, ss_client_T * client, int * out_message);


// public structures
typedef struct socketServer_S
{
	int initialized;
	int sock_listen;
	ss_client_T clients[GNUCSS_MAX_CLIENT];
	struct sockaddr_in sockaddr;
	callback_func callback;
	idle_callback_func callback_idle;
	close_callback_func callback_close;
	int blocking;
	void * opaque;
	int async_message_required;
	ss_client_T * rcv_from_this_guy_again;
	int endrun; // set to 1 and above to signal program termination
	char result_message[256]; // for writing error message when return value is negative.
	int beefeater;
} socketServer_T;

extern int g_debug_level;


int socketServer_Constructor(socketServer_T * root, int port, int blocking, void * opaque, char ** out_result_message);
int socketServer_Destrutor(socketServer_T * root);
// opaque is sent to client when callback_func is called.
int socketServer_listen(socketServer_T * root, callback_func callback, idle_callback_func callback_idle, close_callback_func callback_close, int timeout_ms, char ** out_result_message);

int socketServer_SetTimeout_Send(long milliseconds);
int socketServer_SetTimeout_Receive(long milliseconds);
int socketServer_Enable_KeepAlive();

#ifdef __cplusplus
}
#endif

#endif // GNUC_TOOLBOX_SOCKET_SERVER

#endif // __SOCKET_SERVER_H__
