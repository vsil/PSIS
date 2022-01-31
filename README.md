# PSIS

implementação das seguintes funcionalides:
- lista de clientes (linked list; adiciona o ip e o port de cada cliente novo, remove o cliente da lista quando este preme a letra 'd'); a lista será usada no controlo do flow do jogo (que player receve a info da bola, qual o next player, etc...)

"pong.h" : defini a 'struct message' que sera a estrutura atraves da qual as mensagens serao trocadas entre server-cliente
NOTA/QUESTAO: devido à forma como o "recvfrom" funciona (é preciso especificar o nr de bytes da mensagem), acho que devemos usar sempre a mesma struct na troca de mensagens

[updates]: 
- added graphics 
- added 10 second condition
- added command line input
- added function paddle_hit_ball (has to be improved)
- added full duplex communication
- added move_ball messages
- created a makefile

[to_do]:
- remove extra balls that appear on the screen
- improve padde_hit_ball function
- add more comments to the code and refactor in some parts
- double check if the command line IP input is working fine

[to_compile_and_execute]:
- run "make"
- ./server (execute server)
- ./client 127.0.0.1 (execute client)
