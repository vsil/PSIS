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
#include "client_list.h" 			// visualization.h already included here
#include "sock_dg_inet.h"

// receives all the player paddles and scores from the server, adds them to local client paddle_list
void update_player_positions(int sock_fd, struct Paddle_Node** paddle_list, int n_clients)
{
	paddle_position_message m_paddle;
	paddle_position_t paddle;
	int player_score;
	bool current_player;
	int nbytes;

	reset_list(paddle_list);			// frees memory from the list used in the previous round

	for(int i=0; i<n_clients; i++){		// receives one paddle_position_message per client
		nbytes = recv(sock_fd, &m_paddle, sizeof(struct paddle_position_message), 0);
		if (nbytes <= 0)
            printf("Error receiving the message from the server \n");
		paddle = m_paddle.paddle_position;
		player_score = m_paddle.player_score;
		current_player = m_paddle.current_player;

		add_player_list(paddle_list, paddle, player_score, current_player);	  // adds new information to paddle_list
	}
}

// prints players scores on the message_window
void print_players_score(WINDOW * message_win, struct Paddle_Node* client_list, int n_clients){
	
	int i=1;
	message_win = newwin(n_clients+2, WINDOW_SIZE+10, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);

	while(client_list!=NULL){		// goes through client_list and prints players scores
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
	int nbytes;

	nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
				(const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (nbytes < 0)
            printf("Error sending the message to the server \n"); 

	int n_clients;
	ball_position_t ball;
	struct Paddle_Node* Paddle_List = NULL; 

	// receives general message
	nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
	if (nbytes <= 0)
            printf("Error receiving the message from the server \n");
	n_clients = m.number_clients;

	// the server has reached full capacity, shows client waiting message
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
		if (nbytes <= 0)
            printf("Error receiving the message from the server \n");
	}
	
	// receives board_update message
	if(m.command==BOARD_UPDATE){
		ball = m.ball_position;
		// receives paddle_position_message and updates local paddle_list with player information
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

	// Draw the paddles
	draw_all_paddles(my_win, Paddle_List, true);

	// Draw the ball 
    draw_ball(my_win, &ball, true);

	// Prints players scores
	print_players_score(message_win, Paddle_List, n_clients);

	int key;				// Read from keyboard
	char clear[] = "                                          ";	// Clear string 	

	while(1){

		/* gets user input */
		while(!(key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN || key == 'q')){
			key = wgetch(my_win);	
		}

		/* Check the key pressed */
		if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){

			/* Send Paddle_move message to the server */
			m.command = PADDLE_MOVE;
			m.pressed_key = key;
			nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
				(const struct sockaddr *)&server_addr, sizeof(server_addr));
			if (nbytes < 0)
            	printf("Error sending the message to the server \n"); 
		}

		else if(key == 'q'){
			/* Send disconnect message to the server, closes socket and exits application */
			m.command = DISCONNECT;
			nbytes = sendto(sock_fd, &m, sizeof(struct message), 0, 
				(const struct sockaddr *)&server_addr, sizeof(server_addr));
			if (nbytes < 0)
            	printf("Error sending the message to the server \n"); 
			close(sock_fd);
			system("clear");
			exit(0);
		}

		key = 0; // resets user key
				
		// Waits for a message
		nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
		if (nbytes <= 0)
            printf("Error receiving the message from the server \n");

		// If a Board_update message is received
		if(m.command == BOARD_UPDATE){

			draw_ball(my_win, &ball, false);	
			draw_all_paddles(my_win, Paddle_List, false);	// clears previous round paddles and ball

			ball = m.ball_position;							// updates ball information
			n_clients = m.number_clients;

			update_player_positions(sock_fd, &Paddle_List, n_clients);	// receives new paddle positions, updates local Paddle_List
            draw_ball(my_win, &ball, true);			
			draw_all_paddles(my_win, Paddle_List, true);				// draws new ball and paddle positions
			print_players_score(message_win, Paddle_List, n_clients);	// prints players score on the message window

			/* Redraw the box*/
			box(my_win, 0 , 0);	
			wrefresh(my_win);
		}
	}
	close(sock_fd);
	system("clear");
	exit(0);
}
