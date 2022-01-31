// Internet domain sockets libraries
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

// Include threads library
#include <pthread.h> 

// General libraries
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>


// Include header files
#include "client_list.h" 
#include "visualization.h"
#include "pong.h"			
#include "sock_dg_inet.h"

// Global variables
ball_position_t ball;
int n_clients = 0;
struct Node* client_list = NULL;
int sock_fd;
struct message m;
struct sockaddr_in client_addr; 

// Initialize paddle position and ball position variables
paddle_position_t paddle;
ball_position_t previous_ball;


void* listen_to_client_thread(void* arg){
    long int client_socket = *(int*) arg;
    printf("client_socket: %ld \n", client_socket);
    message m;
    int backlog = 10; // ??
    int nbytes=0;
    printf("entered listen_to_client_thread\n");
    while(1){

        while(nbytes<=0){
            nbytes = recv(client_socket, &m, sizeof(struct message), 0);
        }
        printf("received %d bytes \n", nbytes);

        nbytes = 0;
        int pressed_key;  

        char remote_addr_str[100];
        int remote_port; // remove this two

        switch(m.command){
            case PADDLE_MOVE:
                // processes player key input, calculates new paddle position, updates the ball
                // sends board_update message to all clients (ball position and paddles from all clients)
                printf("paddle move message detected");

                pressed_key = m.pressed_key;
                update_paddle(&client_list, client_socket, ball, pressed_key);
                previous_ball = ball;
                moove_ball(&ball);                      // calculates new ball position every "n_clients" PADDLE_MOVE messages
                paddle_hit_ball(&ball, &client_list, &previous_ball);   // update the ball movement when it hits the paddle; update player score                                        
                send_board_update(sock_fd, client_socket, ball, client_list);  // sends board_update message to client with new data
                break;

            case DISCONNECT:
                // removes player from the game; invites player from the waiting list
                printf("disconnection message detected!\n");
                n_clients--;
                delete_client(&client_list, client_socket);  // deletes client entry on client_list

                break;
        }
    }
}


void* new_connection_thread(void* arg){
    
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    int backlog = 10; // ??
    int client_socket;

    while(1){
        listen(sock_fd, backlog);  // doesnt block
        
        client_socket = accept(sock_fd,(struct sockaddr *)&client_addr, &client_addr_size); // blocks -> use thread! creates new socket; used to read/write to client; original socket available for new connections
        
        char remote_addr_str[100];
        int remote_port = ntohs(client_addr.sin_port);
        if (inet_ntop(AF_INET, &client_addr.sin_addr, remote_addr_str, 100) == NULL){
            perror("converting remote addr: ");
        }
        printf("estabilished connection: %s %d:\n", remote_addr_str, remote_port);
        


        if(n_clients == 0){
            place_ball_random(&ball);   //random initialization of ball position
            // start ball thread?
        }
        n_clients++;

        paddle_position_t paddle;
        // random initialization of paddle position
        new_paddle(&paddle, PADDLE_SIZE);
        // loops if new paddle position collides with any of the other clients until valid position has been generated                  
        while(paddle_hit_paddle(paddle, &ball, client_list, client_socket)){
            new_paddle(&paddle, PADDLE_SIZE);
        }
        // adds client to list
        add_client(&client_list, remote_addr_str, remote_port, paddle, client_socket);

        //sends board_update message to the client
        send_board_update(sock_fd, client_socket, ball, client_list);

        //create a thread to receive messages from the client 
        pthread_t listen_to_client_thread_id;
	    pthread_create(&listen_to_client_thread_id, NULL, listen_to_client_thread, &client_socket);
    }
}



// sends board_update message to all the clients, with the information of all players paddle positions and scores
void send_board_update(int sock_fd, int client_socket, ball_position_t ball, struct Node* client_list){

    int nbytes;
    message m;
    paddle_position_message paddle_m;

    // create general board_update message 
    m.command = BOARD_UPDATE;
    m.ball_position = ball;
    m.number_clients = n_clients;

    
    // goes through client_list and sends to the client the paddle positions and scores of each player
    // client builds a paddle list with this data
    for(int i=0; i<n_clients; i++){

        // send general board_update message
        nbytes = send(client_list->client_socket, &m, sizeof(struct message), 0);
        // Error handling (returns -1 if there is an error)
        if (nbytes < 0)
            printf("Error sending the message to the client \n");  

        // send paddle_move message
        paddle_m.paddle_position = client_list->paddle;
        paddle_m.player_score = client_list->player_score;
        paddle_m.current_player = false;

        if(client_list->client_socket==client_socket){
            paddle_m.current_player = true;
        }

        nbytes = send(client_list->client_socket, &paddle_m, sizeof(struct paddle_position_message), 0);
        // Error handling (returns -1 if there is an error)
        if (nbytes < 0)
            printf("Error sending the message to the client \n"); 

        client_list = client_list->next;
    }
}

int main()
{
    // Create an internet domain stream socket
	sock_fd= socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(SOCK_PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

    printf("Socket created and binded \n");
	printf("Ready to receive messages \n");

	char buffer[100];
	int nbytes;


    // create thread that receives connection requests from clients
    pthread_t new_connection_thread_id;
	pthread_create(&new_connection_thread_id, NULL, new_connection_thread, NULL);
	
    while(1){
        sleep(3);
        print_list(client_list);    // prints client_list
    }


    close(sock_fd);
	exit(0);
}
