//includes functions and structs related to operation of a worker thread(which is project specific). 


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "socket_send_recv.h"
#include "service_thread.h"
#include "file_IO.h"



int server_game_loop(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME], int* first_to_connect);
int approve_client_request(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME]);
int send_main_menu_to_client_and_try_to_connect_with_another_player(SOCKET accept_socket, int* num_of_player);
int check_if_two_players_are_ready_to_play_protected(SOCKET accept_socket, int* num_of_player);

int read_write_common_resources_protected(int index_of_parameter_to_access, int read_or_write, int int_data_to_write, char* name_str_to_write, int* int_read,
	char* name_str_read, int increase_or_decrease_by_one);

int check_if_player_connected_first_and_update_num_of_players(int* num_of_player);

extern shared_server_resources resources_struct;
extern HANDLE ghMutex;
extern HANDLE mutex_to_sync_threads_when_waiting_for_players;

DWORD ServiceThread(SOCKET* t_socket) {
	SOCKET accept_socket = *t_socket;
	int num_of_player;

	char client_name[MAX_LENGH_OF_CLIENT_NAME];

		
	if (approve_client_request(accept_socket, client_name) == ERROR_CODE) {

		return ERROR_CODE;
	}
	
	if (send_main_menu_to_client_and_try_to_connect_with_another_player(accept_socket, & num_of_player) == ERROR_CODE) {

		return ERROR_CODE;
	}


	//Connected with a second player!
	//enter game_loop
	int first_to_connect;
	server_game_loop(accept_socket, client_name, &first_to_connect);


	return 1;
}

//get client request, extract username into the argument client_name, send back SERVER_APPROVED.
//if some api api function fails, return ERROR_CODE, otherwise 0. 
int approve_client_request(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME]) {
	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

	//first get the CLIENT_REQUEST
	if (ERROR_CODE == recv_and_extract_communication_message(accept_socket, &communication_message, message_type, parameters_array)) {

		return ERROR_CODE;

	}
	strcpy_s(client_name, MAX_LENGH_OF_CLIENT_NAME, parameters_array[0]);
	free_communication_message_and_parameters(communication_message, parameters_array, message_type);

	//send back SERVER_APPROVED

	if (ERROR_CODE == send_message(accept_socket, SERVER_APPROVED, parameters_array)) {
		return ERROR_CODE;

	}

	return 0;

}


//if some api api function fails, return ERROR_CODE, otherwise 0. 
int send_main_menu_to_client_and_try_to_connect_with_another_player(SOCKET accept_socket, int* num_of_player) {
	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];


	//send main menu message to client

	if (ERROR_CODE == send_message(accept_socket, SERVER_MAIN_MENU, parameters_array)) {
		return ERROR_CODE;

	}


	//recv CLIENT_DISCONNECT or CLIENT_VERSUS

	if (ERROR_CODE == recv_and_extract_communication_message(accept_socket, &communication_message, message_type, parameters_array)) {

		return ERROR_CODE;

	}

	//if CLIENT_DISCONNECT
	if (strcmp(message_type, CLIENT_DISCONNECT) == 0) {

		closesocket(accept_socket);
		free_communication_message_and_parameters(communication_message, parameters_array, message_type);

		return ERROR_CODE;
	}

	free_communication_message_and_parameters(communication_message, parameters_array, message_type);

	// recv CLIENT_VERSUS!
	//client chose to play!
	if (check_if_player_connected_first_and_update_num_of_players(num_of_player) == ERROR_CODE) {

		return ERROR_CODE;
	}

	//wait for another client to connect
	Sleep(WAIT_FOR_RESPONSE);


	//if 2 players are ready
	return check_if_two_players_are_ready_to_play_protected(accept_socket, num_of_player);

}

//if some api api function fails, return ERROR_CODE, otherwise 0. 
int server_game_loop(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME], int* first_to_connect) {
	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

	//1 if this client plays next.
	int my_client_turn = 0;

	//check if this thread should make the first move
	if (*first_to_connect == 1) {
		my_client_turn = 1;

	}

	else {
		//Other client should make the first move.
		my_client_turn = 0;
	}

	//while game is still on
	while (1) {

		if (my_client_turn == 1) {

			//Send turn switch to client with this client's name.
			parameters_array[0] = client_name;
			if (ERROR_CODE == send_message(accept_socket, TURN_SWITCH, parameters_array)) {
				return ERROR_CODE;

			}

		}

		else {
			//Other client turn
			//send the other client's name.


		}




	}


	return SUCCESS_CODE;
}


//This function should be called whenever a thread needs to access the shared database, which is the global struct resources_struct;
// Read or write mutex-protected.
//if an int argument is not needed, -1 should be passed. 
//If char* argument is not needed, NULL should be passed.
//Mmeory for data read should be allocated in caller.
// if increase_or_decrease_by_one==1 , the int parmeter to access will be increased by 1. If 0, will be decrease by 1.
//if some api api function fails, return ERROR_CODE, otherwise 0. 
//read_or_write 0 for read, 1 fore write
int read_write_common_resources_protected(int index_of_parameter_to_access, int read_or_write, int int_data_to_write, char* name_str_to_write, int* int_read,
										char *name_str_read, int increase_or_decrease_by_one) {
	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no time-out interval

	switch (dwWaitResult)
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		__try {
			//read or write to database

			//if asked to read
			if (read_or_write == 0) {

				if (index_of_parameter_to_access == 0) {
					*int_read = resources_struct.first_arrived;
				}

				//read player1 name
				else if (index_of_parameter_to_access == 1) {
					strcpy_s(name_str_read, MAX_LENGH_OF_CLIENT_NAME, resources_struct.player_1_name);

				}

				//read player2 name
				else if (index_of_parameter_to_access == 2) {
					strcpy_s(name_str_read, MAX_LENGH_OF_CLIENT_NAME, resources_struct.player_2_name);

				}

				else if (index_of_parameter_to_access == 3) {
					
					*int_read = resources_struct.num_of_players_ready_to_play;
				}


				else {

				}

			}

			else {
				//asked to write

				if (index_of_parameter_to_access == 3) {

					if (increase_or_decrease_by_one != -1) {
						if (increase_or_decrease_by_one == 0) {

							resources_struct.num_of_players_ready_to_play++;
						}

						else {

							resources_struct.num_of_players_ready_to_play--;
						}

					}
				}

				else if (index_of_parameter_to_access == 0) {

					resources_struct.first_arrived = int_data_to_write;
				}

				else
				{

				}

			}
			
		}

		__finally {
			// Release ownership of the mutex object
			if (!ReleaseMutex(ghMutex))
			{
				printf("Release mutex failed!");
				return ERROR_CODE;
			}
		}

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
	case WAIT_ABANDONED:
		return ERROR_CODE;
	}

	return SUCCESS_CODE;

}


int check_if_player_connected_first_and_update_num_of_players(int* num_of_player) {
	
	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject(
		mutex_to_sync_threads_when_waiting_for_players,    // handle to mutex
		INFINITE);  // no time-out interval

	if (dwWaitResult == WAIT_OBJECT_0) {
	
			//read write protected
			if (resources_struct.first_arrived == 0) {
				resources_struct.first_arrived = 1;
				*num_of_player = 1;


			}

			else {

				*num_of_player = 2;
			}
			
			resources_struct.num_of_players_ready_to_play++;

		}

	else {

		printf("WaitForSingleObject failed in check_if_two_players_are_ready_to_play_protected()\n");

		return ERROR_CODE;
	}


	if (!ReleaseMutex(mutex_to_sync_threads_when_waiting_for_players))
	{
		printf("Release mutex failed!");
		return ERROR_CODE;
	}
	return SUCCESS_CODE;

}


int check_if_two_players_are_ready_to_play_protected(SOCKET accept_socket, int* num_of_player) {

	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject(
		mutex_to_sync_threads_when_waiting_for_players,    // handle to mutex
		INFINITE);  // no time-out interval

	if (dwWaitResult == WAIT_OBJECT_0) {

	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];

	int num_of_players_ready_to_play;
	//read num_of_players_ready_to_play;
	read_write_common_resources_protected(3, 0, -1, NULL, &num_of_players_ready_to_play, NULL, -1);

	if (num_of_players_ready_to_play == NUM_OF_WORKER_THREADS) {

		
		if (ERROR_CODE == send_message(accept_socket, GAME_STARTED, parameters_array)) {
			return ERROR_CODE;

		}

		//can start the game, go to game loop
	}
	else {

		if (ERROR_CODE == send_message(accept_socket, SERVER_NO_OPPONENTS, parameters_array)) {
			num_of_players_ready_to_play--;
			return ERROR_CODE;

		}

		//decrease num_of_players_ready_to_play by 1
		read_write_common_resources_protected(3, 1, -1, NULL, NULL, NULL, 1);

		//set first_arrived to 0
		int first_arrived_int_data_to_write = 1;
		read_write_common_resources_protected(0, 1, first_arrived_int_data_to_write, NULL, NULL, NULL, -1);


		if (!ReleaseMutex(mutex_to_sync_threads_when_waiting_for_players))
		{
			printf("Release mutex failed!");
			return ERROR_CODE;
		}
		return send_main_menu_to_client_and_try_to_connect_with_another_player(accept_socket, num_of_player);
	}

	}

	else {

		printf("WaitForSingleObject failed in check_if_two_players_are_ready_to_play_protected()\n");

		return ERROR_CODE;
	}


	if (!ReleaseMutex(mutex_to_sync_threads_when_waiting_for_players))
	{
		printf("Release mutex failed!");
		return ERROR_CODE;
	}


	return SUCCESS_CODE;

}