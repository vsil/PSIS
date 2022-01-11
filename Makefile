
all: server client

CC = gcc
CFLAGS  = -g

server: pongserver.c client_list.c client_list.h pong.h sock_dg_inet.h
	$(CC) $(CFLAGS) pongserver.c client_list.c -o server

client: pongclient.c visualization.c visualization.h pong.h sock_dg_inet.h
	$(CC) $(CFLAGS) pongclient.c visualization.c -o client -lncurses

clean:
	rm server client

