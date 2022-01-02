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

} shared_server_resources;


DWORD ServiceThread(SOCKET* t_socket);



#endif
