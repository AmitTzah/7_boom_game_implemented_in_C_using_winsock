/*
Authors:

Amit Tzah 316062959
Tomer Shimshi 203200480


Project: Ex4
*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

#include "create_and_handle_processes.h"
#include "HardCodedData.h"
#include "file_IO.h"

#pragma comment(lib,"WS2_32")

void main(int argc, char* argv[]) {

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		exit(1);
	}


}