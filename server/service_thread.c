//includes functions and structs related to operation of a worker thread(which is project specific). Each thread handles a row from the input file.


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "socket_send_recv.h"
#include "service_thread.h"
#include "file_IO.h"


DWORD ServiceThread(SOCKET* t_socket) {
	SOCKET accept_socket = *t_socket;
	char* communication_message = malloc(sizeof(char));
	
		if (recv_communication_message(accept_socket,&communication_message) == TRNS_FAILED)
	{

		printf("Error occuerd in server receving data, error num : % ld", WSAGetLastError());

	}
	else {
		printf("server recieved %d bytes\n", get_size_of_communication_message(communication_message));
		printf("Server recevied the string: %s\n",communication_message);


	}

		free(communication_message);
}
