#include "client_list.h"

/* Given a reference (pointer to pointer) to the head
   of a list and an int, appends a new node at the end  */
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

void delete_client(struct Node** head_ref, char delete_address[], int delete_port)
{
    // Store head node
    struct Node *temp = *head_ref, *prev;
 
    // If head node itself holds the key to be deleted
    if (temp != NULL && strcmp(temp->address, delete_address)==0 && temp->port == delete_port) {
        // printf("eliminated first element\n");
        *head_ref = temp->next; // Changed head
        free(temp); // free old head
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

void print_list(struct Node* node)
{
	printf("______________Client List Start____________\n");
    while (node != NULL) {
		printf("Address: %s\n", node->address);
        printf("Port: %d\n", node->port);
        printf("Paddle (x,y): (%d,%d)\n\n", node->paddle.x, node->paddle.y);
        node = node->next;
    }
    printf("_______________Client List End_____________\n");
}

bool next_player(struct Node** head_ref, char player_address[], int player_port){
   
   struct Node *temp = *head_ref;
   struct Node *next_player;
   struct Node *first_node = *head_ref;

//    int next_player_port;
//    char next_player_address[100];

    if (*head_ref == NULL){
        printf("No players present \n");
        return false;
    }

    while (temp->next != NULL) {

        if(strcmp(temp->address, player_address)==0 && temp->port == player_port){
            break;
        }
        temp = temp->next;
    }
 
    // If key was not present in linked list
    if (temp == NULL){
		printf("Player not found \n");
        return false;
    }

    if (temp->next==NULL){
        // if the current player is the last on the list (including the case where it is the only client of the list),
        // send to the first player of the list

        next_player_port = first_node->port;
        strcpy(next_player_address, first_node->address);
        return true;
    }

    else{
        next_player = temp->next;
        next_player_port = next_player->port;
        strcpy(next_player_address, next_player->address);
        return true;
    }
    // st->next_player_address = d;
    // st->next_player_port = m;
    //send_play_message(sock_fd, next_player_address, next_player_port);
}



/* Given a reference (pointer to pointer) to the head
   of a list and an int, appends a new node at the end  */
void add_player_list(struct Paddle_Node** head_ref, paddle_position_t new_paddle, int player_score)
{
    /* 1. allocate node */
    struct Paddle_Node* new_node = (struct Paddle_Node*) malloc(sizeof(struct Paddle_Node));
    struct Paddle_Node *last = *head_ref;  /* used in step 5*/

    /* 2. put in the data  */
    new_node->paddle = new_paddle;
    new_node->player_score = player_score;

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


// Updates the client list with the new paddle position of the player
void update_paddle(struct Node** client_list, char remote_addr_str[], int remote_port, ball_position_t ball, int key)
{
    struct Node *temp = *client_list;
    paddle_position_t paddle;

    while (temp != NULL){
        if(temp->port==remote_port && strcmp(temp->address, remote_addr_str)==0){
            paddle = temp->paddle;
            moove_paddle(&paddle, key);       // add function to check if it is a valid move (no collisions)
                                              // moove_paddle(CLIENTLIST, &paddle, key)
            return;
        }
    }
    exit(0);      // add message error if client not found    
}

/*
void draw_all_paddles(WINDOW *win, struct Node* client_list, int del){
    struct Node* temp;
    temp = client_list;
    while (temp != NULL) {
        //draw_paddle(win, temp->paddle, true);
        temp = temp->next;
    }
}
*/