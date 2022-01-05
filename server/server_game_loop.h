#ifndef SERVER_GAME_LOOP_H
#define SERVER_GAME_LOOP_H

#include "socket_send_recv.h"


int server_game_loop(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME], int* write_from_offset_to_log_file, char thread_log_file_name[MAX_LENGTH_OF_THREAD_LOG_FILE_NAME]);





#endif
