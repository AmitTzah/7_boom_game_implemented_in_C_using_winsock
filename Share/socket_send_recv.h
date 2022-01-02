//This module holds functions that wrap winsock2.h api functions for the application-level communication protocol.
//includes formating for communication messeges, which is project-specific.


#ifndef SOCKET_SEND_RECV_H
#define SOCKET_SEND_RECV_H

#include <winsock2.h>

#include "HardCodedData.h"

#pragma comment(lib, "ws2_32.lib")


typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;
 


int recv_and_extract_communication_message(SOCKET sd, char** communication_message, char* messeage_type, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS]);
int send_message(SOCKET sd, const char* messeage_type, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS]);

void free_communication_message_and_parameters(char* communication_message, char* parameters_array[MAX_NUM_OF_MESSAGE_PARAMETERS], char* messeage_type);



#endif