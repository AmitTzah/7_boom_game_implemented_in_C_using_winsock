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


int reconnect_or_exit(SOCKET m_socket, const struct sockaddr* name, int namelen, char* ip, char* port);

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
			
		if (1 == reconnect_or_exit(m_socket, (SOCKADDR*)&clientService, sizeof(clientService), argv[1], argv[2])) {
			//user chose to exit.
			goto client_cleanup;

		}

		//else, reconnected!
	}

	printf("Connected to server on %s:%s\n", argv[1], argv[2]);

	int bytes_sent = send(m_socket, "hi", 1+ strlen("hi"), 0);

	printf("Client sent: %d bytes to server\n", bytes_sent);
	
	while (1) {}

client_cleanup:

	//reconnect_or_exit()

	result = WSACleanup();
	if (result != NO_ERROR) {
		printf("error %ld at WSACleanup( ), ending program.\n", WSAGetLastError());
	}
}


// trying to reconnect recursively.
//Return 1 if user chose to Exit.
//return 0 if Succedded to reconnect.
int reconnect_or_exit(SOCKET m_socket, const struct sockaddr* name, int namelen, char * ip, char* port) {
	char choice[MAX_LENGH_OF_INPUT_FROM_USER];

	printf("Failed connecting to server on %s:%s.\n", ip, port);
	printf("Choose what to do next:\n");
	printf("1. Try to reconnect\n");
	printf("2. Exit\n");
	fgets(choice, MAX_LENGH_OF_INPUT_FROM_USER, stdin);
	
	if (choice[0] == '1') {
		if (connect(m_socket, name, namelen) == SOCKET_ERROR) {

			return reconnect_or_exit(m_socket, name, namelen, ip, port);

		}

		else {

			return 0;

		}
	}

	else if (choice[0] == '2') {

		return 1;

	}

	else {

		printf("Error: illegal command\n");
		return reconnect_or_exit(m_socket, name, namelen, ip, port);
	}

}