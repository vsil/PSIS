
all: server client

CC = gcc
CFLAGS  = -g 

server: pongserver.c client_list.c client_list.h visualization.c visualization.h pong.h sock_dg_inet.h
	$(CC) $(CFLAGS) pongserver.c client_list.c visualization.c -o server -lncurses -lpthread

client: pongclient.c client_list.c client_list.h visualization.c visualization.h pong.h sock_dg_inet.h
	$(CC) $(CFLAGS) pongclient.c client_list.c visualization.c -o client -lncurses -lpthread

clean:
	rm server client

