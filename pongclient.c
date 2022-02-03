// Internet domain sockets libraries
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

// Include threads library
#include <pthread.h> 

// General libraries
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

// ncurses library
#include <ncurses.h>

// Include header files
#include "visualization.h"
#include "pong.h" 			
#include "sock_dg_inet.h"


/* Global variables */		
int sock_fd;					 // socket file descriptor	
struct sockaddr_in server_addr;  // server adress
// ball variables 
ball_position_t ball;
ball_position_t old_ball;
// paddle variables 
paddle_position_t paddle;
paddle_position_t old_paddle;
// mutex declaration
pthread_mutex_t mux_curses; 
// play state boolean: true - play state / false - Idle state
bool play_state;	


void* ball_thread(){
	message m;
	int nbytes;
	while(1){
		/* Wait 1 second */
		sleep(1);
		/* Update ball position */
		old_ball = ball;
		pthread_mutex_lock(&mux_curses);  
		draw_ball(&ball, false);
        moove_ball(&ball);
        paddle_hit_ball(&ball, &paddle, &old_ball, &old_paddle);
        draw_ball(&ball, true);
		box(play_win, 0 , 0);	
		wrefresh(play_win);
		pthread_mutex_unlock(&mux_curses);  

		/* Create the Move_ball message */
		m.command = MOVE;
		m.ball_position = ball;
		/* Send the Move_ball message to the server */
		nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
		// Error handling
		if (nbytes < 0)
        	printf("Error sending Move_ball message to the server \n");
	}
}

void* recv_msgs_thread(){
	// Wait for a message from the server
	message m;
	int nbytes;
	pthread_t thread_ball_id;
	/* Clear string */
	char clear[] = "                                          ";
	while(1){
		nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
		// Error handling
		if (nbytes <= 0){
			printf("Error receiving the message from the server \n");
			exit(0);
		}
		// If a Move_ball message is received
		if(m.command == MOVE && !play_state){
			pthread_mutex_lock(&mux_curses);
			/* Print new messages to the text display*/
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"Idle state");
			mvwprintw(message_win, 2,1,"%s",clear);
			mvwprintw(message_win, 3,1,"%s",clear);
			/* Update ball position */
			draw_ball(&ball, false);
			ball = m.ball_position;
			// moove_ball(&ball);
			draw_ball(&ball, true);
			wrefresh(message_win);
			pthread_mutex_unlock(&mux_curses);
		}
		// If a Send_ball message is received
		if(m.command == SEND){
			// Prints
			pthread_mutex_lock(&mux_curses);
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"Play state");
			mvwprintw(message_win, 2,1,"You can control the paddle: ");
			mvwprintw(message_win, 3,1,"%s",clear);
			wrefresh(message_win);
			pthread_mutex_unlock(&mux_curses);

			play_state = true; 		// Entering play_state
			// Creates ball thread only on the first SEND message received
			if (pthread_create(&thread_ball_id, NULL, ball_thread, NULL) != 0)
				printf("Error creating the ball thread \n");
		}

		// If a Release message is received
		if(m.command == RELEASE){
			play_state = false; 	// Entering Idle state
			// Cancel the ball thread - the ball thread is then created in a different client
			if (pthread_cancel(thread_ball_id) != 0)
				printf("Error cancelling the ball thread \n");
			// Prints
			pthread_mutex_lock(&mux_curses);
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"Idle State");
			mvwprintw(message_win, 2,1,clear);
			wrefresh(message_win);
			pthread_mutex_unlock(&mux_curses);
		}
	}

}

int main(int argc, char *argv[]) {
 
	// Create an internet domain datagram socket (ipv4)
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
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SOCK_PORT);
	if( inet_pton(AF_INET, buff, &server_addr.sin_addr) < 1){
		printf("No valid address \n");
		exit(-1);
	}

	// Send connection message
	message m;
	m.command = CONNECT;
	int nbytes;
	nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
			(const struct sockaddr *)&server_addr, sizeof(server_addr));
	// Error handling (returns -1 if there is an error)
    if (nbytes < 0)
        printf("Error sending the message to the server \n");

	initscr();		    	/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
	noecho();			    /* Don't echo() while we do getch */

    /* creates a window and draws a border */
	play_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(play_win, 0 , 0);	
	wrefresh(play_win);
    keypad(play_win, true);
    /* creates a window and draws a border */
    message_win = newwin(5, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);	
	wrefresh(message_win);

	/* Creates and draws a paddle */
    new_paddle(&paddle, PADDLE_SIZE);
    draw_paddle(&paddle, true);

	/* Creates and draws a ball */
    place_ball_random(&ball);
    draw_ball(&ball, true);

	play_state = false; 		// Initialize play_state boolean

	/* Mutex initialization */
	if (pthread_mutex_init (&mux_curses, NULL) != 0){
		printf("\n Mutex init has failed \n");
	}

	/* Creates the thread that receives msgs from the server*/	
	pthread_t thread_recv_msgs_id;
	if (pthread_create(&thread_recv_msgs_id, NULL, recv_msgs_thread, NULL) != 0)
		printf("Error creating the receive messages thread \n");

	int key; // Variable that stores the character read from the keyboard

	while(1){
		/* Read key from keyboard */
		key = wgetch(play_win);
		if (key == 'q'){
			/* Send DISCONNECT message to the server */
			m.command = DISCONNECT;
			nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
			// Error handling (returns -1 if there is an error)
    		if (nbytes < 0)
        		printf("Error sending the message to the server \n");	
			// Exits the client (automatically destroys the threads)
			system("clear");
			exit(0);
		}
		/* Check the key pressed */
       	if ((key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN) && play_state){
			/* Store old positions */
			old_paddle = paddle;
			old_ball = ball;
			pthread_mutex_lock(&mux_curses);  
			/* Update paddle */
            draw_paddle(&paddle, false);
           	moove_paddle (&paddle, key);
            draw_paddle(&paddle, true);
			/* Check if the paddle and the ball collide */
			draw_ball(&ball, false);
			paddle_hit_ball(&ball, &paddle, &old_ball, &old_paddle);
			draw_ball(&ball, true);
			pthread_mutex_unlock(&mux_curses);  

			/* Create the Move_ball message */
			m.command = MOVE;
			m.ball_position = ball;
			/* Send the Move_ball message to the server */
			nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
						(const struct sockaddr *)&server_addr, sizeof(server_addr));
			if (nbytes < 0)
        		printf("Error sending Move_ball message to the server \n");
		}
		pthread_mutex_lock(&mux_curses);  
		mvwprintw(message_win, 2,29,"%c key pressed", key);
		/* Redraw the box*/
		box(play_win, 0 , 0);	
        wrefresh(play_win);
		pthread_mutex_unlock(&mux_curses);  
	}
}
