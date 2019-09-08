/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	Header of SocketExampleClient module. Description of the actual module may be found in the source file.
*/

#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 600 //max is (exit+11)+11 msgs per play, and 50 plays per player = 562 < 600
#define MAX_MESSAGE_LEN 256

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"
#include "ProtocolManager.h"
#include "FileManager.h"


HANDLE msg_in_buffer;
SOCKET m_socket;
char cmd_file_path[MAX_LINE_LEN];
FILE *log_fp;
typedef struct _msg_node_
{
	char message[MAX_MESSAGE_LENGTH];
	char raw_message[MAX_MESSAGE_LENGTH];
	struct _msg_node_ *next;
}msg_node;
msg_node *buffer_head;
HANDLE hThread[2];
HANDLE game_ended;
typedef struct _client_params {
	char log_path[MAX_LINE_LEN];
	int port_num;
	char mode[6];
	char file_path[MAX_LINE_LEN];
}client_params;


DWORD MainClient(client_params input_params);
static void insert_to_buffer(char* msg, char* raw_msg);
void free_list();
void clean_exit();


#endif