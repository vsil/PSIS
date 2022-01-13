#include "client_list.h"

// adds new client to the list (used on server)
void add_client(struct Node** head_ref, char new_address[], int new_port, paddle_position_t new_paddle)
{
    /* 1. allocate node */
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
    struct Node *last = *head_ref;  /* used in step 5*/

    /* 2. put in the data  */
	strcpy(new_node->address, new_address);
	new_node->port = new_port;
    new_node->paddle = new_paddle;
    new_node->player_score = 0;

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

// adds new player to client_list from the waiting_list; returns new player address
address add_client_from_waiting_list(struct Node** head_client_list, struct Node** head_waiting_list, ball_position_t* ball, address player_waiting_addr){

    struct Node *temp = *head_waiting_list;
    paddle_position_t paddle;
    //address player_waiting_addr;

    // new player address is the first client on the waiting_list
    strcpy(player_waiting_addr.addr, temp->address);
    player_waiting_addr.port = temp->port;

    // random initialization of its paddle position
    new_paddle(&paddle, PADDLE_SIZE);
    // check if new paddle position collides with any of the other clients or with ball position                   
    while(paddle_hit_paddle(paddle, ball, *head_client_list, temp->address, temp->port)){
        new_paddle(&paddle, PADDLE_SIZE);
    }    

    add_client(head_client_list, temp->address, temp->port, paddle);    // adds new client to the client_list
    delete_client(head_waiting_list, (*head_waiting_list)->address, (*head_waiting_list)->port);  // removes the new client from the waiting list
    return player_waiting_addr;     // returns new player address
}

// deletes client from linked list
void delete_client(struct Node** head_ref, char delete_address[], int delete_port)
{
    // Store head node
    struct Node *temp = *head_ref, *prev;
 
    // If head node itself holds the key to be deleted
    if (temp != NULL && strcmp(temp->address, delete_address)==0 && temp->port == delete_port) {
        *head_ref = temp->next;  // Changed head
        free(temp);              // free old 
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'

    while (temp != NULL){

        if(temp->port==delete_port && strcmp(temp->address, delete_address)==0){
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
        node = node->next;
    }
    printf("_______________Client List End_____________\n");
}


/*  // prints paddle_list (used for development only)
void print_paddle_list(struct Paddle_Node* node)
{
	printf("______________Client List Start____________\n");
    while (node != NULL) {
        printf("Paddle (x,y): (%d,%d)\n\n", node->paddle.x, node->paddle.y);
        node = node->next;
    }
    printf("_______________Client List End_____________\n");
}
*/

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


// check if new paddle position coincides with any of other players paddle or with the ball position
// returns true if overlap is detected, returns false otherwise (valid position in this case)
bool paddle_hit_paddle(paddle_position_t new_paddle_position, ball_position_t* ball, struct Node* client_list, char player_address[], int player_port){
    
    struct Node* temp;
    temp = client_list;

    int start_x = new_paddle_position.x - new_paddle_position.length;
    int end_x = new_paddle_position.x + new_paddle_position.length;
    
    int rival_start_x;
    int rival_end_x;

    while(temp!=NULL){
        
        // Check if the current node is the one refering to the active player (ignore in this case)
        if(strcmp(temp->address, player_address)==0 && temp->port == player_port){
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

        // check if any x of the new paddle position coincides with ball position
        if(new_paddle_position.y == ball->y){
            for (int i = start_x; i <= end_x; i++){
                if(ball->x == i) 
                    return true;
            }
        }
        temp = temp->next;
    }

    return false;
}
    

// updates the client list with the new paddle position calculated according with the players pressed key
void update_paddle(struct Node** client_list, char remote_addr_str[], int remote_port, ball_position_t ball, int key)
{
    struct Node *temp = *client_list;
    paddle_position_t paddle;
    paddle_position_t previous_position;

    while (temp != NULL){

        // searches for the active player address on the client_list
        if(temp->port==remote_port && strcmp(temp->address, remote_addr_str)==0){

            paddle = temp->paddle;
            previous_position = temp->paddle;

            moove_paddle(&paddle, key);
                                              
            // check if new position collides with any of the other player's paddles
            // if it does, the paddle doesn't move
            if(paddle_hit_paddle(paddle, &ball, *client_list, remote_addr_str, remote_port)){
                temp->paddle = previous_position;
            }
            // otherwise the new paddle position is updated to the list
            else{
                temp->paddle = paddle;
            }
            return;
        }
        temp = temp->next;
    }
    exit(0);             // add message error if client not found    
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

// updates the ball movement when it is hit by the paddle; updates player score
void paddle_hit_ball(ball_position_t * ball, struct Node ** client_list){
    
    struct Node *temp = *client_list;
    
    // for each player paddle, checks collision
    while(temp!=NULL){
        int start_x = temp->paddle.x - temp->paddle.length;
        int end_x = temp->paddle.x + temp->paddle.length;

        // Check if y positions are the same
        if (ball->y == temp->paddle.y){
            // Run through the whole length of the paddle
            for (int i = start_x; i <= end_x; i++){

                if (ball->x == i){
                    temp->player_score++;           // increments player score if ball hits paddle
                
                    if (ball->up_hor_down == 0){
                        do{
                            ball->up_hor_down = rand() % 3 -1;  // randomly changes ball direction when it hits horizontally
                        }while(ball->up_hor_down == 0);
                    }
                    else
                        ball->up_hor_down *= -1;         // inverts ball direction
                    
                    ball->y=ball->y + ball->up_hor_down;

                    // Check for window limits
                    if( ball->y == 0 || ball->y == WINDOW_SIZE-1){
                        ball->up_hor_down *= -1;
                        ball->left_ver_right = rand() % 3 -1;
                    }
                }
            }
        }
        temp = temp->next;
    }
}