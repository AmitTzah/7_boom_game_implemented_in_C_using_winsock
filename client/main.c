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
void get_path_to_log_file(char* path_to_log_file, char* client_name);
void get_connection_succeeded_and_failed_messages(char* connection_succeeded_message, char* connection_failed_message, char* ip, char* port);

int bytes_offset_to_log_file;
char client_log_file_name[MAX_LENGTH_OF_PATH_TO_A_FILE];

char connection_succeeded_message[MAX_LENGH_OF_INPUT_FROM_USER];
char connection_failed_message[MAX_LENGH_OF_INPUT_FROM_USER];

void main(int argc, char* argv[]) {
	SOCKADDR_IN clientService;
	
	bytes_offset_to_log_file = 0;
	get_path_to_log_file(client_log_file_name, argv[3]);
	printf("the client_log_file_name is: %s\n", client_log_file_name);

	get_connection_succeeded_and_failed_messages(connection_succeeded_message, connection_failed_message, argv[1], argv[2]);
	printf("the connection_succeeded_message is: %s\n", connection_succeeded_message);
	printf("the connection_failed_message is: %s\n", connection_failed_message);


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
//return 0 if succeeded to reconnect.
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
			//WinWriteToFile(client_log_file_name,)
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

void get_path_to_log_file(char* path_to_log_file, char* client_name) {

	strcpy_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE, "Client_log_");
	strcat_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE, client_name);
	strcat_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE,".txt");


}
void get_connection_succeeded_and_failed_messages(char* connection_succeeded_message, char* connection_failed_message, char* ip, char* port) {

	
	strcpy_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, "Connected to server on ");
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, ip);
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, ":");
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, port);

	strcpy_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, "Failed connecting to server on ");
	strcat_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, ip);
	strcat_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, ":");
	strcat_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, port);

}