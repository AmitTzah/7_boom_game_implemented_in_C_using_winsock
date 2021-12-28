//includes functions related to file handling

#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <Windows.h>

#include "file_IO.h"
#include "HardCodedData.h"


//based on the example from :
/*
https://riptutorial.com/winapi/example/5736/create-a-file-and-write-to-it

*/

/// <summary>
/// 
/// <Function to read from a given file with given offset>
/// <param name="pathToFile"></path to file to be read >
/// <param name="buffer_for_bytes_read"></the string buffer to which the bytes are read. Should have space for  Null terminator>
/// <param name="num_of_bytes_to_read"></Number of bytes that will be read from offset>
/// <param name="read_from_offset"></offset from which to start reading>
/// <returns></returns the number of bytes that were read>
/// 


int WinReadFromFile(char* pathToFile, char* buffer_for_bytes_read, int num_of_bytes_to_read, int read_from_offset)
{


	OVERLAPPED offset_bytes;
	DWORD num_of_bytes_read;
	LPDWORD  num_of_bytes_read_pointer = &num_of_bytes_read;

	offset_bytes.Offset = read_from_offset;
	offset_bytes.OffsetHigh = 0;
	offset_bytes.hEvent = 0;



	// Open a handle to the file
	HANDLE hFile = CreateFileA(

		pathToFile,    //(LPCTSTR)(pathToFile)
		GENERIC_READ,        // Desired access
		NULL,
		NULL, // Security attributes
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		// Share mode
		NULL
	);


	if (hFile == INVALID_HANDLE_VALUE)
	{
		const int error = GetLastError();
		printf("Error opening file %s!. Error Code: %d\n", pathToFile, error);
		exit(1);
	}

	int res;
	res = ReadFile(hFile, buffer_for_bytes_read, num_of_bytes_to_read, num_of_bytes_read_pointer, &offset_bytes);

	buffer_for_bytes_read[num_of_bytes_to_read] = '\0'; //null terminate the array of bytes read, to get a string.

	if (FALSE == res)
	{
		const int error = GetLastError();
		printf("Error reading file!. Error Code: %d\n", error);
		exit(1);
	}
	CloseHandle(hFile);

	return num_of_bytes_read;
}


//write to a file using the win API lib
//based on the example from :
/*
https://riptutorial.com/winapi/example/5736/create-a-file-and-write-to-it

*/

/// <summary>
/// 
/// </Generic write to file from the given offset. StringToWrite is written to pathToFile>
/// <param name="pathToFile"></Path of the file to be written into.>
/// <param name="stringToWrite"></Null terminated>
/// <param name="StringLen"></Length of string to be written>
/// <param name="write_from_offset"></Offset from which to start overwriting>
/// <returns></returns the number of bytes that were written.>

int WinWriteToFile(char* pathToFile, char* stringToWrite, int StringLen, int write_from_offset)
{

	OVERLAPPED offset_bytes;

	offset_bytes.Offset = write_from_offset;
	offset_bytes.OffsetHigh = 0;
	offset_bytes.hEvent = 0;

	// Open a handle to the file
	HANDLE hFile = CreateFileA(
		pathToFile,   //(LPCTSTR)(pathToFile)
		GENERIC_WRITE,        // Desired access
		NULL,
		NULL, // Security attributes
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);


	if (hFile == INVALID_HANDLE_VALUE)
	{
		const int error = GetLastError();
		printf("WinWriteToFile Error opening file %s!. Error Code: %d\n", pathToFile, error);
		exit(1);
	}


	DWORD bytesWritten;
	int res;

	res = WriteFile(
		hFile,            // Handle to the file
		stringToWrite,  // Buffer to write
		StringLen,   // Buffer size
		&bytesWritten,    // Bytes written
		&offset_bytes);         // Overlapped

	if (FALSE == res)
	{
		const int error = GetLastError();
		printf("Error reading file!. Error Code: %d\n", error);
		exit(1);
	}

	// Close the handle once we don't need it.
	CloseHandle(hFile);

	return bytesWritten;
}





// Based on:
/*
https://www.geeksforgeeks.org/c-program-to-count-the-number-of-characters-in-a-file/
https://www.educative.io/edpresso/what-is-fopens-in-c
*/


/// <summary>
/// 
/// </returns number of characters in file>
/// <param name="pathToFile"></a path to file location>
/// <returns></number of characters in file>

int return_numbers_of_chars_in_file(char* pathToFile) {


	FILE* fp;

	// Declaring a variable of type errno_t to store the return value of fopen_s
	errno_t error_code;

	//Opening file in r mode
	error_code = fopen_s(&fp, pathToFile, "r");

	if (error_code != 0) {
		printf("Error! Failed to open file in r mode!\n");
		exit(1);
	}
	else {
		// Character counter (result)
		int count = 0;

		// To store a character read from file
		char c = 0;

		// Extract characters from file
		// and store in character c
		for (c = getc(fp); c != EOF; c = getc(fp))

			// Increment count for this character
			count = count + 1;

		// Close the file
		fclose(fp);

		return count;
	}

}

/*
 Returns the num of rows in a given text file.
based on https://stackoverflow.com/questions/12733105/c-function-that-counts-lines-in-file
*/

int num_of_rows_in_a_file(char* pathToFile)
{
	FILE* fp = fopen(pathToFile, "r");
	if (fp == NULL)
	{
		printf("Error opening file to get the number of rows!");
		exit(1);
	}
	int count = 0;
	int ch = 0;
	while (!feof(fp))
	{
		ch = fgetc(fp);
		if (ch == '\n')
		{
			count++;
		}
	}
	fclose(fp);
	return count;
}



/// <summary>
/// 
/// </given a path to file where each row is made of integer numbers seperated by spaces, this function parsed the int values of a wanted row into the given array >
/// <param name="pathToFile"></path to file>
/// <param name="index_of_wanted_line"></index of the row to parse>
/// <param name="arr_to_hold_parsed_values"></pointer to the array that will hold the parsed variables>
/// <param name="arr_size"></size of that array, or the number of parsed values in a row.>
void parse_row_to_array_of_ints(char* pathToFile, int index_of_wanted_line, int* arr_to_hold_parsed_values, int arr_size)
{
	int file_offset = 0;
	char* line_buffer = (char*)malloc(sizeof(char) * (MAX_LENGTH_OF_ROW + 1));
	int row_counter = 0;
	file_offset = 0;

	if (line_buffer == NULL)
	{
		printf("error alocating memory in row_to_int ");
		exit(1);
	}
	while (TRUE)
	{

		file_offset += WinReadFromFile(pathToFile, line_buffer, MAX_LENGTH_OF_ROW, file_offset);
		char* temp_char_1 = strtok(line_buffer, "\r\n");
		for (int j = 1; j <= index_of_wanted_line; j++)
		{
			temp_char_1 = strtok(NULL, "\r\n");
		}
		// we are at wanted row
		{
			char* temp_char = strtok(temp_char_1, " ");
			*arr_to_hold_parsed_values = atoi(temp_char);
			arr_to_hold_parsed_values++;

			for (int i = 1; i < arr_size; i++) {
				temp_char = strtok(NULL, " ");
				*arr_to_hold_parsed_values = atoi(temp_char);
				arr_to_hold_parsed_values++;
			}
			break;
		}
	}

	free(line_buffer);
}



/// <summary>
/// 
/// </Returns number of digits in an integer number>
/// <param name="num"></num>
/// <returns></number of digit>
int get_num_of_digits_in_an_int_number(int num)
{
	if (num == 0)
		return 1;
	int count = 0;
	while (num != 0)
	{
		count++;
		num = num / 10;
	}
	return count;
}





/*
* the following func recives file location, file offset and the aproprate vars
* writes them in order to the output file
* the func returns the new offset needed to write an additional line
*/

int write_to_output(char* pathToFile, int virtual_page_num, int physycal_page_num, int time, BOOL need_to_empty, int write_from_offset)
{
	char* Line_To_Write = (char*)malloc(sizeof(char) * MAX_LENGTH_OF_ROW);
	char temp_str[MAX_LENGTH_OF_ROW];
	if (Line_To_Write == NULL)
	{
		printf("error alocating memory for the output file write string");
		exit(1);

	}
	int num_of_bits_to_write = 0;
	//count the number of chars in each int var
	num_of_bits_to_write += get_num_of_digits_in_an_int_number(virtual_page_num);
	num_of_bits_to_write += get_num_of_digits_in_an_int_number(physycal_page_num);
	num_of_bits_to_write += get_num_of_digits_in_an_int_number(time);
	// add an extra one for the E/P
	num_of_bits_to_write += 1;
	sprintf(Line_To_Write, "%d", time);
	strcat(Line_To_Write, " ");

	sprintf(temp_str, "%d", virtual_page_num);



	strcat(Line_To_Write, temp_str);
	strcat(Line_To_Write, " ");
	sprintf(temp_str, "%d", physycal_page_num);
	strcat(Line_To_Write, temp_str);
	strcat(Line_To_Write, " ");
	if (need_to_empty == TRUE)
	{
		strcat(Line_To_Write, "E");
	}
	else
	{
		strcat(Line_To_Write, "P");
	}

	num_of_bits_to_write += 3;// added the spaces and char to the sum
	printf("wrote to output: %s\n", Line_To_Write);
	WinWriteToFile(pathToFile, Line_To_Write, num_of_bits_to_write, write_from_offset);
	write_from_offset += num_of_bits_to_write;
	// start new line

	WinWriteToFile(pathToFile, "\r\n", 2, write_from_offset);
	write_from_offset += 2;

	free(Line_To_Write);
	return write_from_offset;
}


/*
Given a pointer to a path_to_input in a form of string, this function changes path_to_outout to Output.txt in the same location.

*/
char* find_output_path(char* path_to_input_file)
{
	int i = 0;
	char* path_to_output = (char*)malloc(sizeof(char) * (MAX_LENGTH_OF_PATH_TO_A_FILE + 1));


	if (path_to_output == NULL)
	{
		printf("error alocating memory in row_to_int\n");
		exit(1);
	}
	strcpy(path_to_output, path_to_input_file);

	while (path_to_output[i] != '\0') {

		i++;

	}
	i--;
	while ((path_to_output[i] != '\\') && (path_to_output[i] != '/')) {

		i--;
	}

	i++;

	path_to_output[i] = '\0';

	strcat(path_to_output, "Output.txt");

	return path_to_output;

}




