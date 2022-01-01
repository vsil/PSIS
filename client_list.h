#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "pong.h"
#include <stdbool.h>

// A linked list node
struct Node {
    char address[100];
	int port;
    struct Node* next;
};

// Next player
int next_player_port;
char next_player_address[100];
bool is_next_player;

// Funcitons
void add_client(struct Node** head_ref, char new_address[], int new_port);
void delete_client(struct Node** head_ref, char delete_address[], int delete_port);
void print_list(struct Node* node);
bool next_player(struct Node** head_ref, char player_address[], int player_port);