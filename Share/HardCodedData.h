//This header file holds different macros and structures used in the project.

#ifndef HARD_CODED_DATA_H
#define HARD_CODED_DATA_H

//general constants
#define TIMEOUT_IN_MILLISECONDS 5000
#define BRUTAL_TERMINATION_CODE 0x55
#define ERROR_CODE ((int)(-1))
#define SUCCESS_CODE ((int)(0))
#define MAX_LENGTH_OF_PATH_TO_A_FILE 300
#define MAX_LENGTH_OF_ROW 200



//project specific constants


#define THREAD_TIMEOUT_IN_MS 80
#define SERVER_ADDRESS_STR "127.0.0.1"
#define NUM_OF_WORKER_THREADS 2
#define MAX_LENGH_OF_INPUT_FROM_USER 200
#define MAX_NUM_OF_MESSAGE_PARAMETERS 3
#define RECV_IN_CHUNKS 1 //has to be 1, since two messages can be sent one after the other, and in that case we have to assure we don't skip the first '\n'
#define MAX_LENGH_OF_CLIENT_NAME 20
#define MAX_LENGH_OF_MESSAGE_TYPE 50

//messages types
#define CLIENT_REQUEST "CLIENT_REQUEST"
#define CLIENT_VERSUS "CLIENT_VERSUS"
#define CLIENT_PLAYER_MOVE "CLIENT_PLAYER_MOVE"
#define CLIENT_DISCONNECT "CLIENT_DISCONNECT"
#define SERVER_APPROVED "SERVER_APPROVED"
#define SERVER_DENIED "SERVER_DENIED"
#define SERVER_MAIN_MENU "SERVER_MAIN_MENU"
#define GAME_STARTED "GAME_STARTED"
#define TURN_SWITCH "TURN_SWITCH"
#define SERVER_MOVE_REQUEST "SERVER_MOVE_REQUEST"
#define GAME_ENDED "GAME_ENDED" 
#define SERVER_NO_OPPONENTS "SERVER_NO_OPPONENTS"
#define GAME_VIEW "GAME_VIEW"
#define SERVER_OPPONENT_QUIT "SERVER_OPPONENT_QUIT"


#endif