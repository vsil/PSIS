// Internet domain sockets libraries
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

// General libraries
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Include header files
#include "client_list.h" 			// visualization already included here
#include "sock_dg_inet.h"


int main(int argc, char *argv[]) {
 
	// Create an internet domain datagram socket
	int sock_fd;
	sock_fd= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	char buff[4]; // an IPv4 adress is composed of 4 bytes
	// Receive the adress as a command line argument
	if (argc != 2){
		printf("Please, enter a valid argument \n");
		exit(-1);
	}
	else
		strcpy(buff, argv[1]);

	/* Server adress */
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SOCK_PORT);
	if( inet_pton(AF_INET, buff, &server_addr.sin_addr) < 1){
		printf("No valid address \n");
		exit(-1);
	}

	// Initialize message
	struct message m;

	// send connection message
	m.command = CONNECT;

	sendto(sock_fd, &m, sizeof(struct message), 0, 
			(const struct sockaddr *)&server_addr, sizeof(server_addr));

	int nbytes;
	// receive Board_update message with paddles and ball positions
	nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
	
	printf("\n received %d bytes", nbytes);

	// Initialize paddle position and ball position variables
	paddle_position_t* paddle;
	ball_position_t* ball;
	ball = m.ball_position;


	printf("\n begin \n");
	printf("\n command: %d", m.command);
    //printf("\n ball x: %d", ball->x);
    //printf("\n ball x: %d", m.ball_position->x);
    printf("\n client list port: %d", m.client_list->port);
    printf("\n paddle x: %d", m.client_list->paddle->x);
    

	// Create and draw a paddle
	draw_all_paddles(m.client_list, true);

	// Draw the ball ball
    //draw_ball(my_win, &ball, true);

	bool play_state = false; // Initialize play_state boolean
	int key;				 // Read from keyboard

	/* Clear string */
	char clear[] = "                                          ";		

	while(1){

		key = getchar();

		/* Check the key pressed */
		if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){

			/* Send Paddle_move message to the server */
			m.command = PADDLE_MOVE;
			
			//m.ball_position = ball;
			
			sendto(sock_fd, &m, sizeof(struct message), 0, 
				(const struct sockaddr *)&server_addr, sizeof(server_addr));
		}

		else if(key == 'q'){
			m.command = DISCONNECT;
			sendto(sock_fd, &m, sizeof(struct message), 0, 
				(const struct sockaddr *)&server_addr, sizeof(server_addr));
			close(sock_fd);
			system("clear");
			exit(0);
		}

				
		// Wait for a message
		recv(sock_fd, &m, sizeof(struct message), 0);

		// If a Board_update message is received
		if(m.command == BOARD_UPDATE){
			/*Print ball and paddles */
			ball = m.ball_position;
			
			/*
            draw_ball(my_win, &ball, false);
            moove_ball(&ball);
            draw_ball(my_win, &ball, true);
			wrefresh(message_win);
			*/
		}
	}
	close(sock_fd);
	system("clear");
	exit(0);
}
