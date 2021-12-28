//This module holds functions that wrap winsock2.h api functions for the application-level communication protocol.


#ifndef SOCKET_SEND_RECV_H
#define SOCKET_SEND_RECV_H


#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")


typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;


TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd);


TransferResult_t SendString(const char* Str, SOCKET sd);


TransferResult_t ReceiveBuffer(char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd);


TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd);



#endif