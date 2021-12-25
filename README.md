# PSIS

implementação das seguintes funcionalides:
- lista de clientes (linked list; adiciona o ip e o port de cada cliente novo, remove o cliente da lista quando este preme a letra 'd'); a lista será usada no controlo do flow do jogo (que player receve a info da bola, qual o next player, etc...)

"pong.h" : defini a 'struct message' que sera a estrutura atraves da qual as mensagens serao trocadas entre server-cliente
NOTA/QUESTAO: devido à forma como o "recvfrom" funciona (é preciso especificar o nr de bytes da mensagem), acho que devemos usar sempre a mesma struct na troca de mensagens


compilation and execution: 
- gcc pongserver.c -o server
- gcc pongclient.c -o client
 
 ./server
 ./client
