//This module holds functions that wrap winsock2.h api functions for the application-level communication protocol.
//includes formating for communication messeges, which is project-specific.

#include "socket_send_recv.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Receive bytes into a buffer of RECV_IN_CHUNKS size. After each recv, it checks if a newline character is reached.
//This way we are guaranteed to receive the excat meseage sent, into a dynamically allcated char array.
//*communication_message is a pointer to a heap allocated char array, whhich will be realloc as more of the message is read.
//returns TRNS_FAILED in case recv failed. Should be checked in caller!
//communication_message should be freed in caller.
TransferResult_t recv_communication_message(SOCKET sd, char** communication_message) {
	char receive_buffer[RECV_IN_CHUNKS];
	size_t size_of_communication_message = 0;
	do {
		size_t  bytes_recv = recv(sd, receive_buffer, RECV_IN_CHUNKS, 0);

		if (bytes_recv == -1) {

			printf("Error occuerd in receving data, error num : % ld", WSAGetLastError());
			return TRNS_FAILED;
		}


		size_t old_size_of_communication_message = size_of_communication_message;
		size_of_communication_message += bytes_recv;
		
		*communication_message = (char*)realloc(*communication_message, size_of_communication_message);

		if (*communication_message == NULL) {
			printf("Memory allocation failed in recv_communication_message(), exiting...");
			exit(1);

		}

		char* pointer_to_start_of_current_message = *communication_message + old_size_of_communication_message;

		memcpy((pointer_to_start_of_current_message), receive_buffer, bytes_recv);

	

	} while ((*communication_message)[size_of_communication_message-1] != '\n');
}








/**
 * SendBuffer() uses a socket to send a buffer.
 *
 * Accepts:
 * -------
 * Buffer - the buffer containing the data to be sent.
 * BytesToSend - the number of bytes from the Buffer to send.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if sending succeeded
 * TRNS_FAILED - otherwise
 */
TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd)
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;

	while (RemainingBytesToSend > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send(sd, CurPlacePtr, RemainingBytesToSend, 0);
		if (BytesTransferred == SOCKET_ERROR)
		{
			printf("send() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}

		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}





//returns a pointer to a heap-allocated string of the formated message, according to messeage type and parameters array.
//the string should be freed in caller 
char* format_communication_message(const char* messeage_type, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS]) {
	char* communication_message = NULL;
	int size_of_communication_message = 0;
	//check if message has three parameters.
	if (strcmp(messeage_type, "GAME_VIEW") == 0) {
		size_of_communication_message += 5; //semicolons, colons,newline char , nullterminator for easier use in printf.
		size_of_communication_message += strlen("GAME_VIEW");
		for (int j = 0; j < MAX_NUM_OF_MESSAGE_PARAMETERS; j++) {
			size_of_communication_message += strlen(parameters_array[j]);
		}

		communication_message = (char*)calloc(size_of_communication_message, sizeof(char));
		if (communication_message == NULL) {
			printf("Memory allocation failed in format_communication_message(), exiting...");
			exit(1);

		}

		else {

			strcpy_s(communication_message, size_of_communication_message, "GAME_VIEW");
			strcat_s(communication_message, size_of_communication_message, ":");
			strcat_s(communication_message, size_of_communication_message, parameters_array[0]);
			strcat_s(communication_message, size_of_communication_message, ";");
			strcat_s(communication_message, size_of_communication_message, parameters_array[1]);
			strcat_s(communication_message, size_of_communication_message, ";");
			strcat_s(communication_message, size_of_communication_message, parameters_array[2]);
			strcat_s(communication_message, size_of_communication_message, "\n");

		}
	}


	//check if message has only one parameter
	else if (strcmp(messeage_type, "CLIENT_PLAYER_MOVE") == 0 || strcmp(messeage_type, "CLIENT_REQUEST") == 0 ||
		strcmp(messeage_type, "TURN_SWITCH") == 0 || strcmp(messeage_type, "GAME_ENDED") == 0) {

		size_of_communication_message += 3;//colons,newline char , nullterminator for easier use in printf.
		size_of_communication_message += strlen(parameters_array[0]);
		size_of_communication_message += strlen(messeage_type);

		communication_message = (char*)calloc(size_of_communication_message, sizeof(char));
		if (communication_message == NULL) {
			printf("Memory allocation failed in format_communication_message(), exiting...");
			exit(1);

		}

		else {
			strcpy_s(communication_message, size_of_communication_message, messeage_type);
			strcat_s(communication_message, size_of_communication_message, ":");
			strcat_s(communication_message, size_of_communication_message, parameters_array[0]);
			strcat_s(communication_message, size_of_communication_message, "\n");
		}
	}

	//message has no parameters.
	else {
		size_of_communication_message += strlen(messeage_type) + 2;
		communication_message = (char*)calloc(size_of_communication_message, sizeof(char));
		if (communication_message == NULL) {
			printf("Memory allocation failed in format_communication_message(), exiting...");
			exit(1);

		}

		else {
			strcpy_s(communication_message, size_of_communication_message, messeage_type);
			strcat_s(communication_message, size_of_communication_message, "\n");
		}

	}


	return communication_message;
}

int get_size_of_communication_message(char* communication_message) {
	int j = 0;
	while (communication_message[j] != '\n') {

		j++;

	}

	return (j + 1);
}