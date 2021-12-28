#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>


#include<ctype.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <ncurses.h>
#include "pong.h"
#include "sock_dg_inet.h"




// A linked list node
struct Node {
    char address[100];
	int port;
    struct Node* next;
};

/* Given a reference (pointer to pointer) to the head
   of a list and an int, appends a new node at the end  */
void add_client(struct Node** head_ref, char new_address[], int new_port)
{
    /* 1. allocate node */
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
    struct Node *last = *head_ref;  /* used in step 5*/

    /* 2. put in the data  */
	strcpy(new_node->address, new_address);
	new_node->port = new_port;

    /* 3. This new node is going to be the last node, so make next
          of it as NULL*/
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

void sendto_nextplayer(int sock_fd, struct Node** head_ref, char player_address[], int player_port){
   
   struct Node *temp = *head_ref;
   struct Node *next_player;
   struct Node *first_node = *head_ref;

   int next_player_port;
   char next_player_address[100];

    if (*head_ref == NULL){
        printf("No players present");
        return;
    }


    while (temp->next != NULL) {

        if(strcmp(temp->address, player_address)==0 && temp->port == player_port){
            break;
        }
        temp = temp->next;
    }
 
    // If key was not present in linked list
    if (temp == NULL){
		printf("player not found");
        return;
    }

    if (temp->next==NULL){
        // if the current player is the last on the list (including the case where it is the only client of the list),
        // send to the first player of the list

        next_player_port = first_node->port;
        strcpy(next_player_address, first_node->address);
    }

    else{
        next_player = temp->next;
        next_player_port = (next_player)->port;
        strcpy(next_player_address, next_player->address);
    }
    send_playmessage(sock_fd, next_player_address, next_player_port);
}

void printList(struct Node* node)
{
	printf("______________Client List____________\n");
    while (node != NULL) {
		printf("Address: %s\n", node->address);
        printf("Port: %d\n", node->port);
        node = node->next;
    }
}
 

void delete_client(struct Node** head_ref, char delete_address[], int delete_port)
{
    // Store head node
    struct Node *temp = *head_ref, *prev;
 
    // If head node itself holds the key to be deleted
    if (temp != NULL && strcmp(temp->address, delete_address)==0 && temp->port == delete_port) {
        printf("eliminated first element\n");
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

void send_playmessage(int sock_fd, char addr[], int port){
    // will need to expand arguments: include game state variables, such as ball position and paddle

    int nbytes;
    struct message m;
    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    player_addr.sin_family = AF_INET;
    player_addr.sin_port = htons(port);
    if( inet_pton(AF_INET, addr, &player_addr.sin_addr) < 1){
        printf("no valid address: \n");
        exit(-1);
    }

    // create message  (will need to expand struct message)
    m.msg_type = 's';
    nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);
}

int main()
{

	int sock_fd;
	sock_fd= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(SOCK_PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	int err = bind(sock_fd, (struct sockaddr *)&local_addr,
							sizeof(local_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

	char buffer[100];
	int nbytes;
	struct Node* client_list = NULL;
	struct message m;
    int gameOn = 0;

    char reply_message[100];
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);

	while(1){
		nbytes = recvfrom(sock_fd, &m, sizeof(struct message), 0,
		                  ( struct sockaddr *)&client_addr, &client_addr_size);

		char remote_addr_str[100];
		int remote_port = ntohs(client_addr.sin_port);
		if (inet_ntop(AF_INET, &client_addr.sin_addr, remote_addr_str, 100) == NULL){
			perror("converting remote addr: ");
		}

		if(m.msg_type == 'c'){
			printf("connection message detected!\n");

			//adicionar cliente â lista
			add_client(&client_list, remote_addr_str, remote_port);

            if(gameOn==0){
                //first player joining the server
                gameOn = 1;

                //send play_message to the player
                send_playmessage(sock_fd, remote_addr_str, remote_port);
            }
		}

        else if(m.msg_type == 'r'){
            printf("release ball message detected!\n");
            sendto_nextplayer(sock_fd, &client_list, remote_addr_str, remote_port);
        }    

		else if(m.msg_type == 'd'){
			printf("disconnection message detected!\n");
            
            printf("deleting player %s with port: %d", remote_addr_str, remote_port);
			delete_client(&client_list, remote_addr_str, remote_port);
            sendto_nextplayer(sock_fd, &client_list, remote_addr_str, remote_port);
		}

		printList(client_list);
    }

    close(sock_fd);
	exit(0);
}

