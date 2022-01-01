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
#include "visualization.h"
#include "pong.h"
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

	// Initialize paddle position and ball position variables
	paddle_position_t paddle;
	ball_position_t ball;

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

	bool play_state = false; // Initialize play_state boolean
	int key;				 // Read from keyboard

	/* Clock variables */
	time_t begin; 
	double time_spent;			

	while(1){

		// Wait for a message
		recv(sock_fd, &m, sizeof(struct message), 0);

		if(m.command == SEND){
			play_state = true; 		// Entering play_state
			begin = time(NULL);   	// Initialize clock count
		}

		while(play_state){
			
			mvwprintw(message_win, 1,1,"PLAY STATE");
			mvwprintw(message_win, 2,1,"Use arrow keys to control the paddle");
			key = wgetch(my_win);

			/* Check time spent and print it */
			time_spent = (double)(time(NULL) - begin);
			mvwprintw(message_win, 3,23,"Time spent: %f",time_spent);

			/* Check the key pressed */
       		if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){
            	draw_paddle(my_win, &paddle, false);
           		moove_paddle (&paddle, key);
            	draw_paddle(my_win, &paddle, true);

            	draw_ball(my_win, &ball, false);
				paddle_hit_ball(&ball, &paddle);
            	moove_ball(&ball);
				paddle_hit_ball(&ball, &paddle);
            	draw_ball(my_win, &ball, true);

				// draw_ball(my_win, &ball, false);
				// draw_paddle(my_win, &paddle, true);
				// paddle_hit_ball(&ball, &paddle);
				// draw_ball(my_win, &ball, true);

				// m.command = MOVE;
				// m.ball_position = ball;
				// sendto(sock_fd, &m, sizeof(struct message), 0, 
				// 	(const struct sockaddr *)&server_addr, sizeof(server_addr));
        	}
			else if (key == 'r' && time_spent >= 10){
				play_state = false; 	// go back to waiting state
				m.command = RELEASE;
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
			
			mvwprintw(message_win, 3,1,"%c key pressed", key);
        	wrefresh(message_win);
		}
	}
	close(sock_fd);
	system("clear");
	exit(0);
}
