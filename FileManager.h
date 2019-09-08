/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	Header of FileManager module. Description of the actual module may be found in the source file.
*/

#pragma once
#include <stdio.h>
#include <string.h>
#pragma warning(disable:4996)

#define REACHED_EOF 3
#define MORE_LINES_EXIST 4
#define PROGRAM_FAILED -1
#define PROGRAM_SUCCEDDED 0

int GetLine(char* filepath, char* str, int idx);
