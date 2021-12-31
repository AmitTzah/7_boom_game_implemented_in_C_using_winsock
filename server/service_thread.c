//includes functions and structs related to operation of a worker thread(which is project specific). Each thread handles a row from the input file.


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "socket_send_recv.h"
#include "service_thread.h"
#include "file_IO.h"

int server_game_loop(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME]);


DWORD ServiceThread(SOCKET* t_socket) {
	SOCKET accept_socket = *t_socket;

	
	char client_name[MAX_LENGH_OF_CLIENT_NAME];
	
	if (approve_client_request(accept_socket, client_name) == 0) {

		return 0;
	}
	
	if (send_main_menu_to_client_and_try_to_connect_with_another_player(accept_socket) == 0) {

		return 0;
	}


	//Connected with a second player!
	// enter game_loop

	server_game_loop(accept_socket, client_name);



	
	while(1){}

	return 1;
}

//get client request, extract username into the argument client_name, send back SERVER_APPROVED.
//if some api api function fails, return 0, otherwise 1. 
int approve_client_request(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME]) {
	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

	//first get the CLIENT_REQUEST
	if (recv_communication_message(accept_socket, &communication_message) == TRNS_FAILED)
	{
		printf("Error occuerd in server receving data, error num : % ld", WSAGetLastError());
		return 0;

	}
	printf("server recevied message from client: %s\n", communication_message);
	extract_parameters_from_communication_message(communication_message, parameters_array, message_type);
	strcpy_s(client_name, MAX_LENGH_OF_CLIENT_NAME, parameters_array[0]);

	printf("server received from  client %s", communication_message);

	free_communication_message_and_parameters(communication_message, parameters_array, message_type);

	//send back SERVER_APPROVED
	communication_message = format_communication_message(SERVER_APPROVED, parameters_array);
	
	if (SendBuffer(communication_message, get_size_of_communication_message(communication_message), accept_socket) == TRNS_FAILED) {
		printf("Failed to send messeage from client!\n");
		return 0;

	}
	printf("server sent to client %s", communication_message);

	free(communication_message);

	return 1;

}



int send_main_menu_to_client_and_try_to_connect_with_another_player(SOCKET accept_socket) {
	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];


	//send main menu message to client
	communication_message = format_communication_message(SERVER_MAIN_MENU, parameters_array);
	
	if (SendBuffer(communication_message, get_size_of_communication_message(communication_message), accept_socket) == TRNS_FAILED) {
		printf("Failed to send messeage from client!\n");
		return 0;


	}
	printf("server sent to client %s", communication_message);


	//recv CLIENT_DISCONNECT or CLIENT_VERSUS
	if (recv_communication_message(accept_socket, &communication_message) == TRNS_FAILED)
	{
		printf("Error occuerd in server receving data, error num : % ld", WSAGetLastError());
		return 0;

	}
	printf("server recevied message from client: %s\n", communication_message);


	//if CLIENT_DISCONNECT
	if (compare_messages(communication_message, "CLIENT_DISCONNECT\n") == 1) {

		closesocket(accept_socket);

		return 0;

	}
	free(communication_message);

	return 1;
}

int server_game_loop(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME]) {



}
