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


// receives player paddles and scores from the server, adds them to Paddle_List
void update_player_positions(int sock_fd, struct Paddle_Node** paddle_list, int n_clients)
{
	paddle_position_message m_paddle;
	paddle_position_t paddle;
	int player_score;
	bool current_player;

	reset_list(paddle_list);

	for(int i=0; i<n_clients; i++){
		recv(sock_fd, &m_paddle, sizeof(struct paddle_position_message), 0);
		paddle = m_paddle.paddle_position;
		player_score = m_paddle.player_score;
		current_player = m_paddle.current_player;

		//printf("i: %d  x = %d", i, paddle.x);
		add_player_list(paddle_list, paddle, player_score, current_player);	
	}
}

void print_players_score(WINDOW * message_win, struct Paddle_Node* client_list, int n_clients){
	int i=1;

	message_win = newwin(n_clients+2, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);


	while(client_list!=NULL){
		if(client_list->current_player){
			mvwprintw(message_win, i,1,"P%d - %d <---", i, client_list->player_score);
		}
		else{
			mvwprintw(message_win, i,1,"P%d - %d", i, client_list->player_score);
		}
		i++;
		client_list = client_list->next;
	}
	wrefresh(message_win);
}

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

	// send connection message
	struct message m;
	m.command = CONNECT;

	sendto(sock_fd, &m, sizeof(struct message), 0, 
			(const struct sockaddr *)&server_addr, sizeof(server_addr));



	int nbytes;
	int n_clients;
	ball_position_t ball;
	struct Paddle_Node* Paddle_List = NULL; 

	// receives general message
	nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
	n_clients = m.number_clients;

	if (m.command==WAIT_LIST)
	{
		printf("\n Max capacity of the server has been reached (%d players)\n", MAX_CLIENTS);
		if (n_clients==0)
		{
			printf("You are the first player in the waiting list\n");
		}
		else{
			printf("There are currently %d players waiting in front of you\n", n_clients);
		}
		nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
	}
	
	if(m.command==BOARD_UPDATE){
		ball = m.ball_position;

		// receives paddle_position_message and updates Paddle_List with player information
		update_player_positions(sock_fd, &Paddle_List, n_clients);	 
	}

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

	// Draw the paddles
	draw_all_paddles(my_win, Paddle_List, true);

	// Draw the ball 
    draw_ball(my_win, &ball, true);


	int key;				 // Read from keyboard

	/* Clear string */
	char clear[] = "                                          ";		

	while(1){

		while(!(key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN || key == 'q')){
			key = wgetch(my_win);	
		}


		/* Check the key pressed */
		if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){

			/* Send Paddle_move message to the server */
			m.command = PADDLE_MOVE;
			m.pressed_key = key;

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

		key = 0;
				
		// Wait for a message
		recv(sock_fd, &m, sizeof(struct message), 0);

		// If a Board_update message is received
		if(m.command == BOARD_UPDATE){

			draw_ball(my_win, &ball, false);	
			draw_all_paddles(my_win, Paddle_List, false);					// not working ????

			ball = m.ball_position;
			n_clients = m.number_clients;

			update_player_positions(sock_fd, &Paddle_List, n_clients);	 
            draw_ball(my_win, &ball, true);			
			draw_all_paddles(my_win, Paddle_List, true);
			print_players_score(message_win, Paddle_List, n_clients);	
		}
	}
	close(sock_fd);
	system("clear");
	exit(0);
}
