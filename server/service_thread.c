//includes functions and structs related to operation of a worker thread(which is project specific). 


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "socket_send_recv.h"
#include "service_thread.h"
#include "file_IO.h"



int server_game_loop(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]);
int approve_client_request(SOCKET accept_socket, char client_name[MAX_LENGH_OF_CLIENT_NAME]);
int send_main_menu_to_client_and_try_to_connect_with_another_player(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]);
int check_if_two_players_are_ready_to_play_protected(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]);
int check_if_move_has_finished_the_game(char* player_guess, int* game_has_finished);

int read_write_common_resources_protected(int index_of_parameter_to_access, int read_or_write, int int_data_to_write, char* name_str_to_write, int* int_read,
	char* name_str_read, int increase_or_decrease_by_one);

int check_if_player_connected_first_and_update_num_of_players(int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]);

extern shared_server_resources resources_struct;
extern HANDLE ghMutex;
extern HANDLE mutex_to_sync_threads_when_waiting_for_players;
extern HANDLE event_for_syncing_threads_in_game_loop;

DWORD ServiceThread(SOCKET* t_socket) {
	SOCKET accept_socket = *t_socket;
	int num_of_player;

	char client_name[MAX_LENGH_OF_CLIENT_NAME];
		
	if (approve_client_request(accept_socket, client_name) == ERROR_CODE) {

		return ERROR_CODE;
	}
	
	if (send_main_menu_to_client_and_try_to_connect_with_another_player(accept_socket, & num_of_player, client_name) == ERROR_CODE) {

		return ERROR_CODE;
	}


	//Connected with a second player!
	//enter game_loop
	
	server_game_loop(accept_socket, &num_of_player, client_name);


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
int send_main_menu_to_client_and_try_to_connect_with_another_player(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]) {
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
if (check_if_player_connected_first_and_update_num_of_players(num_of_player, client_name) == ERROR_CODE) {

	return ERROR_CODE;
}

//wait for another client to connect
Sleep(WAIT_FOR_RESPONSE);


//if 2 players are ready
return check_if_two_players_are_ready_to_play_protected(accept_socket, num_of_player, client_name);

}

//if some api api function fails, return ERROR_CODE, otherwise 0. 
int server_game_loop(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]) {
	char* communication_message = NULL;
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	char message_type[MAX_LENGH_OF_MESSAGE_TYPE];
	int my_client_turn;
	char other_client_name[MAX_LENGH_OF_CLIENT_NAME];
	//get other client's name
	if (*num_of_player == 1) {
		read_write_common_resources_protected(2, 0, -1, NULL, NULL, other_client_name, -1);

	}

	else {

		read_write_common_resources_protected(1, 0, -1, NULL, NULL, other_client_name, -1);

	}

	//check if this thread should make the first move
	if (*num_of_player == 1) {
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

			//Ask client for move
			if (ERROR_CODE == send_message(accept_socket, SERVER_MOVE_REQUEST, parameters_array)) {
				return ERROR_CODE;

			}

			//Recv from client CLIENT_MOVE REQUEST
			if (ERROR_CODE == recv_and_extract_communication_message(accept_socket, &communication_message, message_type, parameters_array)) {

				return ERROR_CODE;

			}


			//check_if_move_has_finished_the_game(current game_number, player guess);
			// if game hasn't ennded, update he game_number with the player's guess.

			//Next turn is not mine.
			my_client_turn = 0;

			//finished turn, signal to the other thread that is waiting.
			if (SetEvent(event_for_syncing_threads_in_game_loop) == 0) {
				printf("SetEvent() failed in server\n");

				return ERROR_CODE;
			}

			
			//send_game view to client according to result

			free_communication_message_and_parameters(communication_message, parameters_array, message_type);

		}

		else {
			//Other client turn
			//send the other client's name.

			//Send turn switch to client with this client's name.
			parameters_array[0] = other_client_name;
			if (ERROR_CODE == send_message(accept_socket, TURN_SWITCH, parameters_array)) {
				return ERROR_CODE;

			}
			//next turn is mine
			my_client_turn = 1;

			WaitForSingleObject(event_for_syncing_threads_in_game_loop, INFINITE);
			//send game_view to client

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

	if (dwWaitResult == WAIT_OBJECT_0) {

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


				else if (index_of_parameter_to_access == 4) {

					*int_read = resources_struct.game_number;
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

			else if (index_of_parameter_to_access == 1) {

				strcpy_s(resources_struct.player_1_name, MAX_LENGH_OF_CLIENT_NAME, name_str_to_write);

			}

			else if (index_of_parameter_to_access == 2) {

				strcpy_s(resources_struct.player_2_name, MAX_LENGH_OF_CLIENT_NAME, name_str_to_write);

			}

			else
			{

			}

		}
			
	}

		
	else {

		printf("WaitForSingleObject failed in read_write_common_resources_protected()\n");

		return ERROR_CODE;
	}

	// Release ownership of the mutex object
	if (!ReleaseMutex(ghMutex))
	{
			printf("Release mutex failed!");
			return ERROR_CODE;
	}

	return SUCCESS_CODE;

}


//Also writes the name of the client to player_1_name if arrived first, otherwise to player_2_name
//if some api api function fails, return ERROR_CODE, otherwise 0. 
int check_if_player_connected_first_and_update_num_of_players(int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]) {
	
	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no time-out interval

	if (dwWaitResult == WAIT_OBJECT_0) {
	
			//read write protected
			if (resources_struct.first_arrived == 0) {
				resources_struct.first_arrived = 1;
				*num_of_player = 1;
				strcpy_s(resources_struct.player_1_name, MAX_LENGH_OF_CLIENT_NAME, client_name);

			}

			else {

				*num_of_player = 2;
				strcpy_s(resources_struct.player_2_name, MAX_LENGH_OF_CLIENT_NAME, client_name);

			}
			
			resources_struct.num_of_players_ready_to_play++;

		}

	else {

		printf("WaitForSingleObject failed in check_if_two_players_are_ready_to_play_protected()\n");

		return ERROR_CODE;
	}


	if (!ReleaseMutex(ghMutex))
	{
		printf("Release mutex failed!");
		return ERROR_CODE;
	}
	return SUCCESS_CODE;

}

//this function checks if two threads are ready to play.
//If no, calls revursively to send_main_menu_to_client_and_try_to_connect_with_another_player()
int check_if_two_players_are_ready_to_play_protected(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]) {

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
			//num_of_players_ready_to_play--
			read_write_common_resources_protected(3, 1, -1, NULL, NULL, NULL, 1);

			return ERROR_CODE;

		}

		//decrease num_of_players_ready_to_play by 1
		read_write_common_resources_protected(3, 1, -1, NULL, NULL, NULL, 1);

		//set first_arrived to 0
		int first_arrived_int_data_to_write = 0;
		read_write_common_resources_protected(0, 1, first_arrived_int_data_to_write, NULL, NULL, NULL, -1);


		if (!ReleaseMutex(mutex_to_sync_threads_when_waiting_for_players))
		{
			printf("Release mutex failed!");
			return ERROR_CODE;
		}
		return send_main_menu_to_client_and_try_to_connect_with_another_player(accept_socket, num_of_player, client_name);
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



//if some api api function fails, return ERROR_CODE, otherwise 0. 
int check_if_move_has_finished_the_game(char* player_guess, int* game_has_finished) {

	int game_number_read;
	

	if (read_write_common_resources_protected(4, 0,-1,NULL, &game_number_read,NULL,-1) == ERROR_CODE) {

		return ERROR_CODE;

	 }
	 
	if(game_number_read)




	return SUCCESS_CODE;
}