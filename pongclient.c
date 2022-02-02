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
#include <time.h>

// ncurses library
#include <ncurses.h>

// Include header files
#include "visualization.h"
#include "pong.h" 			
#include "sock_dg_inet.h"

// Global variables
WINDOW * my_win;
int sock_fd;
struct sockaddr_in server_addr;
ball_position_t ball;
ball_position_t old_ball;
paddle_position_t paddle;
paddle_position_t old_paddle;
message m;
bool play_state;
pthread_mutex_t lock;

int h=0;

void* ball_thread(void* arg){
	int nbytes;
	while(1){
		sleep(3);
		/* Update ball position */
		old_ball = ball;
		draw_ball(my_win, &ball, false);
        moove_ball(&ball);
        // draw_ball(my_win, &ball, true);
		// /* Check if the paddle and the ball collide */
        // draw_ball(my_win, &ball, false);
        // paddle_hit_ball(&ball, &paddle, &old_ball, &old_paddle);
        draw_ball(my_win, &ball, true);
		/* Redraw the box*/
		// box(my_win, 0 , 0);	
		// wrefresh(message_win);
		pthread_mutex_lock(&lock);
		m.command = MOVE;
		m.ball_position = ball;
		mvwprintw(message_win, 3,1, "send ball position: (%d, %d)", m.ball_position.x, m.ball_position.y);
		nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
		mvwprintw(message_win, 4,1, "send ball position: (%d, %d)", m.ball_position.x, m.ball_position.y);
		if (nbytes < 0)
        		printf("Error sending Move_ball message to the server \n");
		pthread_mutex_unlock(&lock);
		box(my_win, 0 , 0);	
		wrefresh(message_win);
	}
}

void* keyboard_thread(void* arg){
	int key;
	int nbytes;
	while(1){
		/* Read key from keyboard */
		key = wgetch(my_win);
		if (key == 'q'){
			pthread_mutex_lock(&lock);
			/* Send DISCONNECT message to the server */
			m.command = DISCONNECT;
			nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
			// Error handling (returns -1 if there is an error)
    		if (nbytes < 0)
        		printf("Error sending the message to the server \n");
			pthread_mutex_unlock(&lock);	
			// Closes the socket and terminates the client
			pthread_mutex_destroy(&lock);
			close(sock_fd);
			system("clear");
			exit(0);

		}
		/* Check the key pressed */
       	if ((key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN) && play_state){
			/* Store old positions */
			old_paddle = paddle;
			old_ball = ball;
			/* Update paddle */
            draw_paddle(my_win, &paddle, false);
           	moove_paddle (&paddle, key);
            draw_paddle(my_win, &paddle, true);
			/* Check if the paddle and the ball collide */
			draw_ball(my_win, &ball, false);
			paddle_hit_ball(&ball, &paddle, &old_ball, &old_paddle);
			draw_ball(my_win, &ball, true);
			pthread_mutex_lock(&lock);
			m.command = MOVE;
			m.ball_position = ball;
			nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
			if (nbytes < 0)
        		printf("Error sending Move_ball message to the server \n");
			pthread_mutex_unlock(&lock);
		}
		mvwprintw(message_win, 2,29,"%c key pressed", key);
		/* Redraw the box*/
		box(my_win, 0 , 0);	
        wrefresh(message_win);
	}
}

int main(int argc, char *argv[]) {
 
	// Create an internet domain datagram socket (ipv4)
	// int sock_fd;
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
	// struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SOCK_PORT);
	if( inet_pton(AF_INET, buff, &server_addr.sin_addr) < 1){
		printf("No valid address \n");
		exit(-1);
	}

	// Send connection message
	m.command = CONNECT;
	int nbytes;
	nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
			(const struct sockaddr *)&server_addr, sizeof(server_addr));
	// Error handling (returns -1 if there is an error)
    if (nbytes < 0)
        printf("Error sending the message to the server \n");

	// Curses mode
	initscr();		    	/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
	noecho();			    /* Don't echo() while we do getch */

    /* creates a window and draws a border */
    // WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
	my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
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

	play_state = false; 		// Initialize play_state boolean

	// /* Clock variables */
	// time_t begin; 
	// double time_spent;

	/* Clear string */
	char clear[] = "                                          ";

	if (pthread_mutex_init (&lock, NULL) != 0){
		printf("\n Mutex init has failed \n");
	}
		
	pthread_t thread_keyboard_id;
	pthread_create(&thread_keyboard_id, NULL, keyboard_thread, NULL);

	command_t recv_commd;

	pthread_t thread_ball_id;

	while(1){

		// Wait for a message from the server
		nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
		pthread_mutex_lock(&lock);
		// Error handling
		if (nbytes == 0)
            printf("000000000000000000000000000SSIJDENWJENWIJDVH \n");
		if (nbytes < 0)
            printf("Error receiving the message from the server \n");
		recv_commd = m.command;	
		pthread_mutex_unlock(&lock);

		// If a Move_ball message is received
		if(recv_commd == MOVE){
			/* Print new messages to the text display*/
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"WAITING STATE");
			mvwprintw(message_win, 2,1,"%s",clear);
			mvwprintw(message_win, 3,1,"%s",clear);
			/* Update ball position */
			draw_ball(my_win, &ball, false);
			ball = m.ball_position;
            // moove_ball(&ball);
            draw_ball(my_win, &ball, true);
			wrefresh(message_win);
		}
		// If a Send_ball message is received
		if(recv_commd == SEND){
			play_state = true; 		// Entering play_state
			
			if (h == 0){			
				pthread_create(&thread_ball_id, NULL, ball_thread, NULL);  // creates ball thread only on the first SEND message received
			}
			h++;
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"PLAY STATE");
			mvwprintw(message_win, 2,1,"You can control the paddle: ");
			mvwprintw(message_win, 4,1, "%d", h);
			wrefresh(message_win);
		}

		// If a Release message is received
		if(recv_commd == RELEASE){
			play_state = false; 		// entering waiting_state
			h=0;
			pthread_cancel(thread_ball_id);
			mvwprintw(message_win, 1,1,"%s",clear);
			mvwprintw(message_win, 1,1,"WAITING STATE");
			mvwprintw(message_win, 2,1,clear);
			mvwprintw(message_win, 4,1, "%d", h);
			wrefresh(message_win);
		}
	}
	// Closes the socket and terminates the client
	close(sock_fd);
	system("clear");
	exit(0);
}
