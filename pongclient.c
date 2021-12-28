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
	//printf(" socket created \n Ready to send\n");

	char recv_message[100];

	char linha[100] = "127.0.0.1";  // this should be given as argument in terminal
/*
	printf("What is the network address of the recipient? ");
	fgets(linha, 100, stdin);
	linha[strlen(linha)-1] = '\0';
*/

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
 
	nbytes = recv(sock_fd, recv_message, 100, 0);
	printf("\n Received connection confirmation: '%s'", recv_message);

	while(1){
		

		nbytes = recv(sock_fd, &m, sizeof(struct message), 0);
			printf("\nmessage received!");
			if(m.msg_type=='s'){
				printf("\nEntering play state! \n");

				char ch;
				do{
					printf("what is your character(a..z)?: ");
					ch = getchar();
					ch = tolower(ch);  
				}while(!isalpha(ch));

				if(ch=='d'){
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
