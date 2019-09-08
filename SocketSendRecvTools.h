/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	Header of SendRecvTools module. Description of the actual module may be found in the source file.
*/

#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H


#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")


typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;


TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd);

TransferResult_t SendString(const char *Str, SOCKET sd);

TransferResult_t ReceiveBuffer(char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd);

TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd);


#endif