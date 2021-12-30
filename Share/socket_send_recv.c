//This module holds functions that wrap winsock2.h api functions for the application-level communication protocol.
//includes formating for communication messeges, which is project-specific.

#include "socket_send_recv.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Receive bytes into a buffer of RECV_IN_CHUNKS size. After each recv, it checks if a newline character is reached.
//This way we are guaranteed to receive the excat meseage sent, into a dynamically allcated char array.
//*communication_message is a pointer to  char array.
//returns TRNS_FAILED in case recv failed. Should be checked in caller!
//communication_message should be freed in caller.
TransferResult_t recv_communication_message(SOCKET sd, char** communication_message) {
	char receive_buffer[RECV_IN_CHUNKS];
	*communication_message = malloc(sizeof(char));
	if (*communication_message == NULL) {

		printf("malloc failed in recv_communication_message()\n"); 

		return TRNS_FAILED;
	}

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
			return TRNS_FAILED;

		}

		char* pointer_to_start_of_current_message = *communication_message + old_size_of_communication_message;

		memcpy((pointer_to_start_of_current_message), receive_buffer, bytes_recv);

	

	} while ((*communication_message)[size_of_communication_message-1] != '\n');

	return TRNS_SUCCEEDED;
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


//based on https://stackoverflow.com/questions/27495346/compraring-char-array-without-using-strcmp-in-c/27495579
//similar to strcom. This function compares two char arrays, that are messeage-formated (ending in newline instead of null terminator)
//return 1 if equal. else 0
int compare_messages(char* array1, char* array2)
{
	if ((array1 == NULL) || (array2 == NULL)) {
		printf("Passed a null array to comapre messages!\n");
		return 0;
	}
		
	while ((*array1 != '\n') && (*array2 != '\n') && (*array1 == *array2))
	{
		array1++;
		array2++;
	}
	return (*array1 == *array2);
}

//reverse of format_communication_message()
//given a communication_message, it  extracts the parameters and message type into the appropriate  arguments IN THE FORM OF STRINGS!.
//messeage_type should be stack-allocated in caller, using the max lengh of a message type (which is known pre compilation).
//parameters array elements will be allocated on heap, should be freed in caller!
void extract_parameters_from_communication_message(char* communication_message, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS], char* messeage_type) {
	int i = 0;
	int size_of_parameter = 0;
	int parameter_index = 0;
	while (communication_message[i] != ':' && communication_message[i] != '\n') {
		i++;
	}

	int j = 0;
	//copy meesage to message_type
	while (j != i) {

		messeage_type[j] = communication_message[j];
		j++;
	}
	messeage_type[j] = '\0';

	
	if (communication_message[i] == ':') {
	//message has parameters!
	//extract parameters into parameters_array. Remember to free parameters_array in caller.

		while (communication_message[i] != '\n') {

			if (communication_message[i] == ';' || communication_message[i] == ':') {
				size_of_parameter = 0;
				if (communication_message[i] == ';') {
					parameters_array[parameter_index][j] = '\0'; 
					parameter_index++;
				};
				

				j = i + 1;
				//get size_of_parameter
				while (communication_message[j] != ';' && communication_message[j]!='\n') {
					j++;
					size_of_parameter++;

				}
			}
			i=i+1;
			j = 0;
			parameters_array[parameter_index] = malloc((1+size_of_parameter) * sizeof(char)); //+1 for null terminator
			if (parameters_array[parameter_index] == NULL) {

				printf("Memory allocation failed in extract_parameters_from_communication_message()\n");
				exit(1);
			}
			while (communication_message[i] != ';' && communication_message[i] != '\n') {
				parameters_array[parameter_index][j] = communication_message[i];

				i++;
				j++;
			}



		}


		parameters_array[parameter_index][j] = '\0';
	}
}

//this function should be called after the caller finished working with the arguments of extract_parameters_from_communication_message()
void free_communication_message_type_and_parameters(char* communication_message, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS], char* messeage_type) {
	int num_of_parameters = 0;
	if (strcmp(messeage_type, "GAME_VIEW") == 0) {

		num_of_parameters = 3;

	}

	if (strcmp(messeage_type, "CLIENT_PLAYER_MOVE") == 0 || strcmp(messeage_type, "CLIENT_REQUEST") == 0 ||
		strcmp(messeage_type, "TURN_SWITCH") == 0 || strcmp(messeage_type, "GAME_ENDED") == 0) {

		num_of_parameters = 1;
	}

	

	for (int i = 0; i < num_of_parameters; i++) {

		free(parameters_array[i]);
	}
	free(communication_message);
	


}