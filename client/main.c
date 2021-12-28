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


#include "create_and_handle_processes.h"
#include "HardCodedData.h"
#include "file_IO.h"

#pragma comment(lib,"WS2_32")

void main(int argc, char* argv[]) {
	SOCKADDR_IN clientService;
	

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	int result;
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		exit(1);
	}

	SOCKET m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		goto client_cleanup;

	}


	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(argv[1]); //Setting the IP address to connect to
	clientService.sin_port = htons(strtol(argv[2],NULL, 10)); //Setting the port to connect to.

	/*
		AF_INET is the Internet address family.
	*/


	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed to connect.\n");
		goto client_cleanup;

	}

	int bytes_sent = send(m_socket, "hi", 1+ strlen("hi"), 0);

	printf("Client sent: %d bytes to server\n", bytes_sent);
	
	while (1) {}

client_cleanup:


	result = WSACleanup();
	if (result != NO_ERROR) {
		printf("error %ld at WSACleanup( ), ending program.\n", WSAGetLastError());
	}
}