//This module holds functions that wrap winsock2.h api functions for the application-level communication protocol.
//includes formating for communication messeges, which is project-specific.

#include "socket_send_recv.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/**
 * SendString() uses a socket to send a string.
 * Str - the string to send.
 * sd - the socket used for communication.
 */
TransferResult_t SendString(const char* Str, SOCKET sd)
{
	/* Send the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	TotalStringSizeInBytes = (int)(strlen(Str) + 1); // terminating zero also sent	

	SendRes = SendBuffer(
		(const char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // sizeof(int) 
		sd);

	if (SendRes != TRNS_SUCCEEDED) return SendRes;

	SendRes = SendBuffer(
		(const char*)(Str),
		(int)(TotalStringSizeInBytes),
		sd);

	return SendRes;
}

/**
 * Accepts:
 * -------
 * ReceiveBuffer() uses a socket to receive a buffer.
 * OutputBuffer - pointer to a buffer into which data will be written
 * OutputBufferSize - size in bytes of Output Buffer
 * BytesReceivedPtr - output parameter. if function returns TRNS_SUCCEEDED, then this
 *					  will point at an int containing the number of bytes received.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */
TransferResult_t ReceiveBuffer(char* OutputBuffer, int BytesToReceive, SOCKET sd)
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;

	while (RemainingBytesToReceive > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if (BytesJustTransferred == SOCKET_ERROR)
		{
			printf("recv() failed, error %d\n", WSAGetLastError());
			return TRNS_FAILED;
		}
		else if (BytesJustTransferred == 0)
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

/**
 * ReceiveString() uses a socket to receive a string, and stores it in dynamic memory.
 *
 * Accepts:
 * -------
 * OutputStrPtr - a pointer to a char-pointer that is initialized to NULL, as in:
 *
 *		char *Buffer = NULL;
 *		ReceiveString( &Buffer, ___ );
 *
 * a dynamically allocated string will be created, and (*OutputStrPtr) will point to it.
 *
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving and memory allocation succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */
TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd)
{
	/* Recv the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;

	if ((OutputStrPtr == NULL) || (*OutputStrPtr != NULL))
	{
		printf("The first input to ReceiveString() must be "
			"a pointer to a char pointer that is initialized to NULL. For example:\n"
			"\tchar* Buffer = NULL;\n"
			"\tReceiveString( &Buffer, ___ )\n");
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	RecvRes = ReceiveBuffer(
		(char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // 4 bytes
		sd);

	if (RecvRes != TRNS_SUCCEEDED) return RecvRes;

	StrBuffer = (char*)malloc(TotalStringSizeInBytes * sizeof(char));

	if (StrBuffer == NULL)
		return TRNS_FAILED;

	RecvRes = ReceiveBuffer(
		(char*)(StrBuffer),
		(int)(TotalStringSizeInBytes),
		sd);

	if (RecvRes == TRNS_SUCCEEDED)
	{
		*OutputStrPtr = StrBuffer;
	}
	else
	{
		free(StrBuffer);
	}

	return RecvRes;
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