/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	Header of SocketExampleServer module. Description of the actual module may be found in the source file.
*/

#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H


#define MAX_LINE_LEN 100
#define PROGRAM_FAILED -1
#define PROGRAM_SUCCEDDED 0

typedef struct _server_params {
	char file_path[MAX_LINE_LEN];
	int port_num;
}server_params;


DWORD MainServer(server_params input_param);
void clean_disconnect(int *Ind);

#endif