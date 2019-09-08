/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	Header of SocketExampleShared module. Description of the actual module may be found in the source file.
*/

#pragma warning(disable:4996)

#ifndef SOCKET_EXAMPLE_SHARED_H
#define SOCKET_EXAMPLE_SHARED_H



#define SERVER_ADDRESS_STR "127.0.0.1"

#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

#define MAX_NAME_LENGTH 30
#define MAX_LINE_LEN 100
#define PROGRAM_FAILED -1
#define PROGRAM_SUCCEDDED 0

int player_turn;

typedef struct _player_ {
	char name[MAX_NAME_LENGTH];
	int currently_playing;
	int color;

}player;


#endif