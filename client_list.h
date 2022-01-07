// General libraries
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "visualization.h"


/* List of commands */
typedef enum command_t {CONNECT, BOARD_UPDATE, PADDLE_MOVE, DISCONNECT} command_t;

/* Message definition */
typedef struct message
{   
    command_t command;               // commands
    ball_position_t ball_position;   // ball position
    int number_clients;             // paddle positions
    char pressed_key;                                 
}message;


typedef struct paddle_position_message
{
    paddle_position_t paddle_position;
    //bool current_player;
    int player_score; 
}paddle_position_message;


// A linked list node, stored on server
struct Node {
    char address[100];
    int port;
    paddle_position_t paddle;
    //bool current_player;
    int player_score;
    struct Node* next;
};

// A linked list node for player informations, stored on client
struct Paddle_Node {
    paddle_position_t paddle;
    //bool current_player;
    int player_score;
    struct Paddle_Node* next;
};    

// Next player
int next_player_port;
char next_player_address[100];
bool is_next_player;

// Functions

void add_client(struct Node** head_ref, char new_address[], int new_port, paddle_position_t new_paddle);
void delete_client(struct Node** head_ref, char delete_address[], int delete_port);
void print_list(struct Node* node);
bool next_player(struct Node** head_ref, char player_address[], int player_port);

//NEW TO SUPERPONG
void add_player_list(struct Paddle_Node** head_ref, paddle_position_t new_paddle, int player_score);
void update_paddle(struct Node** client_list, char remote_addr_str[], int remote_port, ball_position_t ball, int key);

// GRAPHICS
//void draw_all_paddles(WINDOW *win, struct Node* client_list, int del);
