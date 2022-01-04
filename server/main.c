/*
Authors:

Amit Tzah 316062959
Tomer Shimshi 203200480


Project: Ex4
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

#include "service_thread.h"
#include "create_and_handle_processes.h"
#include "HardCodedData.h"
#include "file_IO.h"
#include "socket_send_recv.h"

#pragma comment(lib,"WS2_32")

HANDLE ghMutex;
HANDLE mutex_to_sync_threads_when_waiting_for_players;
HANDLE event_for_syncing_threads_in_game_loop;

//this barrier will be used so have all threads enter the the game loop at the same time.
SYNCHRONIZATION_BARRIER barrier;

shared_server_resources resources_struct;

int accept_or_deny_connections(HANDLE ThreadHandles[NUM_OF_WORKER_THREADS], SOCKET ThreadInputs[NUM_OF_WORKER_THREADS], SOCKET MainSocket);
int create_thread_syncing_objects();

void main(int argc, char* argv[]) {

	resources_struct.game_has_ended = 0;
	resources_struct.game_number = 0;
	resources_struct.first_arrived = 0;
	resources_struct.num_of_players_ready_to_play = 0;
	resources_struct.current_player_move = NULL;

	int result;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	if (create_thread_syncing_objects() == ERROR_CODE) {
		goto server_cleanup;
	}

	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		goto server_cleanup;
	}

	// Create a socket.    
	 SOCKET MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup;
	}

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(strtol(argv[1], NULL, 10));

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup;
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (int Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	printf("Waiting for a client to connect...\n");

	//loop to open threads and aceept connections as needed.
	//until exit is entered.
	if (accept_or_deny_connections(ThreadHandles, ThreadInputs, MainSocket) == 1) {

		goto server_cleanup;

	}

server_cleanup:
	
	result = WSACleanup();
	if (result != NO_ERROR) {
		printf("error %ld at WSACleanup( ), ending program.\n", WSAGetLastError());
	}

}


//if some api api function fails, return ERROR_CODE, otherwise 0. 
int accept_or_deny_connections(HANDLE ThreadHandles[NUM_OF_WORKER_THREADS], SOCKET ThreadInputs[NUM_OF_WORKER_THREADS], SOCKET MainSocket) {

	while (1)
	{

		char* communication_message = NULL;
		char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];



		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			return ERROR_CODE;
		}

		printf("Client Connected.\n");

		int Ind = find_index_of_unused_thread(ThreadHandles, NUM_OF_WORKER_THREADS);
		char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

		// if 2 clients are already connected
		if (Ind == NUM_OF_WORKER_THREADS)
		{
			//first get the CLIENT_REQUEST
			if (ERROR_CODE == recv_and_extract_communication_message(AcceptSocket, &communication_message, message_type, parameters_array)) {

				return ERROR_CODE;

			}


			free_communication_message_and_parameters(communication_message, parameters_array, message_type);



			//send back SERVER_DENIED

			if (ERROR_CODE == send_message(AcceptSocket, SERVER_DENIED, parameters_array)) {
				return ERROR_CODE;

			}


			closesocket(AcceptSocket);

		}
		else
		{

			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&(ThreadInputs[Ind]),
				0,
				NULL
			);
			if (ThreadHandles[Ind] == NULL) {

				printf("Error creaating a thread in server. Code: %d\n", GetLastError());
				return ERROR_CODE;

			}

		}
	}

	return SUCCESS_CODE;

}

//if some api api function fails, return ERROR_CODE, otherwise 0. 
int create_thread_syncing_objects() {
	DWORD last_error;


	event_for_syncing_threads_in_game_loop = CreateEvent(
		NULL, /* default security attributes */
		FALSE,       /* Auto-reset event */
		FALSE,      /* initial state is non-signaled */
		NULL);         /* create event object without a name */
	/* Check if succeeded and handle errors */
	last_error = GetLastError();

	if (last_error != ERROR_SUCCESS) {
		printf("Error creating event onject in server main\n");
		return ERROR_CODE;

	}


	// Create a mutex with no initial owner
	//to protect common resources.
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (ghMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return ERROR_CODE;
	}


	// Create a mutex with no initial owner
	//to sync threads when waiting for players.
	mutex_to_sync_threads_when_waiting_for_players = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (mutex_to_sync_threads_when_waiting_for_players == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return ERROR_CODE;
	}

	if (false == InitializeSynchronizationBarrier(&barrier, NUM_OF_WORKER_THREADS, -1)) {

		printf("InitializeSynchronizationBarrier error: %d\n", GetLastError());
		return ERROR_CODE;

	}



	return SUCCESS_CODE;


}
