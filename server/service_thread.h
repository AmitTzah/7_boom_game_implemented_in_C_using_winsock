//includes functions and structs related to operation of a worker thread(which is project specific). 

#ifndef SERVICE_THREAD_H
#define SERVICE_THREAD_H

#include <stddef.h>
#include <stdbool.h>
#include <Windows.h>


#include "HardCodedData.h"
#include "create_and_handle_processes.h"


//order of fields matter!
//add a new field at the end of the struct and update read_write_common_resources_protected() accordingly
typedef struct shared_server_resources {
	int first_arrived;
	char player_1_name[MAX_LENGH_OF_CLIENT_NAME];
	char player_2_name[MAX_LENGH_OF_CLIENT_NAME];
	int num_of_players_ready_to_play;
	int game_number;
	int game_has_ended;
	char* current_player_move;

} shared_server_resources;


DWORD ServiceThread(SOCKET* t_socket);

int read_write_common_resources_protected(int index_of_parameter_to_access, int read_or_write, int int_data_to_write, char* name_str_to_write, int* int_read,
	char** name_str_read, int increase_or_decrease_by_one);


#endif
