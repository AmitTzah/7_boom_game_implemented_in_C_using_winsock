//This module holds functions that wrap winsock2.h api functions for the application-level communication protocol.
//includes formating for communication messeges, which is project-specific.


#ifndef SOCKET_SEND_RECV_H
#define SOCKET_SEND_RECV_H

#include <winsock2.h>

#include "HardCodedData.h"

#pragma comment(lib, "ws2_32.lib")


typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;
 


int recv_and_extract_communication_message(SOCKET sd, char** communication_message, char* messeage_type, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS]);

TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd);



int format_communication_message(const char* messeage_type, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS], char** communication_message);

int get_size_of_communication_message(char* communication_message);

int compare_messages(char* array1, char* array2);
void free_communication_message_and_parameters(char* communication_message, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS], char* messeage_type);


#endif