//includes functions and structs related to operation of a worker thread(which is project specific). Each thread handles a row from the input file.


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "service_thread.h"
#include "file_IO.h"


DWORD ServiceThread(SOCKET* t_socket) {
	SOCKET accept_socket = *t_socket;
	char buffer[21];
	int bytes_recv = recv(*t_socket, buffer, 21, 0);
	if (bytes_recv == -1) {

		printf("Error occuerd in server receving data, error num : % ld", WSAGetLastError());
	}
	else {
		printf("server recieved %d bytes\n", bytes_recv);
		printf("Server recevied the string: %s\n", buffer);
	}
}
