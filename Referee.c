/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	This module is the "owner" of the playing board and the module which helps the server
				define a legal move. It also contians the functions which change the actual values on the 
				board (PlayMove) and print the board on the sevrver promt in colors.
*/

#include "Referee.h"

/*
Function:		PlayMove
Inputs:			col - the column which the client asked to insert disk to.
				player - current playing player (represented by enum).
Outputs:		MOVE_SUCCEEDED/MOVE_FAILED representing if play was legal or illegal.
Functionality:	The function return if the current play requested by one of the player is succseeded or failed.
*/
int PlayMove(int col, int player)
{
	int row;

	for (row = BOARD_HEIGHT - 1; row >= 0; row--)
	{
		if (board[row][col] == 0)
		{
			board[row][col] = player;
			return MOVE_SUCCEEDED;
		}
	}

	return MOVE_FAILED;
}

/*
Function:		initialize_board
Inputs:			None.
Outputs:		No return value, board[][] is initialized to all zeros.
Functionality:	The function initialize the board at every start of a new game (all board's cells are zeros).
*/
void initialize_board()
{
	int row, col;

	for (row = 0; row < BOARD_HEIGHT; row++)
	{
		for (col = 0; col < BOARD_WIDTH; col++)
		{
			board[row][col] = 0;
		}
	}
}

/*
Function:		check_for_win
Inputs:			player - current playing player.
Outputs:		GAME_WON/GAME_CONTINUES - indicates the game's status.
Functionality:	The function using 4 auxiliary function (check_horizontal,check_vertical,check_up_diagonal,check_down_diagonal) and decides
is the player won and thus the game is ended or the game hasn't won yet and thus we shall continue to play.
*/
int check_for_win(int player)
{
	if (check_horizontal(player) == GAME_WON)
		return GAME_WON;
	if (check_vertical(player) == GAME_WON)
		return GAME_WON;
	if (check_up_diagonal(player) == GAME_WON)
		return GAME_WON;
	if (check_down_diagonal(player) == GAME_WON)
		return GAME_WON;

	return GAME_CONTINUES;
}

/*
Function:		check_horizontal
Inputs:			player - current playing player.
Outputs:		GAME_WON/GAME_CONTINUES - indicates the game's status.
Functionality:	The function is an auxiliary function for check_for_win() only for the horizontal sequence of 4 same disks.
*/
int check_horizontal(int player)
{
	int row, col, streak = 0;

	for (row = 0; row < BOARD_HEIGHT; row++)
	{
		for (col = 0; col < BOARD_WIDTH; col++)
		{
			if (board[row][col] == player)
				streak++;
			else
				streak = 0;
			if (4 <= streak)
				return GAME_WON;
		}
		streak = 0;
	}

	return GAME_CONTINUES;
}

/*
Function:		check_vertical.
Inputs:			player - current playing player.
Outputs:		GAME_WON/GAME_CONTINUES - indicates the game's status.
Functionality:	The function is an auxiliary function for check_for_win() only for the vertical sequence of 4 same disks.
*/
int check_vertical(int player)
{
	int row, col, streak = 0;

	for (col = 0; col < BOARD_WIDTH; col++)
	{
		for (row = 0; row < BOARD_HEIGHT; row++)
		{
			if (board[row][col] == player)
				streak++;
			else
				streak = 0;
			if (4 <= streak)
				return GAME_WON;
		}
		streak = 0;

	}
	return GAME_CONTINUES;
}

/*
Function:		check_up_diagonal.
Inputs:			player - current playing player.
Outputs:		GAME_WON/GAME_CONTINUES - indicates the game's status.
Functionality:	The function is an auxiliary function for check_for_win() only for the up diagonals sequences of 4 same disks.
*/
int check_up_diagonal(int player)
{
	int i, j, k, streak = 0;

	for (int k = 0; k <= BOARD_WIDTH + BOARD_HEIGHT - 2; k++)
	{
		for (int j = 0; j <= k; j++)
		{

			int i = k - j;
			if (i < BOARD_HEIGHT && j < BOARD_WIDTH)
			{
				if (board[i][j] == player)
					streak++;
				else
					streak = 0;
				if (4 <= streak)
					return GAME_WON;
			}
		}
		streak = 0;
	}
	return GAME_CONTINUES;
}

/*
Function:		check_down_diagonal.
Inputs:			player - current playing player.
Outputs:		GAME_WON/GAME_CONTINUES - indicates the game's status.
Functionality:	The function is an auxiliary function for check_for_win() only for the down diagonals sequences of 4 same disks.
*/
int check_down_diagonal(int player)
{
	int Ylength = BOARD_HEIGHT;
	int Xlength = BOARD_WIDTH;
	int maxLength, k, y, x, streak = 0;

	if (Ylength > Xlength)
		maxLength = Ylength;
	else
		maxLength = Xlength;

	for (k = 0; k <= 2 * (maxLength - 1); k++)
	{
		for (y = Ylength - 1; y >= 0; y--)
		{
			x = k - (Ylength - y);
			if (y < BOARD_HEIGHT && x < BOARD_WIDTH && x >= 0)
			{
				if (board[y][x] == player)
					streak++;
				else
					streak = 0;
				if (4 <= streak)
					return GAME_WON;
			}
		}
		streak = 0;
	}

	return GAME_CONTINUES;
}

/*
Function:		PrintBoard
Inputs:			A 2D array representing the board
Outputs:		Prints the board, no return value
Functionality:	This function prints the board, and uses O as the holes and the disks are presented by red or yellow backgrounds.
*/
void PrintBoard(int board_to_print[BOARD_HEIGHT][BOARD_WIDTH])
{
	//This handle allows us to change the console's color
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int row, column;

	printf("\n");
	//Draw the board
	for (row = 0; row < BOARD_HEIGHT; row++)
	{
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("| ");
			if (board_to_print[row][column] == RED_PLAYER)
				SetConsoleTextAttribute(hConsole, RED);

			else if (board_to_print[row][column] == YELLOW_PLAYER)
				SetConsoleTextAttribute(hConsole, YELLOW);

			printf("O");

			SetConsoleTextAttribute(hConsole, BLACK);
			printf(" ");
		}
		printf("\n");

		//Draw dividing line between the rows
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("----");
		}
		printf("\n");
	}
	
}

/*
Function:		prep_board
Inputs:			BOARD_VIEW msg sent by the server after last play has been made
Outputs:		Prints the board, no return value
Functionality:	Updates the values of the board variable based on BOARD_VIEW message received
*/
void prep_board(char* msg)
{
	int row, col;
	int idx = 0;

	for (row = 0; row <= BOARD_HEIGHT - 1; row++)
	{
		for (col = 0; col < BOARD_WIDTH; col++)
		{
			
			idx = row * (BOARD_WIDTH+1) + col;
			
			board[BOARD_HEIGHT-1-row][col] = msg[idx] - '0';

		}
	}
}