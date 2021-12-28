//includes functions and structs related to operation of a worker thread(which is project specific). Each thread handles a row from the input file.

#ifndef SERVICE_THREAD_H
#define SERVICE_THREAD_H

#include <stddef.h>
#include <stdbool.h>
#include <Windows.h>


#include "HardCodedData.h"
#include "create_and_handle_processes.h"

DWORD ServiceThread(SOCKET* t_socket);


#endif
