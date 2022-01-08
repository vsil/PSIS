// Internet domain sockets libraries
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

// ncurses library
#include <ncurses.h>

// Include header files
#include "client_list.h"        // "visualization.h" included here
#include "pong.h"
#include "sock_dg_inet.h"



void send_board_update(int sock_fd, char addr[], int port, ball_position_t ball, struct Node* client_list, int n_clients){

    int nbytes;
    message m;
    paddle_position_message paddle_m;

    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    player_addr.sin_family = AF_INET;    // INET domain
    player_addr.sin_port = htons(port);
    if( inet_pton(AF_INET, addr, &player_addr.sin_addr) < 1){
        printf("no valid address: \n");
        exit(-1);
    }

    // create general message 
    m.command = BOARD_UPDATE;
    m.ball_position = ball;
    m.number_clients = n_clients;

    nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);

    // go through client list and send to the client the paddle info of each player
    // client builds a paddle list with this information
    for(int i=0; i<n_clients; i++){
        paddle_m.paddle_position = client_list->paddle;
        paddle_m.player_score = client_list->player_score;
        paddle_m.current_player = false;


        if(client_list->port==port && strcmp(client_list->address, addr)==0){
            paddle_m.current_player = true;
        }

        nbytes = sendto(sock_fd, &paddle_m, sizeof(struct paddle_position_message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);

        client_list = client_list->next;
    }
}


void send_move_message(int sock_fd, struct Node** head_ref, char player_address[], int player_port, ball_position_t ball){

    int nbytes;
    struct message m;
    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    /* Message to be sent */
    //m.command = MOVE;
    m.ball_position = ball;

    // Store head node
    struct Node *temp = *head_ref;

    while (temp != NULL){
        if(!(temp->port==player_port && strcmp(temp->address, player_address)==0)){
            player_addr.sin_family = AF_INET;    // INET domain
            player_addr.sin_port = htons(temp->port);
            if( inet_pton(AF_INET, temp->address, &player_addr.sin_addr) < 1){
                printf("no valid address: \n");
                exit(-1);
            }
            nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);
        }
        temp = temp->next;
    }
}


int main()
{
    // Create an internet domain datagram socket
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

    printf("Socket created and binded \n");
	printf("Ready to receive messages \n");

	char buffer[100];
	int nbytes;
	struct Node* client_list = NULL;
	struct message m;

	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);

    int n_clients = 0;

    // Initialize paddle position and ball position variables
	paddle_position_t paddle;
	ball_position_t ball;

	while(1){

		nbytes = recvfrom(sock_fd, &m, sizeof(struct message), 0,
		                  ( struct sockaddr *)&client_addr, &client_addr_size);

		char remote_addr_str[100];
		int remote_port = ntohs(client_addr.sin_port);
		if (inet_ntop(AF_INET, &client_addr.sin_addr, remote_addr_str, 100) == NULL){
			perror("converting remote addr: ");
		}
        printf("received %d bytes from %s %d:\n", nbytes, remote_addr_str, remote_port);
        
        int pressed_key;

        switch(m.command){

            case CONNECT:
                n_clients++;
                printf("connection message detected!\n");

                if(n_clients == 1){
                    //random initialization of ball position
                    place_ball_random(&ball);
                }

                else if(n_clients>10){
                    printf("Maximum number of clients reached");
                    break;                                          // check if this workd
                }

                //random initialization of paddle position
                new_paddle(&paddle, PADDLE_SIZE);                   // MAKE THIS FUNC RANDOM
                
			    //adicionar cliente â lista
			    add_client(&client_list, remote_addr_str, remote_port, paddle);

                //sends board_update message to the client
                send_board_update(sock_fd, remote_addr_str, remote_port, ball, client_list, n_clients);
                break;

            case PADDLE_MOVE:
                // calculates new paddle position, updates the ball
                // sends board_update message to all clients (ball position and paddles from all clients)
                
                pressed_key = m.pressed_key;

                /* BALL MOVEMENT (from the paper)
                "The ball should move (following the code in the provided skeleton) after every n
                Paddle_move messages received from the clients (where n is the number of connected
                clients)"    - dont quite understand this
                */

                printf("Paddle_Move message received (pressed key: %d)\n", pressed_key);
                update_paddle(&client_list, remote_addr_str, remote_port, ball, pressed_key);

                send_board_update(sock_fd, remote_addr_str, remote_port, ball, client_list, n_clients);


                break;

            case DISCONNECT:
                n_clients--;
                printf("disconnection message detected!\n");     
                printf("deleting player %s with port: %d \n", remote_addr_str, remote_port);
		        delete_client(&client_list, remote_addr_str, remote_port);
				if (next_player(&client_list, remote_addr_str, remote_port))
							//send_play_message(sock_fd, next_player_address, next_player_port);
                break;
        }
		print_list(client_list);
    }

    close(sock_fd);
	exit(0);
}
