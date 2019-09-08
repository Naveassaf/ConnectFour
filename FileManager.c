/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	This module contains a signle function which helps in tracking the program's current location within the provided player file
*/

#include "FileManager.h"

/*
Function:		GetLine
Inputs:			filepath - the path to file we want to read from.
str - pointer that the specific line will be stored in.
idx - integer that represent the specific line from file we want to read.
Outputs:		return value = MORE_LINES_EXISTS/EOF depends on location in file.
Functionality:	The function read a specific line from file and stores it in str global pointer for further useage.
*/
int GetLine(char* filepath, char* str, int idx)
{
	FILE *file = fopen(filepath, "r");
	int count = 0;
	char c = '\0';
	char line[100];
	int cur_len = 0;

	if (file != NULL)
	{
		while (!feof(file))
		{
			while (c != '\n' && !feof(file))
			{
				c = getc(file);

				if (c != '\n')
				{
					line[cur_len] = c;
					cur_len++;
				}
			}

			count++;

			c = '\0';
			line[cur_len] = '\0';

			if (idx == count)
			{
				strcpy(str, line);
				fclose(file);
				return MORE_LINES_EXIST;
			}
			else
				cur_len = 0;

		}
		fclose(file);
		return REACHED_EOF;
	}
	else
		printf("unable to open file\n"); // Didn't manage to open file.
	
	return PROGRAM_FAILED;

}

