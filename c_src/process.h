#ifndef ZPRACOVANI_H
#define ZPRACOVANI_H

extern int pocetCastiZpravy;


void disconnectFromServer(Player *hrac);
int index_druheho(int indexHrace);
void processMove(char **zprava, Player *hrac);
void giveUp(Player *hrac);
void cancelGame(Player *hrac);
void gameOver(Game *partie, int druhKonce);
void processMessage(char **zprava, Player *hrac);
char **parseMessage(char *zprava);
void startGame(Game *partie);
void connectPlayerToGame(char *idString, Player *hrac);
void sendListOfOpensGames(Player *hrac);
void zaloz_partii(Player *hrac);
int getSequenceNumber(char *zprava);
char **parseMessageForAck(char *zprava);

#endif
