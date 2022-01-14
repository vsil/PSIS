#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

// General libraries
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

// Header files
#include "visualization.h"

#define MAX_CLIENTS 2

// A linked list node, stored on server
struct Node {
    char address[100];
    int port;
    paddle_position_t paddle;
    paddle_position_t previous_paddle;
    int player_score;
    struct Node* next;
};

// A linked list node for player informations (paddle position and score), stored on client
struct Paddle_Node {
    paddle_position_t paddle;
    bool current_player;
    int player_score;
    struct Paddle_Node* next;
};    

// Address structure 
typedef struct address{
    char addr[100];
    int port;
}address;

// Functions
void add_client(struct Node** head_ref, char new_address[], int new_port, paddle_position_t new_paddle);
void delete_client(struct Node** head_ref, char delete_address[], int delete_port);
void print_list(struct Node* node);
void add_player_list(struct Paddle_Node** head_ref, paddle_position_t new_paddle, int player_score, bool current_player);
void update_paddle(struct Node** client_list, char remote_addr_str[], int remote_port, ball_position_t ball, int key);
void reset_list(struct Paddle_Node** head_ref);
void draw_all_paddles(WINDOW *win, struct Paddle_Node* paddle_list, bool del);
address add_client_from_waiting_list(struct Node** head_client_list, struct Node** head_waiting_list, ball_position_t* ball, address player_waiting_addr);
bool paddle_hit_paddle(paddle_position_t new_paddle_position, ball_position_t* ball,struct Node* client_list, char player_address[], int player_port);
void paddle_hit_ball(ball_position_t * ball, struct Node ** client_list, ball_position_t * previous_ball);

#endif