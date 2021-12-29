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
	char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS];
	TransferResult_t send_recv_result;

	communication_message = malloc(sizeof(char));
	if (communication_message == NULL) { printf("malloc failed in main() server");  }
	//first get the CLIENT_REQUEST
	if (recv_communication_message(accept_socket, &communication_message) == TRNS_FAILED)
	{
		printf("Error occuerd in server receving data, error num : % ld", WSAGetLastError());
	}
	printf("server recevied message from client: %s\n", communication_message);
	//get_parameter_array_from_messsage()

	free(communication_message);
	//send back SERVER_APPROVED
	communication_message = format_communication_message("SERVER_APPROVED", parameters_array);
	send_recv_result = SendBuffer(communication_message, get_size_of_communication_message(communication_message), accept_socket);
	if (send_recv_result == TRNS_FAILED) {
		printf("Failed to send messeage from client!\n");
	
	}
	free(communication_message);

	while(1){}
}
