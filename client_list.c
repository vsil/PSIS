#include "client_list.h"

// adds new client to the list (used on server)
void add_client(struct Node** head_ref, char new_address[], int new_port, paddle_position_t new_paddle, int new_client_socket)
{
    /* 1. allocate node */
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
    struct Node *last = *head_ref;  /* used in step 5*/

    /* 2. put in the data  */
	strcpy(new_node->address, new_address);
	new_node->port = new_port;
    new_node->paddle = new_paddle;
    new_node->player_score = 0;
    new_node->client_socket = new_client_socket;

    /* 3. This new node is going to be the last node, so make next of it as NULL */
    new_node->next = NULL;

    /* 4. If the Linked List is empty, then make the new node as head */
    if (*head_ref == NULL)
    {
       *head_ref = new_node;
       return;
    } 

    /* 5. Else traverse till the last node */
    while (last->next != NULL)
        last = last->next;

    /* 6. Change the next of last node */
    last->next = new_node;
    return;
}

// deletes client from linked list
void delete_client(struct Node** head_ref, int client_socket)
{
    // Store head node
    struct Node *temp = *head_ref, *prev;
 
    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->client_socket == client_socket) {
        *head_ref = temp->next;  // Changed head
        free(temp);              // free old 
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'

    while (temp != NULL){

        if(temp->client_socket == client_socket){
                break;
        }
        prev = temp;
        temp = temp->next;
    }
    
    // If key was not present in linked list
    if (temp == NULL){
		printf("\n client not found \n");
        return;
    }

    // Unlink the node from linked list
    prev->next = temp->next;
 
    free(temp); // Free memory
}

// prints client_list
void print_list(struct Node* node)
{
	printf("______________Client List Start____________\n");
    while (node != NULL) {
		printf("Address: %s\n", node->address);
        printf("Port: %d\n", node->port);
        printf("Socket: %d\n", node->client_socket);
        node = node->next;
    }
    printf("_______________Client List End_____________\n");
}

// adds new_paddle data do local paddle_list (used on client)
void add_player_list(struct Paddle_Node** head_ref, paddle_position_t new_paddle, int player_score, bool current_player)
{
    /* 1. allocate node */
    struct Paddle_Node* new_node = (struct Paddle_Node*) malloc(sizeof(struct Paddle_Node));
    struct Paddle_Node *last = *head_ref;  /* used in step 5*/

    /* 2. put in the data  */
    new_node->paddle = new_paddle;
    new_node->player_score = player_score;
    new_node->current_player = current_player;

    /* 3. This new node is going to be the last node, so make next
          of it as NULL*/// Internet domain sockets libraries
    new_node->next = NULL;

    /* 4. If the Linked List is empty, then make the new node as head */
    if (*head_ref == NULL)
    {
       *head_ref = new_node;
       return;
    } 

    /* 5. Else traverse till the last node */
    while (last->next != NULL)
        last = last->next;

    /* 6. Change the next of last node */
    last->next = new_node;
    return;
}

// paddle_hit_balls the client list with the new paddle position calculated according with the players pressed key
void update_paddle(struct Node** client_list, int client_socket, int key)
{
    struct Node *temp = *client_list;
    paddle_position_t paddle;
    paddle_position_t previous_position;

    while (temp != NULL){

        // searches for the active player address on the client_list
        if(temp->client_socket==client_socket){

            paddle = temp->paddle;
            previous_position = temp->paddle;

            moove_paddle(&paddle, key);
                                              
            // check if new position collides with any of the other player's paddles
            // if it does, the paddle doesn't move
            if(paddle_hit_paddle(paddle, *client_list, client_socket)){
                temp->paddle = previous_position;
                temp->previous_paddle = previous_position;
            }
            // otherwise the new paddle position is updated to the list
            else{
                temp->paddle = paddle;
                temp->previous_paddle = previous_position;
            }
            return;
        }
        temp = temp->next;
    }
    exit(0);             // add message error if client not found    
}

/* deletes the entire linked list */
void reset_list(struct Paddle_Node** head_ref)
{
 
    /* deref head_ref to get the real head */
    struct Paddle_Node* current = *head_ref;
    struct Paddle_Node* next = NULL;
 
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
 
    /* deref head_ref to affect the real head back
        in the caller. */
    *head_ref = NULL;
}

// draws all the paddles on paddle_list
void draw_all_paddles(WINDOW *win, struct Paddle_Node* paddle_list, bool del){
    struct Paddle_Node* temp;
    temp = paddle_list;
    while (temp != NULL) {
        draw_paddle(win, &(temp->paddle), temp->current_player, del);
        temp = temp->next;
    }
}



// check if new paddle position coincides with any of other players paddle or with the ball position
// returns true if overlap is detected, returns false otherwise (valid position in this case)
bool paddle_hit_paddle(paddle_position_t new_paddle_position, struct Node* client_list, int client_socket){
    
    struct Node* temp;
    temp = client_list;

    int start_x = new_paddle_position.x - new_paddle_position.length;
    int end_x = new_paddle_position.x + new_paddle_position.length;
    
    int rival_start_x;
    int rival_end_x;

    while(temp!=NULL){
        
        // Check if the current node is the one refering to the active player (ignore in this case)
        if(temp->client_socket==client_socket){
            temp = temp->next;
            continue;
        }
        
        // Check if y positions of the active player and the rival player are the same
        if(new_paddle_position.y == temp->paddle.y){
            rival_start_x = temp->paddle.x - temp->paddle.length;
            rival_end_x = temp->paddle.x + temp->paddle.length;

            // check if the other player paddle is respectively on the left or on the right of the new position or coincides with it (assumes all paddle with same length)
            if( (rival_end_x >= start_x  && rival_start_x < start_x)  || (rival_start_x <= end_x && rival_end_x>end_x) || (rival_start_x==start_x && rival_end_x==end_x) ){
                return true;
            }

        }

        temp = temp->next;
    }

    return false;
}

// updates the ball movement when it is hit by the paddle; updates player score
paddle_hit_ball(ball_position_t * ball, struct Node ** client_list, ball_position_t * previous_ball){
    
    struct Node *temp = *client_list;
    paddle_position_t paddle;
    paddle_position_t old_paddle;
    
    // for each player paddle, checks collision
    while(temp!=NULL){
        paddle = temp->paddle;
        old_paddle = temp->previous_paddle;

        int start_x = paddle.x - paddle.length;
        int end_x = paddle.x + paddle.length;

        // Checks if y positions are the same (same row)
        if (ball->y == paddle.y){
            // Runs through the whole length of the paddle
            for (int i = start_x; i <= end_x; i++){
                // If the ball x position coincides with the paddle
                if (ball->x == i){

                    temp->player_score++; // increments player score

                    // Case 1: the ball and the paddle are moving horizontally
                    if (ball->up_hor_down == 0 && paddle.y == old_paddle.y){
                        ball->left_ver_right *= -1;
                        ball->x=ball->x + ball->left_ver_right;
                    }
                    // Case 2: the ball is moving horizontally and the paddle vertically
                    else if ((ball->up_hor_down == 0 && paddle.y != old_paddle.y)){
                        if (old_paddle.y < paddle.y)
                            ball->up_hor_down = 1;
                        else
                            ball->up_hor_down = -1;
                        ball->y = ball->y + ball->up_hor_down;
                    }
                    // Case 3: the other scenarios
                    else{
                        ball->up_hor_down *= -1;
                        ball->y=ball->y + ball->up_hor_down;
                    }
                    // Check for window limits
                    if( ball->y == 0 || ball->y == WINDOW_SIZE-1){
                        ball->up_hor_down *= -1;
                        ball->left_ver_right = rand() % 3 -1;
                    }
                }
            }
        }

        // Check for the case where the ball and the paddle swap y positions
        if (paddle.y == previous_ball->y && ball->y == old_paddle.y && paddle.y != old_paddle.y && ball->y != previous_ball -> y){
            for (int i = start_x; i <= end_x; i++){
                if (ball->x == i){
                    temp->player_score++; // increments player score
                    ball->up_hor_down *= -1;
                    ball->y=ball->y + 2*ball->up_hor_down; 
                } 
            }
            // Check for window limits
            if( ball->y == 0 || ball->y == WINDOW_SIZE-1){
                ball->up_hor_down *= -1;
                ball->left_ver_right = rand() % 3 -1;
            }
        }
        temp = temp->next;
    }
}