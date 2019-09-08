/*
Authors:		Nave Assaf 308044809, Roi Elad 305625402
Project:		ex4
Description:	The main file of the project - reads in command lines and creates a server or client thread based on the 
				command line arguments provided.
*/


#include <stdio.h>
#include <windows.h>
#include "SocketExampleServer.h"

#pragma warning (disable : 4996)

typedef struct _client_params {
	char log_path[MAX_LINE_LEN];
	int port_num;
	char mode[6];
	char file_path[MAX_LINE_LEN];
}client_params;


DWORD MainClient(client_params input_params);

/*
Function:		main
Inputs:			argc - number of command line arguments provided
				argv - "split" command line
Outputs:		0 for successful run, -1 for crash
Functionality:	main thread - only calls real_main() and returns it's return value.
*/
int main(int argc, char* argv[])
{
	return real_main(argc, argv);
}

/*
Function:		real_main
Inputs:			argc - number of command line arguments provided
				argv - "split" command line
Outputs:		0 for successful run, -1 for crash
Functionality:	where the main server thread or the main client threads are created. sets the parameters
				necessary for the thread creation based on the command line provided, creates the thread, 
				and waits for the thread to end - returning PROGRAM_SUCCEDDED if all went well.
*/
int real_main(int argc, char* argv[])
{
	server_params *pass_to_server = NULL;
	HANDLE server_handle;
	client_params *pass_to_client = NULL;
	HANDLE client_handle;


	//If command call is of client's program
	if (0 == strcmp("client", argv[1]))
	{
		

		if (NULL == (pass_to_client = (client_params*)malloc(sizeof(client_params))))
		{
			printf("Memory allocation failed.\n");
			return PROGRAM_FAILED;
		}

		strcpy(pass_to_client->log_path, argv[2]);
		pass_to_client->port_num = atoi(argv[3]);
		strcpy(pass_to_client->mode, argv[4]);
		
		if (0 == strcmp("file", argv[4]))
		{
			strcpy(pass_to_client->file_path, argv[5]);
		}
		else
		{
			strcpy(pass_to_client->file_path, "None");
		}

		client_handle = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)MainClient,
			pass_to_client,
			0,
			NULL
		);

		if (NULL == client_handle)
		{
			printf("Client thread creation failed.");
			return PROGRAM_FAILED;
		}
		if (WAIT_FAILED == WaitForSingleObject(client_handle, INFINITE))
		{
			printf("Client thread wait failed.");
			return PROGRAM_FAILED;
		}
		
		CloseHandle(client_handle);
		free(pass_to_client);
	}
	else if ((0 == strcmp("server", argv[1])))
	{

		if (NULL == (pass_to_server = (server_params*)malloc(sizeof(server_params))))
		{
			printf("Memory allocation failed.\n");
			return PROGRAM_FAILED;
		}
		
		strcpy(pass_to_server->file_path, argv[2], MAX_LINE_LEN);
		pass_to_server->port_num = atoi(argv[3]);
		
		server_handle = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)MainServer,
			pass_to_server,
			0,
			NULL
		);
		
		if (NULL == server_handle)
		{
			printf("Server thread creation failed.");
			return PROGRAM_FAILED;
		}

		if (WAIT_FAILED == WaitForSingleObject(server_handle, INFINITE))
		{
			printf("Server thread wait failed.");
			return PROGRAM_FAILED;
		}
		
		CloseHandle(server_handle);
		free(pass_to_server);
	}
	else
	{
		printf("Improper command line call provided\n");
		return PROGRAM_FAILED;
	}

	return PROGRAM_SUCCEDDED;
}