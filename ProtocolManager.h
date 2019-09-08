/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	Header of ProtocolManager module. Description of the actual module may be found in the source file.
*/

#pragma once

#include "SocketExampleShared.h"
#include "Referee.h"
#include <windows.h>
#include <stdio.h>

#define MAX_MESSAGE_LENGTH 100
#define CLEAN_EXIT 555
#define HANDLE_ALL_SUCCESS 0
#define HANDLE_ALL_FAIL -1

char player_name[MAX_LINE_LEN];
HANDLE file_turn_sem;
int player_number;
int file;

int GetMessageType(char* string);
char* ClientPackageMessage(char* string, char* format_str);
char* ServerPackageMessage(char* string, char* format_str);
char* translate(char* str, char* mod_str);
char* PassMessage(player* sender, char* sent_message, char* format_str);
int ClientHandleAllCases(char * current_message, int *player_number, FILE *fp);
void ServerHandleAllCases(char * current_message, int *player_number);
void PrintBoardColorless(char* board_str);
char* PassBoard(char *board_str);
char* slang_translate(char* str, char* mod_str);
char* UnPackageMessage(char* string, char* slang_str);