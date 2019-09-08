#pragma once

#include "SocketExampleShared.h"

#define MAX_MESSAGE_LENGTH 100

int GetMessageType(char* string);
char* PackageMessage(char* string, char* format_str);
char* translate(char* str, char* mod_str);
char* PassMessage(player* sender, char* sent_message, char* format_str);
void HandleAllCases(char * current_message);
void PrintBoardColorless(char* board_str);
char* PassBoard(char *board_str);
char* slang_translate(char* str, char* mod_str);
char* UnPackageMessage(char* string, char* slang_str);