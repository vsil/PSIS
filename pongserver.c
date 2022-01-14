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


// Create a function to send a PLAY command to a client
void send_play_message(int sock_fd, char addr[], int port){

    int nbytes;
    message m;
    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    player_addr.sin_family = AF_INET;    // INET domain (ipv4)
    player_addr.sin_port = htons(port);  // translates the port to network byte order
    // Player address converted to binary + error handling
    if( inet_pton(AF_INET, addr, &player_addr.sin_addr) < 1){
        printf("no valid address: \n");
        exit(-1);
    }

    // Message to be sent
    m.command = SEND;
    // Send the message
    nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);
    // Error handling (returns -1 if there is an error)
    if (nbytes < 0)
        printf("Error sending the message to the client \n");                
}

// Create a function to send a MOVE command to a client as well as the updated ball position
void send_move_message(int sock_fd, struct Node** head_ref, char player_address[], int player_port, ball_position_t ball){

    int nbytes;
    message m;
    struct sockaddr_in player_addr;
    socklen_t player_addr_size = sizeof(struct sockaddr_in);  

    // Message to be sent
    m.command = MOVE;
    m.ball_position = ball;

    // Store head node
    struct Node *temp = *head_ref;

    // Run through the client list
    // Send the message to all the clients in the list (except the active one)
    while (temp != NULL){
        if(!(temp->port==player_port && strcmp(temp->address, player_address)==0)){
            player_addr.sin_family = AF_INET;           // INET domain (ipv4)
            player_addr.sin_port = htons(temp->port);   // translates the port to network byte order
            // Player adress converted to binary + error handling
            if( inet_pton(AF_INET, temp->address, &player_addr.sin_addr) < 1){
                printf("no valid address: \n");
                exit(-1);
            }
            // Send the message
            nbytes = sendto(sock_fd, &m, sizeof(struct message), 0,
                (const struct sockaddr *) &player_addr, player_addr_size);
            // Error handling (returns -1 if there is an error)
            if (nbytes < 0)
                printf("Error sending the message to the client \n");
        }
        temp = temp->next;
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
	local_addr.sin_family = AF_INET;         // INET domain (ipv4)
	local_addr.sin_port = htons(SOCK_PORT);  // translates the port to network byte order
	// Allow the program to receive messages sent to any address (available on that computer) 
    local_addr.sin_addr.s_addr = INADDR_ANY; 

    // Assign/bind one address
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
	message m;
    bool game_start = true;
    int n_clients = 0;

	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);


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
                n_clients++;
                printf("connection message detected!\n");
			    // Add client to the list
			    add_client(&client_list, remote_addr_str, remote_port);
                if(n_clients == 1){
                    // The first player joining the server receives the ball
                    // Send play_message to the player
                    send_play_message(sock_fd, remote_addr_str, remote_port);
                    game_start = false; // Set game start
                }
                break;
            case RELEASE:
                printf("release ball message detected!\n");
                    /* Send the ball to the next player;
                    if there is only one player, send the ball to that player */
                    if (n_clients == 1)
                        send_play_message(sock_fd, remote_addr_str, remote_port);
                    else
						if (next_player(&client_list, remote_addr_str, remote_port))
							send_play_message(sock_fd, next_player_address, next_player_port);
                break;
            case MOVE:
                printf("move ball message detected!\n");
                // Send MOVE message
                send_move_message(sock_fd, &client_list, remote_addr_str, remote_port, m.ball_position);
                break;
            case DISCONNECT:
                n_clients--;
                printf("disconnection message detected!\n");     
                printf("deleting player %s with port: %d \n", remote_addr_str, remote_port);
                // Delete client from the client list
		        delete_client(&client_list, remote_addr_str, remote_port);
                /* Sends the ball to the next player, 
                in case there is any player in the client list */
				if (next_player(&client_list, remote_addr_str, remote_port))
							send_play_message(sock_fd, next_player_address, next_player_port);
                break;
        }
		print_list(client_list);
    }
    // Closes the socket and terminates the server
    close(sock_fd);
	exit(0);
}

