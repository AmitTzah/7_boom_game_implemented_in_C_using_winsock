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

void main(int argc, char* argv[]) {
	
	int result;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	TransferResult_t send_recv_result;
	char* communication_message=NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];

	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		exit(1);
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

	while (1)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{	
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup;
		}

		printf("Client Connected.\n");

		int Ind = find_index_of_unused_thread(ThreadHandles, NUM_OF_WORKER_THREADS);

		// if 2 clients are already connected
		if (Ind == NUM_OF_WORKER_THREADS) 
		{
			//first get the CLIENT_REQUEST
			if (recv_communication_message(AcceptSocket, &communication_message) == TRNS_FAILED)
			{
				printf("Error occuerd in server receving data, error num : % ld", WSAGetLastError());
				goto server_cleanup;
			}
			free(communication_message);

			//send back SERVER_DENIED
			communication_message = format_communication_message(SERVER_DENIED, parameters_array);
			send_recv_result = SendBuffer(communication_message, get_size_of_communication_message(communication_message), AcceptSocket);
			if (send_recv_result == TRNS_FAILED) {
				printf("Failed to send messeage from client!\n");
				goto server_cleanup;
			}

			printf("server sent to client %s", communication_message);

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
		}
	} 






	


server_cleanup:
	
	result = WSACleanup();
	if (result != NO_ERROR) {
		printf("error %ld at WSACleanup( ), ending program.\n", WSAGetLastError());
	}

}