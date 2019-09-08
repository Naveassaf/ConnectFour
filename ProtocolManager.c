/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	This module manages the syntheetic protocol we used in order to deliver the necessary messages 
				between the clients and servers. It contains functions wihich, given a message, package it into
				the defined protocol as well as functions which given a "packaged" message, read it according to the
				protocol's rules and act accordingly (along with several help functions for these procedures). 
*/

#include "ProtocolManager.h"

/*
Function:		ClientPackageMessage
Inputs:			string - string received from one of the clients (command line/client's file format)
format_str - empty global string to be changed to the desired one.
Outputs:		format_str - transormed string (according to PROTOCOL's formats).
Functionality:	The function recieves a string (e.g "play 5") and transforms it to be in alligned with protocol (e.g "PLAY_REQUEST:5").
*/
char* ClientPackageMessage(char* string, char* format_str)
{
	char* res_str[MAX_MESSAGE_LENGTH];			//Assuming no more white spaces than twice "real char" in message.
												//char* format_str = (char*)malloc((3 * strlen(string) + 15));
	char col_num[2];

	if((strlen("play 1") == strlen(string)) &&(strncmp("play", string, strlen("play")) == 0))
	{
		strncpy(format_str, "PLAY_REQUEST:", strlen("PLAY_REQUEST: "));
		format_str[strlen("PLAY_REQUEST:")] = string[strlen(string) - 1];

		if (' ' != string[strlen(string) - 2])
		{
			format_str[strlen("PLAY_REQUEST:")] = '9';
			format_str[strlen("PLAY_REQUEST:") + 1] = '\0';
		}
		else
			format_str[strlen("PLAY_REQUEST:") + 1] = '\0';


		return (format_str);
	}
	else if (strncmp("message ", string, strlen("message ")) == 0)
	{

		strncpy(format_str, "SEND_MESSAGE: ", strlen("SEND_MESSAGE:  "));
		translate(string, res_str);
		strcat(format_str, res_str);
		return (format_str);
	}
	else if((strlen("exit") == strlen(string))&& (strncmp("exit", string, strlen("exit")) == 0))
	{

		strcpy(format_str, "GAME_ENDED");
	}
	else
	{
		strcpy(format_str, "Error:Illegal command\n");
		return (format_str);
	}
}

/*
Function:		ClientHandleAllCases
Inputs:			current_message - the current message that was received from server.
Outputs:		None.
Functionality:	The function will be called from client every time there's trnasacation and performs the steps should be perform (including
prints, writing to logfile and call auxiliary functions like print board's status).
*/
int ClientHandleAllCases(char * current_message, int *player_number, FILE *fp)
{
	char formated_str[MAX_MESSAGE_LENGTH + MAX_NAME_LENGTH];
	int board_to_print[BOARD_HEIGHT][BOARD_WIDTH];

	if (strncmp("NEW_USER_ACCEPTED", current_message, strlen("NEW_USER_ACCEPTED")) == 0)
	{
		//change player_num accordinngly
		*player_number = (current_message[strlen(current_message) - 1]) - 1 - '0';
	}
	else if (strncmp("NEW_USER_DECLINED", current_message, strlen("NEW_USER_DECLINED")) == 0)
	{
		printf("Request to join was refused\n");
		return CLEAN_EXIT;
	}
	else if (strncmp("GAME_STARTED", current_message, strlen("GAME_STARTED")) == 0)
	{
		if ((*player_number == 0) && file == 1)
		{
			
			//semaphore "up" 1 for player 0
			if (FALSE == ReleaseSemaphore(file_turn_sem, 1, NULL))
			{
				printf("Error releasing semaphore!-game started\n");
				return HANDLE_ALL_FAIL;
			}
		}
		printf("Game is on!\n");
	}
	else if (strncmp("TURN_SWITCH", current_message, strlen("TURN_SWITCH")) == 0)
	{
		printf("%s's turn\n", current_message + 12);
		fprintf(fp, "%s's turn\n", current_message + 12);
		if ((0 == strcmp(player_name, current_message + 12)) && file)
		{
			//up on semaphore
			if (FALSE == ReleaseSemaphore(file_turn_sem, 1, NULL))
			{
				printf("Error releasing semaphore-turn switch!\n");
				fprintf(fp, "Error releasing semaphore!-turn switch\n");
				return HANDLE_ALL_FAIL;
			}
		}
	}
	else if (strncmp("RECEIVE_MESSAGE", current_message, strlen("RECEIVE_MESSAGE")) == 0)
	{
		UnPackageMessage(current_message, formated_str);
		printf("%s", formated_str);
	}
	else if (strncmp("BOARD_VIEW", current_message, strlen("BOARD_VIEW")) == 0)
	{
		prep_board(current_message+11);
		PrintBoard(board);
	}
	else if (strncmp("PLAY_ACCEPTED", current_message, strlen("PLAY_ACCEPTED")) == 0)
		printf("Well Played\n");
	else if (strncmp("PLAY_DECLINED", current_message, strlen("PLAY_DECLINED")) == 0)
	{
		
		if (0 == strcmp(current_message + 14, "Game; ;has; ;not; ;started"))
		{
			printf("Game has not started\n");
		}
		else if (0 == strcmp(current_message + 14, "Not; ;your; ;turn"))
		{
			printf("Not your turn\n");
		}
		else if (0 == strcmp(current_message + 14, "Illegal; ;move"))
		{
			//up on semaphore
			if ((FALSE == ReleaseSemaphore(file_turn_sem, 1, NULL)) && file)
			{
				printf("Error releasing semaphore!-play declined\n");
				fprintf(fp, "Error releasing semaphore!-play declined\n");
				return HANDLE_ALL_FAIL;
			}
			printf("Illegal move\n");
		}
				
		/*
		switch (current_message[14])
		{
		case '1':printf("Not your turn\n"); break;
		case '2':
		{
			//up on semaphore
			if ((FALSE == ReleaseSemaphore(file_turn_sem, 1, NULL)) && file)
			{
				printf("Error releasing semaphore!-play declined\n");
				fprintf(fp, "Error releasing semaphore!-play declined\n");
				return HANDLE_ALL_FAIL;
			}
			printf("Illegal move\n"); break;
		}
		case '3':printf("Game has not started\n"); break;
		}*/
	}

	else if (strncmp("GAME_ENDED", current_message, strlen("GAME_ENDED")) == 0)
	{
		if (strncmp("tie", current_message + 11, strlen("tie")) == 0)
		{
			printf("Game ended. Everybody wins!\n");
			fprintf(fp,"Game ended. Everybody wins!\n");
		}
		else
		{
			printf("Game ended.The winner is %s!\n", current_message + 11);
			fprintf(fp,"Game ended.The winner is %s!\n", current_message + 11);
		}
	}
	else
		printf("Unsupported Message\n");

	return HANDLE_ALL_SUCCESS;
}

/*
Function:		GetMessageType
Inputs:			string - string received from one of the clients.
Outputs:		PLAY/MESSAGE/EXIT/ERR depends on the received string format.
Functionality:	The function decides the message's type (can be one of 3 optionals+error) and return an enumarated value.
*/
int GetMessageType(char* string)
{
	enum { PLAY, MESSAGE, ERR, EXIT };

	if (strncmp("PLAY_REQUEST", string, strlen("PLAY_REQUEST")) == 0)
		return PLAY;
	else if (strncmp("SEND_MESSAGE", string, strlen("SEND_MESSAGE")) == 0)
		return MESSAGE;
	else if (strncmp("EXIT", string, strlen("EXIT")) == 0)
		return EXIT;
	else
		return ERR;
}

/*
Function:		ServerPackageMessage
Inputs:			string - string received from one of the clients (command line/client's file format)
format_str - empty global string to be changed to the desired one.
Outputs:		format_str - transormed string (according to PROTOCOL's formats).
Functionality:	The function recieves a string (e.g "play 5") and transforms it to be in alligned with protocol (e.g "PLAY_REQUEST:5").
*/
char* ServerPackageMessage(char* string, char* format_str)
{
	char* res_str[MAX_MESSAGE_LENGTH];			//Assuming no more white spaces than twice "real char" in message.
												
	char col_num[2];

	if (strncmp("play", string, strlen("play")) == 0)
	{
		strncpy(format_str, "PLAY_REQUEST:", strlen("PLAY_REQUEST: "));
		format_str[strlen("PLAY_REQUEST:")] = string[strlen(string) - 1];

		if (' ' != string[strlen(string) - 2])
		{
			format_str[strlen("PLAY_REQUEST:")] = '9';
			format_str[strlen("PLAY_REQUEST:") + 1] = '\0';
		}
		else
			format_str[strlen("PLAY_REQUEST:") + 1] = '\0';


		return (format_str);
	}
	else if (strncmp("message", string, strlen("message")) == 0)
	{

		strncpy(format_str, "SEND_MESSAGE: ", strlen("SEND_MESSAGE:  "));
		translate(string, res_str);
		strcat(format_str, res_str);
		return (format_str);
	}
	else if (strncmp("exit", string, strlen("exit")) == 0)
	{

		strcpy(format_str, "EXIT");
	}
	else
	{
		strcpy(format_str, "Error:Illegal command\n");
		return (format_str);
	}
	
	return PROGRAM_SUCCEDDED;
}

/*
Function:		translate
Inputs:			str - string received from one of the clients (command line/client's file format)
mod_str - empty global string to be changed to the desired one.
Outputs:		mod_str - part of transormed string (according to PROTOCOL's formats).
Functionality:	The function is an auxiliary function to PackageMessage(). Doing string manipulations (i.e replace spaces with ";;").
*/
char* translate(char* str, char* mod_str)
{
	int len = strlen(str);
	int i = 0;  // Index in modified string 
	int j = 8; // Index in original string 

			   // Traverse string 
	while (j < len - 1)
	{
		// Replace occurrence of " " with ";;" 
		if (str[j] == ' ')
		{
			// Increment j by 2 
			j++;
			mod_str[i++] = ';';
			mod_str[i++] = ' ';
			if(str[j] != ' ')
				mod_str[i++] = ';';
			continue;
		}
		mod_str[i++] = str[j++];
	}
	if (j == len - 1)
		mod_str[i++] = str[j];
	// add a null character to terminate string 
	mod_str[i++] = '\n';
	mod_str[i] = '\0';
	return mod_str;
}

/*
Function:		PassMessage
Inputs:			sender - player* that represent the client who sent the message.
sent_message - the message itself.
format_str - empty global string to be transformed.
Outputs:		format_str - message to be sent to the other client.
Functionality:	The function will be called from server every time one client is sending message to another. it change it according to
recieve message protocol.
*/
char* PassMessage(player* sender, char* sent_message, char* format_str)
{

	char new_message[MAX_MESSAGE_LENGTH + MAX_NAME_LENGTH];

	char name[MAX_NAME_LENGTH];

	strcpy(name, sender->name);

	strcpy(new_message, name);                                 // new_message = <sender's name>

	strcpy(new_message + strlen(name), ";");                  // new_message = <sender's name>;

	strcpy(new_message + strlen(name) + 1, sent_message + 14); // new_message = <sender's name>;<sent_message>

	strcpy(format_str, "RECEIVE_MESSAGE:");                    // format_str = "RECIEVED_MESSAGE: "

	strcat(format_str, new_message);                           // format_str = "RECIEVED_MESSAGE: <sender's name>;<sent_message>"

	return format_str;
}

/*
Function:		ServerHandleAllCases
Inputs:			current_message - the current message that was received from server.
Outputs:		None.
Functionality:	The function will be called from client every time there's trnasacation and performs the steps should be perform (including
prints, writing to logfile and call auxiliary functions like print board's status).
*/
void ServerHandleAllCases(char * current_message)
{
	char formated_str[MAX_MESSAGE_LENGTH + MAX_NAME_LENGTH];
	if (strncmp("NEW_USER_ACCEPTED", current_message, strlen("NEW_USER_ACCEPTED")) == 0)
		printf("Welcome to the game!\n");	
	else if (strncmp("NEW_USER_DECLINED", current_message, strlen("NEW_USER_DECLINED")) == 0)
		printf("Request to join was refused\n");
	else if (strncmp("GAME_STARTED", current_message, strlen("GAME_STARTED")) == 0)
		printf("Game is on!\n");
	else if (strncmp("TURN_SWITCH", current_message, strlen("TURN_SWITCH")) == 0)
		printf("%s's turn\n", current_message + 12);
	else if (strncmp("RECEIVE_MESSAGE", current_message, strlen("RECEIVE_MESSAGE")) == 0)
		UnPackageMessage(current_message, formated_str);
	else if (strncmp("BOARD_VIEW", current_message, strlen("BOARD_VIEW")) == 0)
	{
		PrintBoardColorless(current_message + 11);
	}
	else if (strncmp("PLAY_ACCEPTED", current_message, strlen("PLAY_ACCEPTED")) == 0)
		printf("Well Played\n");
	else if (strncmp("PLAY_DECLINED", current_message, strlen("PLAY_DECLINED")) == 0)
	{
		switch (current_message[14])
		{
		case '1':printf("Not your turn\n"); break;
		case '2':printf("Illegal move\n"); break;
		case '3':printf("Game has not started\n"); break;
		}
	}

	else if (strncmp("GAME_ENDED", current_message, strlen("GAME_ENDED")) == 0)
	{
		if (strncmp("tie", current_message + 11, strlen("tie")) == 0)
			printf("Game ended. Everybody wins!\n");
		else
			printf("Game ended.The winner is %s!\n", current_message + 11);
	}
	else
		printf("Unsupported Message\n");
}

/*
Function:		PrintBoardColorless
Inputs:			board_str - the current board status represented in string that received from server.
Outputs:		None. prints an "image" of the board's status.
Functionality:	The function that will be called from client after recieving BOARD_VIEW, and prints the board status according to our convention.
*/
void PrintBoardColorless(char* board_str)
{
	
	int row, col;
	int idx = 0;

	for (row = 5; row >= 0; row--)
	{
		for (col = 0; col < 8; col++)
		{
			idx = row * 8 + col;

			if (board_str[idx] != '3')
			{
				printf("%c\t", board_str[idx]);
				idx++;
			}
			else
			{
				printf("\n");
				idx++;
			}

		}
	}
	printf("\n");
}

/*
Function:		slang_translate
Inputs:			str - the received string from server (according to protocol).
mod_str - empty global string.
Outputs:		mod_str - modified string which can be print to client after done with UnPackageMessage().
Functionality:	The function is the opposite function to translate(), and it is an auxiliary function to UnPackageMessage().
				it is mainly ignore the ';' and place ':' after the players name.
*/
char* slang_translate(char* str, char* mod_str)
{
	
	int len = strlen(str);
	int i = 0;  // Index in modified string 
	int j = 16; // Index in original string 
	int sender_flag = 1;

	while (j < len - 1) 	// Traverse string 
	{
		if (str[j] == ';')
		{
			if (sender_flag)
			{
				mod_str[i++] = ':'; // first ";;" means we're right after sender's name so we put ":"
				sender_flag--;
			}
			j++;
			continue;
		}
		mod_str[i++] = str[j++];
	}
	if (j == len - 1)
		mod_str[i++] = str[j];
	// add a null character to terminate string 
	mod_str[i] = '\0';
	return mod_str;
}

/*
Function:		UnPackageMessage
Inputs:			string - the received string from server (according to protocol).
slang_str - empty global string.
Outputs:		slang_str - modified string which can be print to client's prompt.
Functionality:	The function is the opposite to package- when client recieves massage, it unpackage it and print to it's prompt.
*/
char* UnPackageMessage(char* string, char* slang_str)
{

	if (strncmp("RECEIVE_MESSAGE", string, strlen("RECEIVE_MESSAGE")) == 0)
	{
		slang_translate(string, slang_str);
	}
	return (slang_str);
}

/*
Function:		PassBoard
Inputs:			board_str - empty global string.
Outputs:		board_str - modified string with the board's status (1,2 for playes, 3 for '\n'- useful for print in client).
Functionality:	The function transfers the boards status to the clients, so they only need to print it in order to understand the status.
*/
char* PassBoard(char *board_str)
{
	int idx = 0;
	char str_format[BOARD_HEIGHT*BOARD_WIDTH*2] = "BOARD_VIEW:";
	char curr_board[BOARD_HEIGHT*BOARD_WIDTH*2];
	int curr_col_int;
	char curr_element;
	char* curr_col_buff[MAX_LINE_LEN];
	for (int row = BOARD_HEIGHT - 1; row >= 0; row--)
	{
		for (int col = 0; col < BOARD_WIDTH; col++)
		{
			curr_element = board[row][col] + '0';
			curr_board[idx] = curr_element;

			idx++;
		}
		curr_board[idx] = '3';	// known invalid number in matrix means end of iteration.
		idx++;
	}
	curr_board[idx] = '\0';
	
	strcpy(board_str, str_format);
	strcpy(board_str + 11, curr_board);	// new_message = <sender's name>;;<sent_message>
										
	return (board_str);
}

