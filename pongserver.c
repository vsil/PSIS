// Internet domain sockets libraries
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

// ncurses library
#include <ncurses.h>

// Include header files
#include "client_list.h"       
#include "pong.h"
#include "sock_dg_inet.h"


void send_wait_list_message(int sock_fd, char addr[], int port, int n_clients){

    int nbytes;
    message m;

    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    player_addr.sin_family = AF_INET;    // INET domain
    player_addr.sin_port = htons(port);  // translates the port to network byte order
    // Player address converted to binary + error handling
    if( inet_pton(AF_INET, addr, &player_addr.sin_addr) < 1){
        printf("no valid address: \n");
        exit(-1);
    }

    // Create message to be sent
    m.command = WAIT_LIST;
    m.number_clients = n_clients;
    printf("waiting clients: %d \n", m.number_clients); 
    // Send the message                   
    nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);
}

void send_board_update(int sock_fd, char addr[], int port, ball_position_t ball, struct Node* client_list, int n_clients){

    int nbytes;
    message m;
    paddle_position_message paddle_m;

    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    player_addr.sin_family = AF_INET;     // INET domain
    player_addr.sin_port = htons(port);   // translates the port to network byte order
    // Player adress converted to binary + error handling
    if( inet_pton(AF_INET, addr, &player_addr.sin_addr) < 1){
        printf("no valid address: \n");
        exit(-1);
    }

    // Create board_update message 
    m.command = BOARD_UPDATE;
    m.ball_position = ball;
    m.number_clients = n_clients;
    // Send the message to the target client
    nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);

    /* Go through client list and send to the client the paddle info of each player
     client builds a paddle list with this information */
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
    // Create an internet domain datagram socket (ipv4)
	int sock_fd;
	sock_fd= socket(AF_INET, SOCK_DGRAM, 0);
    // Error handling
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

    /* Server adress */
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;           // INET domain (ipv4)
	local_addr.sin_port = htons(SOCK_PORT);    // translates the port to network byte order
    // Allow the program to receive messages sent to any address (available on that computer)
	local_addr.sin_addr.s_addr = INADDR_ANY;

    // Assign/bind one adress
	int err = bind(sock_fd, (struct sockaddr *)&local_addr,
							sizeof(local_addr));
    
    // Check for any bind error
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

    printf("Socket created and binded \n");
	printf("Ready to receive messages \n");

    // Variables initialization
	char buffer[100];
	int nbytes;
	struct Node* client_list = NULL;
    struct Node* waiting_list = NULL;
	struct message m;

	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);

    // Initialize paddle position and ball position variables
	paddle_position_t paddle;
	ball_position_t ball;

    int n_clients = 0;
    int n_waiting_list = 0;
    int n_ball_turn = 0;
    int pressed_key;    // Keyboard key received from client

	while(1){
        // Waits for a message from any client (receives the number of bytes)
		nbytes = recvfrom(sock_fd, &m, sizeof(struct message), 0,
		                  ( struct sockaddr *)&client_addr, &client_addr_size);
        if (nbytes <= 0)
            printf("Error receiving the message \n");

		char remote_addr_str[100];
        // Network to host byte order conversion
		int remote_port = ntohs(client_addr.sin_port);
        // Converts the network address into a character string
		if (inet_ntop(AF_INET, &client_addr.sin_addr, remote_addr_str, 100) == NULL){
			perror("converting remote addr: ");
		}
        printf("received %d bytes from %s %d:\n", nbytes, remote_addr_str, remote_port);

        switch(m.command){
            case CONNECT:
                printf("connection message detected!\n");
                n_clients++;

                if(n_clients == 1){
                    // Random initialization of ball position
                    place_ball_random(&ball);
                }
                if(n_clients > MAX_CLIENTS){
                    printf("Maximum number of clients reached\n Client added to the waiting list");
                    n_waiting_list++;
                    send_wait_list_message(sock_fd, remote_addr_str, remote_port, n_waiting_list);
                    // adds client to waiting list (argument paddle is ignored) 
                    add_client(&waiting_list, remote_addr_str, remote_port, paddle);                                           
                }

                // Random initialization of paddle position
                new_paddle(&paddle, PADDLE_SIZE);
                /* Checks if the new paddle position collides with any of the other paddles,
                if it collides, draws a new paddle until it does not collide*/
                while(paddle_hit_paddle(paddle, client_list, remote_addr_str, remote_port)){
                    new_paddle(&paddle, PADDLE_SIZE);
                }

			    // Add client to the list
			    add_client(&client_list, remote_addr_str, remote_port, paddle);

                // Sends board_update message to the client
                send_board_update(sock_fd, remote_addr_str, remote_port, ball, client_list, n_clients);
                break;

            case PADDLE_MOVE:
                // Calculates new paddle position, updates the ball
                // Sends board_update message to all clients (ball position and paddles from all clients)
                
                pressed_key = m.pressed_key;
                printf("paddle move message detected");

                update_paddle(&client_list, remote_addr_str, remote_port, ball, pressed_key);

                n_ball_turn++;
                if(n_ball_turn==n_clients){
                    // calculates new ball position every n PADDLE_MOVE messages
                    moove_ball(&ball);
                    paddle_hit_ball(&ball, &client_list);
                    n_ball_turn=0;
                }

                send_board_update(sock_fd, remote_addr_str, remote_port, ball, client_list, n_clients);
                break;

            case DISCONNECT:

                printf("disconnection message detected!\n");
                n_clients--;
                printf("deleting player %s with port: %d \n", remote_addr_str, remote_port);
                // Delete client from the client list
		        delete_client(&client_list, remote_addr_str, remote_port);

                /* Ckeks if there is any client in the waiting list, if so,
                 adds the client to the list and sends a board update message*/    
                if(waiting_list!=NULL){
                    address player_waiting_addr;
                    player_waiting_addr = add_client_from_waiting_list(&client_list, &waiting_list);
                    n_waiting_list--;
                    n_clients++;
                    send_board_update(sock_fd, player_waiting_addr.addr, player_waiting_addr.port, ball, client_list, n_clients);
                }

                break;
        }
		print_list(client_list);
    }
    // Closes the socket and terminates the server
    close(sock_fd);
	exit(0);
}
