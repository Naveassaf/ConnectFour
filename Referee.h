/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	Header of Referee module. Description of the actual module may be found in the source file.
*/

#include <stdio.h>
#include <Windows.h>

#define BLACK  15
#define RED    204
#define YELLOW 238

#define RED_PLAYER 1
#define YELLOW_PLAYER 2

#define BOARD_HEIGHT 6
#define BOARD_WIDTH 7

#define MOVE_FAILED 1111
#define MOVE_SUCCEEDED 2222

#define GAME_WON 3333
#define GAME_CONTINUES 4444


int board[BOARD_HEIGHT][BOARD_WIDTH];
HANDLE  hConsole;


int PlayMove(int col, int player);
void initialize_board();
void PrintBoard(int **board_to_print);
int check_vertical(int player);
int check_horizontal(int player);
int check_down_diagonal(int player);
int check_up_diagonal(int player);
int check_for_win(int player);
void prep_board(char* msg);