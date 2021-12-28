#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>


#include<ctype.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <ncurses.h>
#include "pong.h"
#include "sock_dg_inet.h"

WINDOW * message_win;

int main(){

	char buff[100]; 
	int nbytes;

	int sock_fd= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	char recv_message[100];

	char linha[100] = "127.0.0.1";  // this should be given as argument in terminal

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SOCK_PORT);
	if( inet_pton(AF_INET, linha, &server_addr.sin_addr) < 1){
		printf("no valid address: \n");
		exit(-1);
	}

	// send connection message
	struct message m;
	m.msg_type = 'c';
	sendto(sock_fd, &m, sizeof(struct message), 0, 
			(const struct sockaddr *)&server_addr, sizeof(server_addr));

	int play_state = 0;

	while(1){
		system("clear");
		printf("\n waiting for message \n");
		
		nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
			printf("\nmessage received! \n");

		if(m.msg_type=='s'){
			printf("\nEntering play state! \n");
			play_state = 1;
		}

		while(play_state==1){
			char ch;

			do{
				printf("what is your character?: 'r'-release 'q'-disconnect ");
				ch = getchar();
				fflush(stdin);
				ch = tolower(ch);  
			}while(!isalpha(ch));

			if(ch=='r'){
				// implement aditional condition to check if 10 seconds have passed in play state
				printf("Releasing ball ...");

				play_state = 0; //go back to waiting state
				m.msg_type = 'r';
				sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
			}

			else if(ch=='q'){
				printf("Disconnecting ...");
				m.msg_type = 'd';
				sendto(sock_fd, &m, sizeof(struct message), 0, 
					(const struct sockaddr *)&server_addr, sizeof(server_addr));
				printf("Sent disconnection message");
				close(sock_fd);
				exit(0);
			}
		}
	}
	close(sock_fd);
	exit(0);
}
