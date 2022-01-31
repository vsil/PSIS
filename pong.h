#ifndef PONG_H
#define PONG_H

/* List of commands */
typedef enum command_t {CONNECT, BOARD_UPDATE, PADDLE_MOVE, DISCONNECT, WAIT_LIST} command_t;

/* General Message definition */
typedef struct message
{   
    command_t command;
    ball_position_t ball_position;
    int number_clients;
    int pressed_key;                                 
}message;

/* Message used when board_update message is exchanged */
typedef struct paddle_position_message
{
    paddle_position_t paddle_position;
    int player_score;
    bool current_player; 
}paddle_position_message;

#endif