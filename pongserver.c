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
        *head_ref = temp->next; // Changed head
        free(temp); // free old head
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && strcmp(temp->address, delete_address)!=0 && temp->port != delete_port) {
        prev = temp;
        temp = temp->next;
    }
 
    // If key was not present in linked list
    if (temp == NULL)
		printf("client not found");
        return;
 
    // Unlink the node from linked list
    prev->next = temp->next;
 
    free(temp); // Free memory
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

    char reply_message[100];
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);

	while(1){

		printf("...");
		nbytes = recvfrom(sock_fd, &m, sizeof(struct message), 0,
		                  ( struct sockaddr *)&client_addr, &client_addr_size);

		char remote_addr_str[100];
		int remote_port = ntohs(client_addr.sin_port);
		if (inet_ntop(AF_INET, &client_addr.sin_addr, remote_addr_str, 100) == NULL){
			perror("converting remote addr: ");
		}
		//printf("received %d bytes from %s %d:\n", nbytes, remote_addr_str, remote_port);


		if(m.msg_type == 'c'){
			printf("connection message detected!\n");
			//printf("remote_addr_str: %s remote_port: %d", remote_addr_str, remote_port);
			
			//adicionar cliente â lista
			add_client(&client_list, remote_addr_str, remote_port);

			char reply_message[100] = "client added to the list";		//this was for debug, will be deleted
			nbytes = sendto(sock_fd, reply_message, strlen(reply_message)+1, 0, &client_addr, client_addr_size);
		}

		else if(m.msg_type == 'd'){
			printf("disconnection message detected!\n");

			delete_client(&client_list, remote_addr_str, remote_port);
		}

		printList(client_list);
/*
		char linha[100];
		printf("going to send a reply (press enter to continue\n");
		fgets(linha, 100, stdin);

		nbytes = sendto(sock_fd,
	                    reply_message, strlen(reply_message)+1, 0,
	                    (const struct sockaddr *) &client_addr, client_addr_size);
		printf("\nsent %d %s\n\n", nbytes, reply_message);
*/
    }

    close(sock_fd);
	exit(0);
}

/*
        if(m.msg_type == 0){
            ch = m.ch;
            pos_x = WINDOW_SIZE/2;
            pos_y = WINDOW_SIZE/2;

            //STEP 3
            char_data[n_chars].ch = ch;
            char_data[n_chars].pos_x = pos_x;
            char_data[n_chars].pos_y = pos_y;
            n_chars++;
        }    
*/

