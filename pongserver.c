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


// sends wait_list message to new client added to waiting list when max server capacity has been reached
void send_wait_list_message(int sock_fd, char addr[], int port, int n_clients){

    int nbytes;
    message m;
    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    player_addr.sin_family = AF_INET;    // INET domain
    player_addr.sin_port = htons(port);
    if( inet_pton(AF_INET, addr, &player_addr.sin_addr) < 1){
        printf("no valid address: \n");
        exit(-1);
    }

    // create general message 
    m.command = WAIT_LIST;
    m.number_clients = n_clients;
    nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);
}

// sends board_update message, with the information of all players paddle positions and scores
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

    // create general board_update message 
    m.command = BOARD_UPDATE;
    m.ball_position = ball;
    m.number_clients = n_clients;

    nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);

    // goes through client_list and sends to the client the paddle positions and scores of each player
    // client builds a paddle list with this data
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
    struct Node* waiting_list = NULL;
	struct message m;

	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);

    int n_clients = 0;
    int n_waiting_list = 0;
    int n_ball_turn = 0;

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

                printf("connection message detected!\n");

                if(n_clients == 0){
                    place_ball_random(&ball);   //random initialization of ball position
                }

                else if(n_clients>MAX_CLIENTS-1){
                    printf("Maximum number of clients reached\n Client added to the waiting list");
                    send_wait_list_message(sock_fd, remote_addr_str, remote_port, n_waiting_list);
                    n_waiting_list++;
                    add_client(&waiting_list, remote_addr_str, remote_port, paddle);  // adds client to waiting list (paddle argument ignored)
                    break;                                          
                }

                n_clients++;

                // random initialization of paddle position
                new_paddle(&paddle, PADDLE_SIZE);
                // loops if new paddle position collides with any of the other clients until valid position has been generated                  
                while(paddle_hit_paddle(paddle, &ball, client_list, remote_addr_str, remote_port)){
                    new_paddle(&paddle, PADDLE_SIZE);
                }

                // adds client to list
			    add_client(&client_list, remote_addr_str, remote_port, paddle);

                //sends board_update message to the client
                send_board_update(sock_fd, remote_addr_str, remote_port, ball, client_list, n_clients);
                break;

            case PADDLE_MOVE:
                // processes player key input, calculates new paddle position, updates the ball
                // sends board_update message to all clients (ball position and paddles from all clients)
                printf("paddle move message detected");

                pressed_key = m.pressed_key;
                update_paddle(&client_list, remote_addr_str, remote_port, ball, pressed_key);
                
                n_ball_turn++;
                if(n_ball_turn==n_clients){
                    moove_ball(&ball);                      // calculates new ball position every "n_clients" PADDLE_MOVE messages
                    paddle_hit_ball(&ball, &client_list);   // update the ball movement when it hits the paddle; update player score
                    n_ball_turn=0;
                }                                              // ADICIONAR DINAMICA COM DOUBLE PADDLE_HIT_BALL, À SEMELHANCA DO RELAY PONG?

                send_board_update(sock_fd, remote_addr_str, remote_port, ball, client_list, n_clients);  // sends board_update message to client with new data

                break;

            case DISCONNECT:
                // removes player from the game; invites player from the waiting list
                printf("disconnection message detected!\n");
                n_clients--;
		        delete_client(&client_list, remote_addr_str, remote_port);  // deletes client entry on client_list
                
                // adds player from the waiting_list if there is any
                if(waiting_list!=NULL){
                    address player_waiting_addr;
                    player_waiting_addr = add_client_from_waiting_list(&client_list, &waiting_list, &ball, player_waiting_addr);    // transfers the first client that joined the waiting_list to client_list
                    n_waiting_list--;
                    n_clients++;
                    send_board_update(sock_fd, player_waiting_addr.addr, player_waiting_addr.port, ball, client_list, n_clients);
                }

                break;
        }
		print_list(client_list);    // prints client_list
    }

    close(sock_fd);
	exit(0);
}
