#ifndef HRACI_H
#define HRACI_H


//globalni promena spojoveho seznamu hracu
extern Player *playerList;


void removePlayer(Player *hrac);
void freePlayer(Player *hrac);
Player *newPlayer(struct sockaddr_in clientaddr, char *jmenoHrace);
Player *findPlayer(char* jmeno);
void printListOfPlayers();
void addPlayer(Player *hrac);


#endif
