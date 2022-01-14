// Internet domain sockets libraries
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

// General libraries
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// ncurses library
#include <ncurses.h>

// Include header files
#include "visualization.h"
#include "pong.h" 			
#include "sock_dg_inet.h"


int main(int argc, char *argv[]) {
 
	// Create an internet domain datagram socket (ipv4)
	int sock_fd;
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	// Error handling
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
	message m;

	// Send connection message
	m.command = CONNECT;
	int nbytes;
	nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
			(const struct sockaddr *)&server_addr, sizeof(server_addr));
	// Error handling (returns -1 if there is an error)
    if (nbytes < 0)
        printf("Error sending the message to the server \n");

	// Initialize paddle position and ball position variables
	paddle_position_t paddle;
	ball_position_t ball;
	// Initialize variables to store previous paddle andd ball positions
	paddle_position_t old_paddle;
	ball_position_t old_ball;

	// Curses mode
	initscr();		    	/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
	noecho();			    /* Don't echo() while we do getch */

    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    keypad(my_win, true);
    /* creates a window and draws a border */
    message_win = newwin(5, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);	
	wrefresh(message_win);

	// Create and draw a paddle
    new_paddle(&paddle, PADDLE_SIZE);
    draw_paddle(my_win, &paddle, true);

	// Create and draw a ball
    place_ball_random(&ball);
    draw_ball(my_win, &ball, true);

	bool play_state = false; 	// Initialize play_state boolean
	int key;				 	// Read from keyboard

	/* Clock variables */
	time_t begin; 
	double time_spent;

	/* Clear string */
	char clear[] = "                                          ";		

	while(1){

		// Wait for a message from the server
		nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
		// Error handling
		if (nbytes <= 0)
            printf("Error receiving the message from the server \n");

		// If a Move_ball message is received
		if(m.command == MOVE){
			/* Print new messages to the text display*/
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"WAITING STATE");
			mvwprintw(message_win, 2,1,"%s",clear);
			mvwprintw(message_win, 3,1,"%s",clear);
			/* Update ball position */
			draw_ball(my_win, &ball, false);
			ball = m.ball_position;
            moove_ball(&ball);
            draw_ball(my_win, &ball, true);
			wrefresh(message_win);
		}
		// If a Send_ball message is received
		if(m.command == SEND){
			play_state = true; 		// Entering play_state
			begin = time(NULL);   	// Initialize clock count
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"PLAY STATE");
			mvwprintw(message_win, 2,1,"You can control the paddle: ");
			wrefresh(message_win);
		}

		while(play_state){
			
			/* Read key from keyboard */
			key = wgetch(my_win);

			/* Check time spent and print it */
			time_spent = (double)(time(NULL) - begin); 	// Wallclock time
			mvwprintw(message_win, 1,22,"Time spent: %f",time_spent);
			mvwprintw(message_win, 3,1,"%s",clear);

			/* Check the key pressed */
       		if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){
				/* Store old positions */
				old_paddle = paddle;
				old_ball = ball;
				/* Update paddle */
            	draw_paddle(my_win, &paddle, false);
           		moove_paddle (&paddle, key);
            	draw_paddle(my_win, &paddle, true);
				/* Update ball */
            	draw_ball(my_win, &ball, false);
            	moove_ball(&ball);
            	draw_ball(my_win, &ball, true);
				/* Check if the paddle and the ball collide */
            	draw_ball(my_win, &ball, false);
            	paddle_hit_ball(&ball, &paddle, &old_ball, &old_paddle);
            	draw_ball(my_win, &ball, true);
				/* Redraw the box*/
				box(my_win, 0 , 0);	
				wrefresh(my_win);

				m.command = MOVE;
				m.ball_position = ball;
				nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
				// Error handling (returns -1 if there is an error)
    			if (nbytes < 0)
        			printf("Error sending the message to the server \n");
			}
			else if (key == 'r' && time_spent >= 10){
				play_state = false; 	// go back to waiting state
				/* Send RELEASE message to the server */
				m.command = RELEASE;
				nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
				// Error handling (returns -1 if there is an error)
    			if (nbytes < 0)
        			printf("Error sending the message to the server \n");
				/* Print new messages to the text display*/
				mvwprintw(message_win, 1,1,"%s",clear);
				mvwprintw(message_win, 1,1,"WAITING STATE");
				mvwprintw(message_win, 2,1,"%s",clear);
				mvwprintw(message_win, 3,1,"%s",clear);
			}
			else if(key == 'q'){
				/* Send DISCONNECT message to the server */
				m.command = DISCONNECT;
				nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
				// Error handling (returns -1 if there is an error)
    			if (nbytes < 0)
        			printf("Error sending the message to the server \n");
				// Closes the socket and terminates the client
				close(sock_fd);
				system("clear");
				exit(0);
			
			}
			mvwprintw(message_win, 2,29,"%c key pressed", key);
        	wrefresh(message_win);
		}
	}
	// Closes the socket and terminates the client
	close(sock_fd);
	system("clear");
	exit(0);
}
