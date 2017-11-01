#ifndef PARTIE_H
#define PARTIE_H

extern Game *gameList;


void lockGames();
void unlockGames();

char *createGamesList();
int newGameID();
void removeGame(Game *partie);
void freeGame(Game **partie);
Game *newGame(Player *hrac);
Game *findGame(int idPartie);
void printListOfGames();
void addGame(Game *partie);


#endif 
