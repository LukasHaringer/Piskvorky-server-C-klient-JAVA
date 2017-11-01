#ifndef SERVER_H
#define SERVER_H

extern char separator[];

void *clientThread(void *novyHrac);
void *connectThread(struct sockaddr_in *clientaddr);
int sendMessage(char **zprava, Player *hrac);
void *receiveThread(void * port);
int check(char *zprava);
int sendReconnect(Player *hrac);
void sendAck(Player *hrac);
void sendAckAgain(Player *hrac);


#endif
