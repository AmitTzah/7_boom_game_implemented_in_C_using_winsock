#ifndef SERVER_GAME_LOOP_H
#define SERVER_GAME_LOOP_H

#include "socket_send_recv.h"


int server_game_loop(SOCKET accept_socket, int* num_of_player, char client_name[MAX_LENGH_OF_CLIENT_NAME]);





#endif
