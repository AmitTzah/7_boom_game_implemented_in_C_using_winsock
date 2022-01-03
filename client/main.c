/*
Authors:

Amit Tzah 316062959
Tomer Shimshi 203200480


Project: Ex4
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
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


void get_connection_succeeded_and_failed_and_server_denied_messages(char* connection_succeeded_message, char* connection_failed_message, char* server_denied_message, char* ip, char* port);
int reconnect_or_exit(SOCKET m_socket, const struct sockaddr* name, int namelen, int illegal_command, int server_denied_message);
void get_path_to_log_file(char* path_to_log_file, char* client_name);
int establish_a_connection_with_server(SOCKET m_socket, char* ip, char* port, char* user_name);
char* getline(void);
int isNumber(char s[]);


int write_from_offset_to_log_file;
char client_log_file_name[MAX_LENGTH_OF_PATH_TO_A_FILE];
int server_main_menu(SOCKET m_socket, int illegal_command);
int game_loop(SOCKET m_socket, char* user_name);

char connection_succeeded_message[MAX_LENGH_OF_IP_PORT_MESSAGES];
char connection_failed_message[MAX_LENGH_OF_IP_PORT_MESSAGES];
char server_denied_message[MAX_LENGH_OF_IP_PORT_MESSAGES];

void main(int argc, char* argv[]) {

	write_from_offset_to_log_file = 0;
	get_path_to_log_file(client_log_file_name, argv[3]);

	get_connection_succeeded_and_failed_and_server_denied_messages(connection_succeeded_message, connection_failed_message, server_denied_message, argv[1], argv[2]);



	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	int result;
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		goto client_cleanup;
	}

	SOCKET m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		goto client_cleanup;

	}

	
	//Attempt to connect with server
	if (ERROR_CODE == establish_a_connection_with_server(m_socket, argv[1], argv[2], argv[3])) {
		//failed to connect or server denied
		goto client_cleanup;
	}

	//SERVER_APPROVED!
	//get main menu
	if (ERROR_CODE== server_main_menu(m_socket, 0)) {
		//Chose to quit or some api function failed.
		goto client_cleanup;
	}
	
	//Received game_started	
	//Start the game loop
	if (ERROR_CODE == game_loop(m_socket, argv[3])) {

		goto client_cleanup;

	}

	
	
	while (1) {}

client_cleanup:

	
	result = WSACleanup();
	if (result != NO_ERROR) {
		printf("error %ld at WSACleanup( ), ending program.\n", WSAGetLastError());
	}

}

//sends client request to server and receives a response (denied or approved)
//returns ERROR_CODE if fails acutely or user chooses to exit, in that case goto client_cleanup in caller
int establish_a_connection_with_server(SOCKET m_socket, char* ip, char* port, char* user_name) {
	int tried_to_reconnect = 0;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char* communication_message = NULL;
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

	SOCKADDR_IN clientService;

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(ip); //Setting the IP address to connect to
	clientService.sin_port = htons(strtol(port, NULL, 10)); //Setting the port to connect to.

	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		tried_to_reconnect = 1;
		if (1 == reconnect_or_exit(m_socket, (SOCKADDR*)&clientService, sizeof(clientService), 0, 0)) {
			//user chose to exit.
			return ERROR_CODE;

		}

		//else, reconnected!
	}

	if (tried_to_reconnect != 1) {
		WinWriteToFile(client_log_file_name, connection_succeeded_message, strlen(connection_succeeded_message), write_from_offset_to_log_file);
		printf("%s", connection_succeeded_message);
		write_from_offset_to_log_file += strlen(connection_succeeded_message);
	}

	//send CLIENT_REQUEST
	parameters_array[0] = user_name;

	if (ERROR_CODE == send_message(m_socket, CLIENT_REQUEST, parameters_array)) {
		return ERROR_CODE;

	}

	// recv SERVER_DENIED or SERVER_APPROVED
	if (ERROR_CODE == recv_and_extract_communication_message(m_socket, &communication_message, message_type, parameters_array)) {

		return ERROR_CODE;

	}


	//if server denied
	if (strcmp(message_type, SERVER_DENIED) == 0) {

		if (1 == reconnect_or_exit(m_socket, (SOCKADDR*)&clientService, sizeof(clientService), 0, 1)) {
			//user chose to exit.
			return ERROR_CODE;

		}
		//else user reconnected
	}

	free_communication_message_and_parameters(communication_message, parameters_array, message_type);

	//SERVER_APPROVED!
	return 0;

}

//Recv main_menu from server
//returns ERROR_CODE if fails acutely or user chooses to quit, in that case goto client_cleanup in caller.
//returns 0 after sending CLIENT_VERSUS(chossing to play).
int server_main_menu(SOCKET m_socket, int illegal_command) {
	char* communication_message = NULL;
	char choice[MAX_LENGH_OF_IP_PORT_MESSAGES];
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

	// recv main menu message
	if (illegal_command != 1) {
		
		printf("client waiting for main menue message...\n");

		if (ERROR_CODE == recv_and_extract_communication_message(m_socket, &communication_message, message_type, parameters_array)) {

			return ERROR_CODE;

		}

	}
	free_communication_message_and_parameters(communication_message, parameters_array, message_type);

	printf("Choose what to do next:\n");
	printf("1. Play against another client\n");
	printf("2. Quit\n");
	fgets(choice, MAX_LENGH_OF_IP_PORT_MESSAGES, stdin);

	if (choice[0] == '1') {
		//send CLIENT_VERSUS
		if (ERROR_CODE == send_message(m_socket, CLIENT_VERSUS, parameters_array)) {
			return ERROR_CODE;

		}

		//receive GAME_STARTED or server_no_opponents
		if (ERROR_CODE == recv_and_extract_communication_message(m_socket, &communication_message, message_type, parameters_array)) {

			return ERROR_CODE;

		}
		//if received GAME_STARTED.
		if (strcmp(message_type, GAME_STARTED) == 0) {
			
			//found another player, go to game loop.
			return SUCCESS_CODE;

		}
		free_communication_message_and_parameters(communication_message, parameters_array, message_type);


		//Received SERVER_NO_OPPONENTS
		
		//GET main menu again
		return server_main_menu(m_socket, 0);

	}

	else if (choice[0] == '2') {

		//send CLIENT_DISCONNECT

		if (ERROR_CODE == send_message(m_socket, CLIENT_DISCONNECT, parameters_array)) {
			return ERROR_CODE;

		}

		return ERROR_CODE;
	}

	else {

		printf("Error: illegal command\n");
		return server_main_menu(m_socket, 1);
	}
}



// trying to reconnect recursively.
// if illegal_command=1, then the function was called user entered a wrong input, and connection failed messeage won't be print to screen and file.
// if server_denied_message=1 the message for server denied will by printed.
//Return 1 if user chose to Exit.
//return 0 if succeeded to reconnect.
int reconnect_or_exit(SOCKET m_socket, const struct sockaddr* name, int namelen, int illegal_command, int is_server_denied_message) {
	char choice[MAX_LENGH_OF_IP_PORT_MESSAGES];

	if (illegal_command == 0) {
		if (is_server_denied_message == 0) {
			//print and write to file the connction failed message

			printf("%s", connection_failed_message);
			WinWriteToFile(client_log_file_name, connection_failed_message, strlen(connection_failed_message), write_from_offset_to_log_file);
			write_from_offset_to_log_file += strlen(connection_failed_message);
		}

		else {

			//print and write to file the server denied message
			printf("%s", server_denied_message);
			WinWriteToFile(client_log_file_name, server_denied_message, strlen(server_denied_message), write_from_offset_to_log_file);
			write_from_offset_to_log_file += strlen(server_denied_message);

		}
	}
	printf("Choose what to do next:\n");
	printf("1. Try to reconnect\n");
	printf("2. Exit\n");
	fgets(choice, MAX_LENGH_OF_IP_PORT_MESSAGES, stdin);
	
	if (choice[0] == '1') {
		if (connect(m_socket, name, namelen) == SOCKET_ERROR) {

			return reconnect_or_exit(m_socket, name, namelen,0,0);

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
		return reconnect_or_exit(m_socket, name, namelen,1,0);
	}

}


int game_loop(SOCKET m_socket, char* user_name) {
	printf("Game is on!\n");

	char* user_input = NULL;
	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

	//while game hasn't finished
	while (1) {

		//receive TURN_SWITCH
		if (ERROR_CODE == recv_and_extract_communication_message(m_socket, &communication_message, message_type, parameters_array)) {

			return ERROR_CODE;

		}
		//check if it's this player's turn
		if (strcmp(parameters_array[0], user_name) == 0) {
			printf("Your turn!\n");
			free_communication_message_and_parameters(communication_message, parameters_array, message_type);

			//receive SERVER_MOVE_REQUEST
			if (ERROR_CODE == recv_and_extract_communication_message(m_socket, &communication_message, message_type, parameters_array)) {

				return ERROR_CODE;
			}
			free_communication_message_and_parameters(communication_message, parameters_array, message_type);

			//get move from user.
			printf("Enter the next number or boom:\n");
			user_input = getline();

			//if not a number or boom
			while (isNumber(user_input) == 0 && strcmp(user_input, "boom")!=0 ) {
				free(user_input);
			printf("Error: illegal command\n");
			printf("Enter the next number or boom:\n");
			user_input = getline();

			}

			//send CLIENT_PLAYER_MOVE

			parameters_array[0] = user_input;
			if (ERROR_CODE == send_message(m_socket, CLIENT_PLAYER_MOVE, parameters_array)) {
				return ERROR_CODE;
			}
			free(user_input);
		}


		//Other's player move
		else {
			printf("%s\'s turn!\n", parameters_array[0]);
			free_communication_message_and_parameters(communication_message, parameters_array, message_type);

		}

	}
	return SUCCESS_CODE;
}



//Puts the correct file name format into path_to_log_file given the client name.
void get_path_to_log_file(char* path_to_log_file, char* client_name) {

	strcpy_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE, "Client_log_");
	strcat_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE, client_name);
	strcat_s(path_to_log_file, MAX_LENGTH_OF_PATH_TO_A_FILE,".txt");


}

//Puts the correct connection_succeeded_and_failed_messages into the given array, by using the ip an port in string formats.
void get_connection_succeeded_and_failed_and_server_denied_messages(char* connection_succeeded_message, char* connection_failed_message, char* server_denied_message,char* ip, char* port) {

	
	strcpy_s(connection_succeeded_message, MAX_LENGH_OF_IP_PORT_MESSAGES, "Connected to server on ");
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_IP_PORT_MESSAGES, ip);
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_IP_PORT_MESSAGES, ":");
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_IP_PORT_MESSAGES, port);
	strcat_s(connection_succeeded_message, MAX_LENGH_OF_IP_PORT_MESSAGES, "\n");


	strcpy_s(connection_failed_message, MAX_LENGH_OF_IP_PORT_MESSAGES, "Failed connecting to server on ");
	strcat_s(connection_failed_message, MAX_LENGH_OF_IP_PORT_MESSAGES, ip);
	strcat_s(connection_failed_message, MAX_LENGH_OF_IP_PORT_MESSAGES, ":");
	strcat_s(connection_failed_message, MAX_LENGH_OF_IP_PORT_MESSAGES, port);
	strcat_s(connection_failed_message, MAX_LENGH_OF_IP_PORT_MESSAGES, "\n");

	strcpy_s(server_denied_message, MAX_LENGH_OF_IP_PORT_MESSAGES, "Server on ");
	strcat_s(server_denied_message, MAX_LENGH_OF_IP_PORT_MESSAGES, ip);
	strcat_s(server_denied_message, MAX_LENGH_OF_IP_PORT_MESSAGES, ":");
	strcat_s(server_denied_message, MAX_LENGH_OF_IP_PORT_MESSAGES, port);
	strcat_s(server_denied_message, MAX_LENGH_OF_IP_PORT_MESSAGES, " denied the connection request.\n");


}

//get input from user of variable length
//reads untill enter is pressed
// return pointer to the read string allocated on heap
//Memory is dynamically allocated, thus needs to be freed in caller.
//source:
//https://stackoverflow.com/questions/314401/how-to-read-a-line-from-the-console-in-c
char* getline(void) {
	char* line = malloc(100), * linep = line;
	size_t lenmax = 100, len = lenmax;
	int c;

	if (line == NULL)
		return NULL;

	for (;;) {
		c = fgetc(stdin);
		if (c == EOF)
			break;

		if (--len == 0) {
			len = lenmax;
			char* linen = realloc(linep, lenmax *= 2);

			if (linen == NULL) {
				free(linep);
				return NULL;
			}
			line = linen + (line - linep);
			linep = linen;
		}

		if ((*line++ = c) == '\n')
			break;
	}
	*(line-1) = '\0';
	return linep;
}


//check if string is a number
//based on https://www.codegrepper.com/code-examples/c/check+if+string+is+number+c
int isNumber(char s[])
{
	for (int i = 0; s[i] != '\0'; i++)
	{
		if (isdigit(s[i]) == 0)
			return 0;
	}
	return 1;
}