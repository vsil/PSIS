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

// Include header files
#include "client_list.h" 
#include "visualization.h"
#include "pong.h"			
#include "sock_dg_inet.h"

// Global variables
ball_position_t ball;
ball_position_t previous_ball;
int n_clients;
struct Node* client_list;
int sock_fd;
struct message m; 
long int current_client_socket;

// Functions prototypes
void* ball_thread(void* arg);
void* listen_to_client_thread(void* arg);
void send_board_update(int sock_fd, int client_socket, ball_position_t ball, struct Node* client_list);

// Thread that does the processing of the ball position every second
void* ball_thread(void* arg){
	int nbytes;
	while(1){
		sleep(1);
        previous_ball = ball;
        moove_ball(&ball);                                                 // calculates new ball position
        paddle_hit_ball(&ball, &client_list, &previous_ball);              // update the ball movement when it hits the paddle; updates player score                                        
        send_board_update(sock_fd, current_client_socket, ball, client_list);  // sends board_update message to all clients with new data
	}
}

// Unique thread for each client that receives messages from the respective client socket;
void* listen_to_client_thread(void* arg){
    long int client_socket = *(long int*) arg;
    message m;
    int backlog = 10;
    int nbytes=0;


    while(1){
        nbytes = recv(client_socket, &m, sizeof(struct message), 0);        
        if (nbytes == 0){
            printf("Received 0 bytes: client disconnected! \n");
            n_clients--;    
            delete_client(&client_list, client_socket);         // deletes client entry on client_list
            //pthread_exit(NULL);
            return NULL;   
        }

        printf("Received %d bytes from client socket %lu \n\n", nbytes, client_socket);

        current_client_socket = client_socket;
        nbytes = 0;
        int pressed_key;  

        switch(m.command){
            case PADDLE_MOVE:
                // processes player key input, calculates new paddle position, updates the ball
                // sends board_update message to all clients (ball position and paddles from all clients)
                printf("Paddle move message detected\n");
                pressed_key = m.pressed_key;
                update_paddle(&client_list, client_socket, pressed_key);
                paddle_hit_ball(&ball, &client_list, &previous_ball);  // update the ball movement when it hits the paddle; update player score                                        
                send_board_update(sock_fd, client_socket, ball, client_list);            
                break;
        }
    }
}

// sends board_update message to all the clients, with the information of all players paddle positions and scores
void send_board_update(int sock_fd, int client_socket, ball_position_t ball, struct Node* client_list){

    int nbytes;
    message m;
    paddle_position_message paddle_m;
    struct Node* temp = client_list;
    struct Node* temp2 = client_list;

    // create general board_update message 
    m.command = BOARD_UPDATE;
    m.ball_position = ball;
    m.number_clients = n_clients;
  
    for(int j=0; j< n_clients; j++){
        // send general board_update message to each client
        nbytes = send(temp->client_socket, &m, sizeof(struct message), 0);
        // Error handling
        if (nbytes < 0)
            printf("Error sending the message to the client \n");

        // goes through client_list and sends to the client the paddle positions and scores of each player
        // client builds a paddle list with this data
        for(int i=0; i< n_clients; i++){
            // send paddle_move message
            paddle_m.paddle_position = temp2->paddle;
            paddle_m.player_score = temp2->player_score;
            paddle_m.current_player = false;

            // check if the receiving client is the local player that controls the paddle
            if(temp->client_socket==temp2->client_socket){
                paddle_m.current_player = true;
            }  
            nbytes = send(temp->client_socket, &paddle_m, sizeof(struct paddle_position_message), 0);
            // Error handling
            if (nbytes < 0)
                printf("Error sending the message to the client \n"); 

            temp2 = temp2->next;
        }
        temp2 = client_list;
        temp = temp->next;
    }
   
}

int main()
{
    // Create an internet domain stream socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
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

    struct sockaddr_in client_addr; 
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    int backlog = 10;           // number of pending connections
    long int client_socket;
    int nbytes;

    n_clients = 0;
    client_list = NULL;

    pthread_t listen_to_client_thread_id[100];
	
    while(1){
        listen(sock_fd, backlog);

        if(n_clients>=MAX_CLIENTS){
            printf("Server has reached maximum capacity. Client connection request rejected \n");
            client_socket = accept(sock_fd,(struct sockaddr *)&client_addr, &client_addr_size);
            close(client_socket);
        }
        else{  
            client_socket = accept(sock_fd,(struct sockaddr *)&client_addr, &client_addr_size);
            char remote_addr_str[100];
            int remote_port = ntohs(client_addr.sin_port);
            if (inet_ntop(AF_INET, &client_addr.sin_addr, remote_addr_str, 100) == NULL){
                perror("converting remote addr: ");
            }
            printf("estabilished connection: %s %d:\n", remote_addr_str, remote_port); 

            if(n_clients == 0){
                place_ball_random(&ball);   //random initialization of ball position
                pthread_t ball_thread_id;
                if (pthread_create(&ball_thread_id, NULL, ball_thread, NULL) != 0) // initialization of ball thread
                    printf("Error creating the receive messages thread \n");
            }

            paddle_position_t paddle;
            // random initialization of paddle position
            do{
                new_paddle(&paddle);    
            }while(paddle_hit_paddle(paddle, client_list, client_socket));
            
            // adds client to list
            add_client(&client_list, remote_addr_str, remote_port, paddle, client_socket);
            print_list(client_list);    // prints client_list
            //sends board_update message to all the clients
            send_board_update(sock_fd, client_socket, ball, client_list);

            //create a thread to receive messages from the client             
            if (pthread_create(&listen_to_client_thread_id[n_clients], NULL, listen_to_client_thread, &client_socket) != 0)
                printf("Error creating the receive messages thread \n");            
            
            printf("New thread created for the client - ID: %lu \n",listen_to_client_thread_id[n_clients]);

            n_clients++;
        }
    }
    close(sock_fd);
	exit(0);
}
