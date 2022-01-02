#include "visualization.h"

/* List of commands */
typedef enum command_t {CONNECT, RELEASE, SEND, MOVE, DISCONNECT} command_t;

/* Message definition */
typedef struct message
{   
    command_t command;               // commands
    ball_position_t ball_position;   // ball position (to be sent when command = MOVE)
}message;


