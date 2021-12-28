//includes functions related to the creation and handling of processes/threads.

#ifndef CREATE_AND_HANDLE_PROCESSES_H
#define CREATE_AND_HANDLE_PROCESSES_H

#include <windows.h>


typedef struct ReadersWritersParam {
	int readers;
	HANDLE mutex;
	HANDLE room_empty_semaphore;
	HANDLE turn_slide_mutex;


} ReadersWritersParam;

DWORD CreateProcessSimpleMain(LPSTR command_line_arguments_to_run, int time_out_in_ms);

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id);
void close_array_of_handles(HANDLE* array_of_handles, int size_of_array);

char* concatenate_command_line_arguments_into_a_string(int num_of_arguments, char* arguments_array[]);

HANDLE* create_and_init_array_semaphore_objects(unsigned int overall_num_of_semaphore_objects, unsigned int initial_semaphore_count, unsigned int maximum_count);

ReadersWritersParam create_and_init_readers_writers_param_struct(int max_num_of_readers);

#endif