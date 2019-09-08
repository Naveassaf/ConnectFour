/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	This module deals is the "main" module of the client threads. It runs 3 different threads
				in parallel - 1. Receives messages from server. 2. reads messages from user or file and 
				inputs them into a buffer. 3. Send the messages from the buffer at the order at which they are
				inserted into it. When a client disconnects, all his resources are freed here. 
*/

#include "SocketExampleClient.h"


//Reading data coming from the server - wait for messages from server and using HandleAllCases()
/*
Function:		RecvDataThread
Inputs:			None.
Outputs:		None.
Functionality:	The function waits until a data received from server and uses HandleAllCases() function (unless an error occured).
*/
static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;
	int code = HANDLE_ALL_SUCCESS;

	while (1)
	{
		char *AcceptedStr = NULL;

		RecvRes = ReceiveString(&AcceptedStr, m_socket);

		if ((0 == strncmp(AcceptedStr, "GAME_ENDED", strlen("GAME_ENDED"))) && (strlen(AcceptedStr) == strlen("GAME_ENDED")))
		{
			printf("Server disconnected. Exiting.\n");
			fprintf(log_fp, "Server disconnected. Exiting.\n");
			TerminateThread(hThread[0], 0x555);
			return PROGRAM_SUCCEDDED;
		}

		if (RecvRes == TRNS_FAILED)
		{
			return PROGRAM_FAILED;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			return PROGRAM_FAILED;
		}
		else
		{
			fprintf(log_fp, "Received from server:%s\n", AcceptedStr);
			code = ClientHandleAllCases(AcceptedStr, &player_number, log_fp);
		}
		if (CLEAN_EXIT == code)
		{
			TerminateThread(hThread[0], 0x555);
			return PROGRAM_SUCCEDDED;
		}


		if ((0 == strncmp(AcceptedStr, "GAME_ENDED", strlen("GAME_ENDED"))))
		{
			TerminateThread(hThread[0], 0x555);
			return PROGRAM_FAILED;
		}

	}

	return PROGRAM_SUCCEDDED;
}

/*
Function:		InputThread
Inputs:			None.
Outputs:		return value is 0 if succseeded.
Functionality:	The function reads data coming from the client (human-keyboard/file) and uses insert_to_buffer in order to insert to a node.
*/
static DWORD InputThread(void)
{
	char SendStr[MAX_MESSAGE_LEN], format_str[MAX_MESSAGE_LEN];
	TransferResult_t SendRes;
	char temp[MAX_MESSAGE_LEN];
	int first = 1;
	int file_line = 1;
	int reached_eof = MORE_LINES_EXIST;

	while (1)
	{
		//new user request is always first message
		if (first)
		{
			strncpy(SendStr, "NEW_USER_REQUEST:", 18);
			if (1 != file)
			{
				strcat(SendStr, player_name);
			}
			else
			{
				GetLine(cmd_file_path, temp, file_line);
				//takes first line to be player's name
				strcpy(player_name, temp);
				strcat(SendStr, temp);
				file_line++;
			}

			insert_to_buffer(SendStr, SendStr);
			first = 0;
		}
		else
		{
			if (1 != file)
			{
				
				gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
				ClientPackageMessage(SendStr, format_str);
			}
			else
			{
				
				reached_eof = GetLine(cmd_file_path, SendStr, file_line);
				file_line++;
				ClientPackageMessage(SendStr, format_str);
				
			}

			if (REACHED_EOF == reached_eof)
				break;

			if (strncmp(format_str, "Error:Illegal command", strlen("Error:Illegal command")) == 0)
			{
				if (!file)
				{
					printf("%s", format_str);
					fprintf(log_fp, "%s", format_str);
				}
				continue;
			}
			else
			{
				insert_to_buffer(format_str, SendStr);
			}
		}
	}
	return PROGRAM_SUCCEDDED;
}

/*
Function:		SendDataThread
Inputs:			player_name - the current playing client.
Outputs:
Functionality:	The function sends data stored in buffer to server.
*/
static DWORD SendDataThread(char* player_name)
{
	TransferResult_t SendRes;
	msg_node *cur_msg = NULL, *last_sent_msg = NULL;
	enum { PLAY, MESSAGE, ERR };
	char SendStr[MAX_MESSAGE_LEN];

	//down on num of msg in buffer semaphore
	if (WAIT_OBJECT_0 != WaitForSingleObject(msg_in_buffer, INFINITE))
		return PROGRAM_FAILED;

	last_sent_msg = buffer_head;
	SendRes = SendString(last_sent_msg->message, m_socket);
	fprintf(log_fp, "Sent to server:%s\n", last_sent_msg->raw_message);
	if (SendRes == TRNS_FAILED)
	{
		return PROGRAM_FAILED;
	}


	while (1)
	{
		
		//down on num of msg in buffer semaphore
		if (WAIT_OBJECT_0 != WaitForSingleObject(msg_in_buffer, INFINITE))
			return PROGRAM_FAILED;

		last_sent_msg = last_sent_msg->next;

		if (1 == file)
		{

			if (PLAY == GetMessageType(last_sent_msg->message))
			{
				if (WAIT_OBJECT_0 != WaitForSingleObject(file_turn_sem, INFINITE))
				{
					printf("Wait on turn semaphore failed.");
					return PROGRAM_FAILED;
				}

			}
		}

		if (0 == strcmp(last_sent_msg->message, "GAME_ENDED"))
		{
			printf("Server disconnected. Exiting.\n");
			fprintf(log_fp, "Server disconnected. Exiting.\n");
			return PROGRAM_SUCCEDDED;
		}
		else
		{
			fprintf(log_fp, "Sent to server:%s\n", last_sent_msg->message);
			SendRes = SendString(last_sent_msg->message, m_socket);
		}
		if (SendRes == TRNS_FAILED)
		{
			return PROGRAM_FAILED;
		}



	}


}

/*
Function:		insert_to_buffer
Inputs:			msg - the current message sent from client (for both cases human/file).
Outputs:		No return vaslues. the buffer is modified by this function.
Functionality:	The function takes the next client's command and inserts it to the buffer (linked list)
*/
static void insert_to_buffer(char* msg, char* raw_msg)
{
	msg_node *new_node = NULL, *cur_node = NULL, *next = NULL;

	if (NULL == (new_node = (msg_node*)malloc(sizeof(msg_node))))
	{
		printf("Memory allocation failed.\n");
		return PROGRAM_FAILED;
	}
	strcpy(new_node->message, msg);
	new_node->next = NULL;
	strcpy(new_node->raw_message, raw_msg);

	if (NULL == buffer_head)
	{
		buffer_head = new_node;
	}
	else
	{
		cur_node = buffer_head;
		while (NULL != cur_node->next)
		{
			cur_node = cur_node->next;
		}
		cur_node->next = new_node;
	}


	//semaphore "up" 1 for player 0
	if (FALSE == ReleaseSemaphore(msg_in_buffer, 1, NULL))
	{
		return PROGRAM_FAILED;
	}
	return PROGRAM_SUCCEDDED;
}

/*
Function:		MainClient
Inputs:			client_params input_param
Outputs:		None
Functionality:	Main thread of client in which all other threads are created. The main functions opens a thread here
				when the second command line argument is 'client'.
*/
DWORD MainClient(client_params *input_params)
{
	SOCKADDR_IN clientService;
	HANDLE *InputHandle = NULL;
	int port_num;
	char temp_name[MAX_LINE_LEN];
	buffer_head = NULL;

	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
					 //The WSADATA structure contains information about the Windows Sockets implementation.

					 //Call WSAStartup and check for errors.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		return PROGRAM_FAILED;
	}

	// Create a socket.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	port_num = input_params->port_num;
	log_fp = fopen(input_params->log_path, "w");

	if (NULL == log_fp)
	{
		printf("Error opening log file\n");
		fprintf(log_fp, "Error opening log file\n");
		return PROGRAM_FAILED;
	}

	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return PROGRAM_FAILED;
	}

	//Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	clientService.sin_port = htons(port_num); //Setting the port to connect to.

	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s:%d. Exiting\n", SERVER_ADDRESS_STR, port_num);
		fprintf(log_fp, "Failed connecting to server on %s:%d. Exiting\n", SERVER_ADDRESS_STR, port_num);
		WSACleanup();
		return PROGRAM_FAILED;
	}
	else
	{
		printf("Connected to server on %s:%d\n", SERVER_ADDRESS_STR, port_num);
		fprintf(log_fp, "Connected to server on %s:%d\n", SERVER_ADDRESS_STR, port_num);
	}

	//Initialize client parameters based on args provided in command line call
	strcpy(player_name, "temporary");
	if (0 == strcmp("file", input_params->mode))
		file = 1;
	else
	{
		printf("Please enter your name:\n");
		gets_s(temp_name, MAX_LINE_LEN); //Reading a string from the keyboard
		strcpy(player_name, temp_name);
		file = 0;
	}

	initialize_board();

	if (file)
		strcpy(cmd_file_path, input_params->file_path);

	msg_in_buffer = CreateSemaphore(
		NULL,					//Default sec.
		0,						//Init value
		MAX_BUFFER_SIZE,		//Max value
		NULL);

	//Create semaphore to signal beginning of Game
	file_turn_sem = CreateSemaphore(
		NULL,			//Default sec.
		0,				//Init value
		1,				//Max value
		NULL);

	if (NULL == msg_in_buffer || NULL == file_turn_sem)
	{
		printf("Semaphore creation failed.");
		return PROGRAM_FAILED;
	}

	// Send, insert to buffer and receive data.
	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		player_name,
		0,
		NULL
	);
	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		NULL,
		0,
		NULL
	);
	InputHandle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)InputThread,
		NULL,
		0,
		NULL
	);

	if (NULL == InputHandle || NULL == hThread[1] || NULL == hThread[0])
	{
		printf("Failed to create a client thread");
		return PROGRAM_FAILED;
	}

	WaitForSingleObject(hThread[0], INFINITE);
	TerminateThread(InputHandle, 0x555);
	TerminateThread(hThread[1], 0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(InputThread);
	CloseHandle(hConsole);
	CloseHandle(file_turn_sem);
	CloseHandle(msg_in_buffer);
	free_list();
	WSACleanup();
	fclose(log_fp);
	return PROGRAM_SUCCEDDED;
}

/*
Function:		free_list
Inputs:			None.
Outputs:		None.
Functionality:	The function release the buffer was allocated dynamically.
*/
void free_list()
{
	msg_node *next_node = NULL, *cur_node = buffer_head;

	while (NULL != cur_node)
	{
		next_node = cur_node->next;
		free(cur_node);
		cur_node = next_node;
	}
}