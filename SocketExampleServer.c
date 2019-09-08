/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	This module is the "main" module of the server thread. It opens a thread in charge of communication
				whith each logged on client. For the first of these threads it also opens a thread which communicates
				until the game begins. Finally, after each move is made, it ensures the move is legal and acts accoridngly
				while passing messages between the players in the defined synthetic protocol of this damn project.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"
#include "SocketExampleServer.h"
#include "Referee.h"
#include "ProtocolManager.h"


#define NUM_OF_WORKER_THREADS 2

#define SEND_STR_SIZE 35

#define NEW_USER_LEN 17

HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ActiveClientHandles[NUM_OF_WORKER_THREADS];
player ActivePlayers[NUM_OF_WORKER_THREADS];
HANDLE game_ended;
static HANDLE game_started;
int port_number;
FILE *log_fp = NULL;
int disconnection_count = 0;
int gave_over = 0;

static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET *t_socket);
static DWORD AwaitGameBeginning(void);


/*
Function:		MainServer
Inputs:			server_params input_param
Outputs:		None
Functionality:	Main thread of server in which all other threads are created. The main functions opens a thread here
				when the second command line argument is 'server'. 
*/
DWORD MainServer(server_params *input_param)
{
	int Ind;
	int Loop;
	char SendStr[SEND_STR_SIZE];
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError()); 
		fprintf(log_fp, "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		return PROGRAM_FAILED;
	}
	/* The WinSock DLL is acceptable. Proceed. */

	port_number = input_param->port_num;
	log_fp = fopen(input_param->file_path, "w");

	if (NULL == log_fp)
	{
		printf("Error opening log file\n");
		fprintf(log_fp, "Error opening log file\n");
		return PROGRAM_FAILED;
	}

	// Create a socket.    
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		fprintf(log_fp, "Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		fprintf(log_fp, "The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(port_number); //The htons function converts a u_short from host to TCP/IP network byte order 
										   //( which is big-endian ).
										   									
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		fprintf(log_fp, "bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	//Create semaphore to signal beginning of Game as turnpike
	game_started = CreateSemaphore(
		NULL,			//Default sec.
		0,				//Init value
		2,				//Max value
		NULL);

	if (NULL == game_started)
	{
		printf("Failed to create game starting semaphore.");
		fprintf(log_fp, "Failed to create game starting semaphore.");
		return PROGRAM_FAILED;
	}
	// Create playing board
	initialize_board();

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		fprintf(log_fp, "Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	printf("Waiting for a client to connect...\n");

	//Loop infinitely and listen for client connection requests
	while (1)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			fprintf(log_fp, "Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}

		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available OR MUST CHECKL IF NAME IS OKAY!!!!
		{
			strcpy(SendStr, "NEW_USER_DECLINED");
			SendRes = SendString(SendStr, AcceptSocket);

			if (SendRes == TRNS_FAILED)
			{
				printf("Service socket error while writing, closing thread.\n");
				fprintf(log_fp, "Service socket error while writing, closing thread.\n");
				closesocket(AcceptSocket);
				return PROGRAM_FAILED;
			}

		}
		else
		{
			ActiveClientHandles[Ind] = AcceptSocket; 
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&Ind,
				0,
				NULL
			);

			if (NULL == ThreadHandles[Ind])
			{
				printf("Failed to create server Service Thread");
				fprintf(log_fp, "Failed to create server Service Thread");
			}
		}
	}

server_cleanup_3:
	CleanupWorkerThreads();

server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
	{
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		fprintf(log_fp, "Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	}

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		fprintf(log_fp, "Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
	}

	CloseHandle(game_started);
	if (NULL != hConsole)
		CloseHandle(hConsole);
	fclose(log_fp);
	return PROGRAM_FAILED;
}

/*
Function:		FindFirstUnusedThreadSlot
Inputs:			None.
Outputs:		Ind- the first unused slot in ThreadHandles[].
Functionality:	The function return a number represting the first unused slot in threads list. according to this number the main
decides if the client can join the game (less than 2 clients) or can't (there are already 2 clients connected to server).
*/
static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) 
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}


/*
Function:		CleanupWorkerThreads
Inputs:			None.
Outputs:		None.
Functionality:	The function calls when there is AcceptSocket FAILED. it closes handle and sockets and remove the thread from ThreadHandle[].
*/
static void CleanupWorkerThreads()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(ActiveClientHandles[Ind]);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				fprintf(log_fp, "Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

/*
Function:		ServiceThread
Inputs:			thread_num - current thread number.
Outputs:		None.
Functionality:	Service thread is the thread that opens for each successful client connection and "talks" to the client. it is the function
in server where almost everything happens (acccept client, give them colors and manage the game).
*/
static DWORD ServiceThread(int *thread_num)
{
	char SendStr[MAX_MESSAGE_LENGTH + MAX_NAME_LENGTH], temp_str[MAX_MESSAGE_LENGTH + MAX_NAME_LENGTH];
	char *temp_name = NULL;
	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	SOCKET *t_socket = NULL, *others_socket = NULL;
	player *this_player = NULL, *other_player = NULL;
	HANDLE await_beginning_handle = NULL;
	int play_column;
	enum { PLAY, MESSAGE, ERR };
	int play_status;
	int moves_made = 0;
	int declined = 0;
	int t = *thread_num;
	int *Ind = &t;

	//Init local variables - player structs
	this_player = &ActivePlayers[*Ind];
	other_player = &ActivePlayers[(*Ind + 1) % 2];

	//Init local sockets to be used for message sending
	t_socket = &ActiveClientHandles[*Ind];
	others_socket = &ActiveClientHandles[(*Ind + 1) % 2];

	///////////////////////////////////
	//GAME PROTOCOL HANDSHAKE PROCESS//
	///////////////////////////////////

	RecvRes = ReceiveString(&temp_name, *t_socket);

	if (RecvRes == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		fprintf(log_fp, "Service socket error while reading, closing thread.\n");
		closesocket(*t_socket);
		return PROGRAM_FAILED;
	}

	//Initializes name player names by parsing NEW_USER_REQUEST message from client connection
	if (*Ind == 1)
	{
		//if name is already take, decline user
		if (strcmp(temp_name + NEW_USER_LEN, other_player->name) == 0)
		{
			strcpy(SendStr, "NEW_USER_DECLINED");
			printf("Second Player: %s rejected, name taken.\n", temp_name + NEW_USER_LEN);
			declined = 1;
		}
		else
		{
			this_player->color = YELLOW_PLAYER;
			this_player->currently_playing = 0;
			strcpy(this_player->name, temp_name + NEW_USER_LEN);
			strcpy(SendStr, "NEW_USER_ACCEPTED:2");
			printf("Second Player: %s accepted.\n", this_player->name);
		}
	}
	else
	{
		this_player->color = RED_PLAYER;
		this_player->currently_playing = 1;
		strcpy(this_player->name, temp_name + NEW_USER_LEN);
		strcpy(SendStr, "NEW_USER_ACCEPTED:1");
		printf("First Player: %s accepted.\n", this_player->name);
	}

	SendRes = SendString(SendStr, *t_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		fprintf(log_fp, "Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return PROGRAM_FAILED;
	}

	if (declined)
	{
		closesocket(*t_socket);
		return PROGRAM_FAILED;
	}

	////////////////////////
	//HAND SHAKE COMPLETED//
	////////////////////////

	/////////////////////////////////////////
	//RED PLAYER WAITS FOR YELLOW TO LOG ON//
	/////////////////////////////////////////

	//Game beginning timing - If first player, wait for game started semaphore to signal, else signal game started
	if (this_player->color == YELLOW_PLAYER)
	{
		//semaphore "up" 2
		if (FALSE == ReleaseSemaphore(game_started, 2, NULL))
		{
			printf("Error releasing semaphore.\n");
			fprintf(log_fp, "Error releasing semaphore.\n");
			return PROGRAM_FAILED;

		}
		//semaphore "down" 1
		if (WAIT_OBJECT_0 != WaitForSingleObject(game_started, INFINITE))
		{
			printf("Error waiting on semaphore.\n");
			fprintf(log_fp, "Error waiting on semaphore.\n");
			return PROGRAM_FAILED;
		}
	}
	else
	{

		//create listening thread for player one until player two is logged on
		await_beginning_handle = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)AwaitGameBeginning,
			NULL,
			0,
			NULL
		);
		if (WAIT_OBJECT_0 != WaitForSingleObject(game_started, INFINITE))
		{
			printf("Error waiting on semaphore.\n");
			fprintf(log_fp, "Error waiting on semaphore.\n");
			return PROGRAM_FAILED;

		}
		TerminateThread(await_beginning_handle, 1);
	}

	//Print game started only once in server
	if (this_player->color == RED_PLAYER)
	{
		printf("GAME_STARTED\n");
	}

	SendRes = SendString("GAME_STARTED", *t_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		fprintf(log_fp, "Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return PROGRAM_FAILED;

	}

	PassBoard(SendStr);
	SendRes = SendString(SendStr, *t_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		fprintf(log_fp, "Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return PROGRAM_FAILED;

	}

	while (!Done)
	{
		char *AcceptedStr = NULL;

		RecvRes = ReceiveString(&AcceptedStr, *t_socket);

		if ((RecvRes == TRNS_FAILED) || (RecvRes == TRNS_DISCONNECTED))
		{
			clean_disconnect(Ind);
			return PROGRAM_FAILED;
		}

		//////////////////////////////////////////
		//Check Message Type and Act Accordingly//
		//////////////////////////////////////////

		//If play command
		if (GetMessageType(AcceptedStr) == PLAY)
		{
			play_column = (AcceptedStr[strlen(AcceptedStr) - 1] - '0');

			//If two-digit column number was recieved
			if (':' != AcceptedStr[strlen(AcceptedStr) - 2])
				play_column = -1;

			if (this_player->currently_playing == 0)
			{
				printf("Attempted play out of turn\n");
				strcpy(SendStr, "PLAY_DECLINED:Not; ;your; ;turn");
			}
			else if (play_column < 0 || play_column >= BOARD_WIDTH)
			{
				printf("Illegal move made\n");
				strcpy(SendStr, "PLAY_DECLINED:Illegal; ;move");
			}
			else
			{
				play_status = PlayMove(play_column, this_player->color);

				if (MOVE_SUCCEEDED == play_status)
				{
					printf("Play accepted\n");
					SendRes = SendString("PLAY_ACCEPTED", *t_socket);
					moves_made++;

					if (SendRes == TRNS_FAILED)
					{
						clean_disconnect(Ind);
						return PROGRAM_FAILED;
					}

					//PrintBoard(board);
					PassBoard(SendStr);

					SendRes = SendString(SendStr, *t_socket);

					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						fprintf(log_fp, "Service socket error while writing, closing thread.\n");
						clean_disconnect(Ind);
						return PROGRAM_FAILED;
					}

					SendRes = SendString(SendStr, *others_socket);

					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						fprintf(log_fp, "Service socket error while writing, closing thread.\n");
						clean_disconnect(Ind);
						return PROGRAM_FAILED;
					}
					
					// If player won the game
					if (check_for_win(this_player->color) == GAME_WON)
					{
						printf("Player %s Won!!\n", this_player->name);
						strcpy(SendStr, "GAME_ENDED:");
						strcat(SendStr, this_player->name);
						SendRes = SendString(SendStr, *t_socket);
						SendRes = SendString(SendStr, *others_socket);
						gave_over = 1;
					}
					//Must also check if board is full
					if (moves_made == (BOARD_HEIGHT*BOARD_WIDTH/2) && this_player->color == YELLOW_PLAYER)
					{
						printf("Tie!\n");
						SendRes = SendString("GAME_ENDED:tie", *t_socket);
						SendRes = SendString("GAME_ENDED:tie", *others_socket);
						gave_over = 1;
					}

					//switches turns and updates players
					strcpy(SendStr, "TURN_SWITCH:");
					strcat(SendStr, other_player->name);
					this_player->currently_playing = 0;
					other_player->currently_playing = 1;

					SendRes = SendString(SendStr, *others_socket);

					if (SendRes == TRNS_FAILED)
					{
						//fprintf(log_fp, "Service socket error while writing, closing thread.\n");
						clean_disconnect(Ind);
						return PROGRAM_FAILED;
					}
					
				}
				else
				{
					printf("Illegal move made\n");
					strcpy(SendStr, "PLAY_DECLINED:Illegal; ;move");
				}
			}
			play_column = -1;

			if (!gave_over)
			{
				SendRes = SendString(SendStr, *t_socket);

				if (SendRes == TRNS_FAILED)
				{
					printf("Service socket error while writing, closing thread.\n");
					fprintf(log_fp, "Service socket error while writing, closing thread.\n");
					clean_disconnect(Ind);
					return PROGRAM_FAILED;
				}
			}
		}
		else if (GetMessageType(AcceptedStr) == MESSAGE)
		{
			PassMessage(this_player, AcceptedStr, temp_str);
			strcpy(SendStr, temp_str);
			SendRes = SendString(SendStr, *others_socket);
		}


		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			fprintf(log_fp, "Service socket error while writing, closing thread.\n");
			clean_disconnect(Ind);
			return PROGRAM_FAILED;
		}

		free(AcceptedStr);
	}

	closesocket(*t_socket);
	return PROGRAM_SUCCEDDED;
}

/*
Function:		AwaitGameBeginning
Inputs:			None.
Outputs:		None. Return value 1 if the current logged in player has disconnected or communication is lost.
Functionality:	The function is a thread which "listens" to first logged on player while waiting for second to log on.
*/
static DWORD AwaitGameBeginning(void)
{
	SOCKET *t_socket = &ActiveClientHandles[0];
	TransferResult_t RecvRes, SendRes;

	while (1)
	{
		char*  temp_start = NULL;

		RecvRes = ReceiveString(&temp_start, *t_socket);

		if ((RecvRes == TRNS_DISCONNECTED) || (RecvRes == TRNS_FAILED))
		{
			printf("Player disconnected. Ending communication.\n");
			fprintf(log_fp, "Player disconnected. Ending communication.\n");
			TerminateThread(ThreadHandles[0], 0x555);
			closesocket(*t_socket);
			return PROGRAM_FAILED;
		}

		SendRes = SendString("PLAY_DECLINED:Game; ;has; ;not; ;started", *t_socket);

		if ((SendRes == TRNS_FAILED) || (SendRes == TRNS_DISCONNECTED))
		{
			printf("Player disconnected. Ending communication.\n");
			TerminateThread(ThreadHandles[0], 0x555);
			closesocket(*t_socket);
			return PROGRAM_FAILED;
		}
	}
	return PROGRAM_FAILED;
}

/*
Function:		clean_disconnect
Inputs:			*Ind.
Outputs:		None.
Functionality:	The function is manage the disconnect when one player left the game (send message to server and initialize board).
*/
void clean_disconnect(int *Ind)
{
	SOCKET *t_socket = NULL, *others_socket = NULL;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	t_socket = &ActiveClientHandles[*Ind];
	others_socket = &ActiveClientHandles[(*Ind + 1) % 2];


	disconnection_count += 1;

	if (disconnection_count == 1)
		SendRes = SendString("GAME_ENDED", *others_socket);
	else if (disconnection_count == 2 && gave_over == 1)
	{
		initialize_board();
		gave_over = 0;
		disconnection_count = 0;
	}
	else if (disconnection_count == 2)
	{
		printf("Player disconnected. Ending communication.\n");
		fprintf(log_fp, "Player disconnected. Ending communication.\n");
		initialize_board();
		disconnection_count = 0;
	}
}