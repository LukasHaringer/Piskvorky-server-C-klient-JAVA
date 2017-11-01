#ifndef MESSAGE_H
#define MESSAGE_H

//globalni promena spojoveho seznamu hracu
extern Message *messagesList;
extern int KnownAdressesCount;
extern int serverSocket;
extern struct sockaddr_in knownAdresses[100];


int confirmMessage(struct sockaddr_in clientaddr);
void freeMessage(Message *message);
void removeMessage(Message *message);
Message *findMessage(struct sockaddr_in clientaddr);
void addMessage(Message *message);
Message *newMessage(char buf[1024], struct sockaddr_in clientaddr, int clientlen);
void printListOfMessages();
#endif

