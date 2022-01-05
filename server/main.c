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


typedef struct exit_thread_parameters {
	int has_user_entered_exit;
	SOCKET socket_to_close;
} exit_thread_parameters;


//this barrier will be used so have all threads enter the the game loop at the same time.
SYNCHRONIZATION_BARRIER barrier;

shared_server_resources resources_struct;

int accept_or_deny_connections(HANDLE ThreadHandles[NUM_OF_WORKER_THREADS], SOCKET ThreadInputs[NUM_OF_WORKER_THREADS], SOCKET MainSocket);
int create_thread_syncing_objects();
DWORD check_for_exit_input(exit_thread_parameters* server_exiting);
void cleanup_worker_threads_and_sockets(HANDLE ThreadHandles[NUM_OF_WORKER_THREADS], SOCKET ThreadInputs[NUM_OF_WORKER_THREADS]);

int main(int argc, char* argv[]) {

	
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
	int error = 0;
	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
	 
	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		//nothing to free yet, can simply exit.
		return ERROR_CODE;
	}

	// Create a socket.    
	 SOCKET MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		error = 1;
		goto server_cleanup;
	}

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		error = 1;
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
		error = 1;
		goto server_cleanup;
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		error = 1;
		goto server_cleanup;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (int Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	if (create_thread_syncing_objects() == ERROR_CODE) {
		error = 1;
		goto server_cleanup;
	}

	//loop to open threads and aceept connections as needed.
	//until exit is entered.


	if (accept_or_deny_connections(ThreadHandles, ThreadInputs, MainSocket) == ERROR_CODE) {
		error = 1;
		goto server_cleanup;

	}

server_cleanup:

	cleanup_worker_threads_and_sockets(ThreadHandles, ThreadInputs);

	if (closesocket(MainSocket) == SOCKET_ERROR) {
		//if socket was already closed since exit was entered, don't print error.
		if (WSAGetLastError() != WSAENOTSOCK) {
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		}
	}

	CloseHandle(ghMutex);
	CloseHandle(mutex_to_sync_threads_when_waiting_for_players);
	CloseHandle(event_for_syncing_threads_in_game_loop);

	result = WSACleanup();
	if (result != NO_ERROR) {
		printf("error %ld at WSACleanup( ), ending program.\n", WSAGetLastError());
	}

	if (error == 1) {

		return ERROR_CODE;
	}
	return SUCCESS_CODE;

}


//if some api api function fails, return ERROR_CODE, otherwise 0. 
//wiil loop untill fails, or exit has been entered.
int accept_or_deny_connections(HANDLE ThreadHandles[NUM_OF_WORKER_THREADS], SOCKET ThreadInputs[NUM_OF_WORKER_THREADS], SOCKET MainSocket) {



	//create a thread to signal when user has entered 'exit'
	exit_thread_parameters server_exiting;

	server_exiting.has_user_entered_exit = 0;
	server_exiting.socket_to_close = MainSocket;

	HANDLE exit_thread= CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)check_for_exit_input,
		&server_exiting,
		0,
		NULL
	);
	if (exit_thread == NULL) {

		printf("Error creaating a thread in server. Code: %d\n", GetLastError());
		return ERROR_CODE;

	}

	//while exit hasnt been entered
	while (1)
	{

		char* communication_message = NULL;
		char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];

		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
			//check if it's not an Interrupted function call from user entering exit.
		{	if(WSAGetLastError()!= WSAEINTR)
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			return ERROR_CODE;

		}

	
		//configure socket to timeout recv calls after WAIT_FOR_RESPONSE ms
		if (set_time_out_to_recv_calls(AcceptSocket,WAIT_FOR_RESPONSE) == SOCKET_ERROR) {
			printf("setsockopt for SO_RCVTIMEO failed with error: %u\n", WSAGetLastError());
			return ERROR_CODE;

		}

		int Ind = find_index_of_unused_thread(ThreadHandles, NUM_OF_WORKER_THREADS);
		char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

		// if 2 clients are already connected
		if (Ind == NUM_OF_WORKER_THREADS)
		{
			//first get the CLIENT_REQUEST

			communication_message = NULL;
			init_parameter_array(parameters_array);

			if (recv_communication_message(AcceptSocket, &communication_message) == TRNS_FAILED)
			{
				printf("Error occuerd in server receving data, error num : % ld\n", WSAGetLastError());
				return ERROR_CODE;

			}

			if (ERROR_CODE == extract_parameters_from_communication_message(communication_message, parameters_array, message_type)) {

				return ERROR_CODE;
			}

			free_communication_message_and_parameters(communication_message, parameters_array, message_type);



			//send back SERVER_DENIED

			char* communication_message = NULL;
			if (ERROR_CODE == format_communication_message(SERVER_DENIED, parameters_array, &communication_message)) {
				return ERROR_CODE;
			}

			if (SendBuffer(communication_message, get_size_of_communication_message(communication_message), AcceptSocket) == TRNS_FAILED) {
				printf("Failed to send messeage!\n");
				return ERROR_CODE;
			}

			free(communication_message);
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

//when user enters exit, this function will signal to caller.
DWORD check_for_exit_input(exit_thread_parameters* server_exiting) {
	char* user_input;
	while (1) {
		user_input = getline();
		if (strcmp(user_input, "exit")==0) {
			server_exiting->has_user_entered_exit = 1;
			closesocket(server_exiting->socket_to_close);
			free(user_input);
			break;

		}
		

		free(user_input);

	}
	return 1;
}

void cleanup_worker_threads_and_sockets(HANDLE ThreadHandles[NUM_OF_WORKER_THREADS], SOCKET ThreadInputs[NUM_OF_WORKER_THREADS])
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			
			if (ERROR_CODE == closesocket(ThreadInputs[Ind])) {
				printf("error %ld at closesocket( )\n", WSAGetLastError());
				}
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
			
		}
	}
}
