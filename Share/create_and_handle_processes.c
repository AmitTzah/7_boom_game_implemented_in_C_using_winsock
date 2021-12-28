//includes functions related to the creation and handling of processes and threads.


#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "HardCodedData.h"
#include "create_and_handle_processes.h"


//private function, based on example from moodle.

/// <summary>
/// 
/// </Creates a process with the given Command line>
/// <param name="CommandLine"></The command line argument, string format>
/// <param name="ProcessInfoPtr"></A pointer to struct that will hold information about the process.>
/// <returns></returns>
BOOL CreateProcessSimple(LPSTR CommandLine, PROCESS_INFORMATION* ProcessInfoPtr)
{
	STARTUPINFO	startinfo = { sizeof(STARTUPINFO), NULL, 0 }; /* <ISP> here we */
															  /* initialize a "Neutral" STARTUPINFO variable. Supplying this to */
															  /* CreateProcess() means we have no special interest in this parameter. */
															  /* This is equivalent to what we are doing by supplying NULL to most other */
															  /* parameters of CreateProcess(). */

	return CreateProcessA(
		NULL, /*  No module name (use command line). */
		CommandLine,			/*  Command line. */
		NULL,					/*  Process handle not inheritable. */
		NULL,					/*  Thread handle not inheritable. */
		FALSE,					/*  Set handle inheritance to FALSE. */
		NORMAL_PRIORITY_CLASS,	/*  creation/priority flags. */
		NULL,					/*  Use parent's environment block. */
		NULL,					/*  Use parent's starting directory. */
		&startinfo,				/*  Pointer to STARTUPINFO structure. */
		ProcessInfoPtr			/*  Pointer to PROCESS_INFORMATION structure. */
	);
}


//public function, based on example from moodle.

/// <summary>
/// 
/// </Create a process, giving it time_out_in_ms Miliseconds to finish>
/// <param name="command_line_arguments_to_run"></a string that holds the command line of the proccess to run>
/// <param name="time_out_in_ms"></time to wait untill the process finishes>
/// <returns></returns exitcode of the process. If everything went OK should be 0.>

DWORD CreateProcessSimpleMain(LPSTR command_line_arguments_to_run, int time_out_in_ms)
{
	PROCESS_INFORMATION procinfo;
	DWORD				waitcode;
	DWORD				exitcode;
	BOOL				retVal;




	/*  Start the child process. */
	retVal = CreateProcessSimple(command_line_arguments_to_run, &procinfo);


	if (retVal == 0)
	{
		const int error = GetLastError();
		printf("Process Creation Failed!\n");

		printf("Error code %d", error);
		exit(1);

	}


	waitcode = WaitForSingleObject(
		procinfo.hProcess,
		time_out_in_ms); /* Waiting 5 secs for the process to end */

	printf("WaitForSingleObject output: ");
	switch (waitcode)
	{
	case WAIT_TIMEOUT:
		printf("WAIT_TIMEOUT\n"); break;
	case WAIT_OBJECT_0:
		printf("WAIT_OBJECT_0\n"); break;
	default:
		printf("0x%x\n", waitcode);
	}

	if (waitcode == WAIT_TIMEOUT) /* Process is still alive */
	{
		printf("Process was not terminated before timeout!\n"
			"Terminating brutally!\n");
		TerminateProcess(
			procinfo.hProcess,
			BRUTAL_TERMINATION_CODE); /* Terminating process with an exit code of 55h */
		Sleep(10); /* Waiting a few milliseconds for the process to terminate,
					note the above command may also fail, so another WaitForSingleObject is required.
					We skip this for brevity */
	}

	GetExitCodeProcess(procinfo.hProcess, &exitcode);

	printf("The exit code for the process is 0x%x\n", exitcode);

	/* Note: process is still being tracked by OS until we release handles */
	CloseHandle(procinfo.hProcess); /* Closing the handle to the process */
	CloseHandle(procinfo.hThread); /* Closing the handle to the main thread of the process */

	return exitcode;
}





//based on https://stackoverflow.com/questions/7174216/how-can-i-concatenate-arguments-in-to-a-string-in-c

/// <summary>
/// 
/// </concatenate command line arguments into a single string that can be passed to Son process, including the name of the Son.exe>
/// <param name="num_of_arguments"></Number of arguments including the name of Son.exe>
/// <param name="arguments_array"></Am array of const strings argv[0], argv[1] etc>
/// <returns></Returns a dynamically allocated command line strings. This string should be freed in caller!>

char* concatenate_command_line_arguments_into_a_string(int num_of_arguments, char* arguments_array[])

{
	int i;
	int needed = 1;                        // plus end of string mark

	for (i = 0; i < num_of_arguments; ++i) {
		needed += strlen(arguments_array[i]) + 1; // plus space
	}

	char* command_line_string = (char*)malloc(sizeof(char) * needed);

	// if memory cannot be allocated
	if (command_line_string == NULL) {
		printf("Error! memory not allocated to command_line_string.\n");
		exit(1);
	}
	else {
		memset(command_line_string, 0, needed);


		strcpy(command_line_string, arguments_array[0]);

		for (i = 1; i < num_of_arguments; ++i) {
			strcat(command_line_string, " ");
			strcat(command_line_string, arguments_array[i]);
		}
	}

	return command_line_string;

}


//public function, based on example from moodle.

/// <summary>
/// 
/// </A function to create a thread, handle should be closed in caller with CloseHandle() function..>
/// <param name="p_start_routine"></The routine of the thread, a function defined as: DWORD WINAPI p_start_routine(LPVOID lpParam){...}>
/// <param name="p_thread_parameters"></a pointer to parameter to be passed to start_routine. Usually a struct.>
/// <param name="p_thread_id"></A pointer to DWORD thread_id, this is an out parameter.>
/// <returns></returns>
//If fails, returns NULL. This should be check and handled in caller!
HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id) {


	HANDLE thread_handle;

	if (NULL == p_start_routine)
	{
		printf("Error when creating a thread\n");
		printf("Received null pointer\n");
		return NULL;
	}

	if (NULL == p_thread_id)
	{
		printf("Error when creating a thread\n");
		printf("Received null pointer\n");
		return NULL;
	}

	thread_handle = CreateThread(
		NULL,                /*  default security attributes */
		0,                   /*  use default stack size */
		p_start_routine,     /*  thread function */
		p_thread_parameters, /*  argument to thread function */
		0,                   /*  use default creation flags */
		p_thread_id);        /*  returns the thread identifier */


	if (thread_handle == 0)
	{
		const int error = GetLastError();
		printf("Thread Creation Failed!\n");

		printf("Error code %d\n", error);
		return thread_handle;

	}

	return thread_handle;
}

//this functions gets an array of handles and closes all handles.
void close_array_of_handles(HANDLE* array_of_handles, int size_of_array) {

	int ret_val;

	for (int j = 0; j < size_of_array; j++) {

		ret_val = CloseHandle(array_of_handles[j]);
		if (FALSE == ret_val)
		{
			const int error = GetLastError();
			printf("Error when closing a handle, error code: %d\n", error);
			exit(1);
		}

	}


}

//if failes, returns NULL, this should be checked and handled in caller!
HANDLE* create_and_init_array_semaphore_objects(unsigned int overall_num_of_semaphore_objects, unsigned int initial_semaphore_count, unsigned int maximum_count) {
	HANDLE* semaphore_array;
	HANDLE semaphore_object;
	semaphore_array = calloc(overall_num_of_semaphore_objects, sizeof(HANDLE));


	if (semaphore_array == NULL)
	{
		printf("Memory allocation of semphore_array failed!\n");
		free(semaphore_array);
		return NULL;
	}


	for (unsigned int i = 0; i < overall_num_of_semaphore_objects; i++) {

		semaphore_object = CreateSemaphore(
			NULL,	/* Default security attributes */
			initial_semaphore_count,		/* Initial Count - all slots are empty */
			maximum_count,		/* Maximum Count */
			NULL);  /* un-named */

		if (semaphore_object == NULL)
		{
			printf("Failed to create semaphore!\n");
			free(semaphore_array);
			return NULL;
		}

		else {

			semaphore_array[i] = semaphore_object;
		}

	}

	return  semaphore_array;
}


ReadersWritersParam create_and_init_readers_writers_param_struct(int max_num_of_readers) {

	ReadersWritersParam params;

	params.readers = 0;
	params.mutex = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	params.room_empty_semaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		1,		/* Initial Count - all slots are empty */
		max_num_of_readers,		/* Maximum Count */
		NULL);  /* un-named */


	params.turn_slide_mutex = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */


	if (params.room_empty_semaphore == NULL || params.turn_slide_mutex == NULL || params.mutex == NULL)
	{
		printf("failed to init readers_writers_param struct, exiting!\n");

		exit(1);
	}

	return params;

}

