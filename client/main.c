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

#include "socket_send_recv.h"
#include "create_and_handle_processes.h"
#include "HardCodedData.h"
#include "file_IO.h"

#pragma comment(lib,"WS2_32")


int reconnect_or_exit(SOCKET m_socket, const struct sockaddr* name, int namelen, int illegal_command);
void get_path_to_log_file(char* path_to_log_file, char* client_name);
void get_connection_succeeded_and_failed_messages(char* connection_succeeded_message, char* connection_failed_message, char* ip, char* port);

int write_from_offset_to_log_file;
char client_log_file_name[MAX_LENGTH_OF_PATH_TO_A_FILE];

char connection_succeeded_message[MAX_LENGH_OF_INPUT_FROM_USER];
char connection_failed_message[MAX_LENGH_OF_INPUT_FROM_USER];

void main(int argc, char* argv[]) {
	SOCKADDR_IN clientService;
	TransferResult_t send_recv_result;
	int tried_to_reconnect = 0;
	write_from_offset_to_log_file = 0;
	get_path_to_log_file(client_log_file_name, argv[3]);
	printf("the client_log_file_name is: %s\n", client_log_file_name);

	get_connection_succeeded_and_failed_messages(connection_succeeded_message, connection_failed_message, argv[1], argv[2]);
	printf("the connection_succeeded_message is: %s", connection_succeeded_message);
	printf("the connection_failed_message is: %s", connection_failed_message);


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
		tried_to_reconnect = 1;
		if (1 == reconnect_or_exit(m_socket, (SOCKADDR*)&clientService, sizeof(clientService),0)) {
			//user chose to exit.
			goto client_cleanup;

		}

		//else, reconnected!
	}

	if (tried_to_reconnect != 1) {
		WinWriteToFile(client_log_file_name, connection_succeeded_message, strlen(connection_succeeded_message), write_from_offset_to_log_file);
		printf("%s", connection_succeeded_message);
		write_from_offset_to_log_file += strlen(connection_succeeded_message);
	}
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	parameters_array[0] = "SERVER_NO_OPONENTS";
	char* messeage_to_send=format_communication_message("SERVER_NO_OPONENTS", parameters_array);

	send_recv_result=SendBuffer(messeage_to_send, get_size_of_communication_message(messeage_to_send), m_socket);
	if (send_recv_result == TRNS_FAILED) {
		printf("Failed to send messeage from client!\n");
		goto client_cleanup;
	}

	printf("Client sent: %d bytes to server\n", get_size_of_communication_message(messeage_to_send));
	free(messeage_to_send);
	
	while (1) {}

client_cleanup:

	//reconnect_or_exit()

	result = WSACleanup();
	if (result != NO_ERROR) {
		printf("error %ld at WSACleanup( ), ending program.\n", WSAGetLastError());
	}
}


// trying to reconnect recursively.
// if illegal_command=1, then the function was called user entered a wrong input, and connection failed messeage won't be print to screen and file.
//Return 1 if user chose to Exit.
//return 0 if succeeded to reconnect.
int reconnect_or_exit(SOCKET m_socket, const struct sockaddr* name, int namelen, int illegal_command) {
	char choice[MAX_LENGH_OF_INPUT_FROM_USER];

	if (illegal_command == 0) {
		printf("%s", connection_failed_message);
		WinWriteToFile(client_log_file_name, connection_failed_message, strlen(connection_failed_message), write_from_offset_to_log_file);
		write_from_offset_to_log_file += strlen(connection_failed_message);
	}
	printf("Choose what to do next:\n");
	printf("1. Try to reconnect\n");
	printf("2. Exit\n");
	fgets(choice, MAX_LENGH_OF_INPUT_FROM_USER, stdin);
	
	if (choice[0] == '1') {
		if (connect(m_socket, name, namelen) == SOCKET_ERROR) {

			return reconnect_or_exit(m_socket, name, namelen,0);

		}

		else {
			printf("%s", connection_succeeded_message);
			WinWriteToFile(client_log_file_name, connection_succeeded_message, strlen(connection_succeeded_message), write_from_offset_to_log_file);
			write_from_offset_to_log_file += strlen(connection_succeeded_message);

			return 0;

		}
	}

	else if (choice[0] == '2') {

		return 1;

	}

	else {

		printf("Error: illegal command\n");
		return reconnect_or_exit(m_socket, name, namelen,1);
	}

}

//Puts the correct file name format into path_to_log_file given the client name.
void get_path_to_log_file(char* path_to_log_file, char* client_name) {

	strcpy_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE, "Client_log_");
	strcat_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE, client_name);
	strcat_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE,".txt");


}

//Puts the correct connection_succeeded_and_failed_messages into the given array, by using the ip an port in string formats.
void get_connection_succeeded_and_failed_messages(char* connection_succeeded_message, char* connection_failed_message, char* ip, char* port) {

	
	strcpy_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, "Connected to server on ");
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, ip);
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, ":");
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, port);
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_INPUT_FROM_USER, "\n");


	strcpy_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, "Failed connecting to server on ");
	strcat_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, ip);
	strcat_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, ":");
	strcat_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, port);
	strcat_s(connection_failed_message, MAX_LENGH_OF_INPUT_FROM_USER, "\n");

}