CC=gcc
all: uklid objekt server

objekt:
	${CC} -c main.c statistics.c player.c message.c process.c game.c winCheck.c

server: main.c
	${CC} main.c statistics.c player.c message.c process.c game.c winCheck.c -o server -g -lpthread -Wall
	rm -f *.o
uklid: 
	rm -f server


	
