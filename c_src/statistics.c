#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "statistics.h"

/*
	Promenne pro statistiku serveru	
*/
int  bytesReceived = 0;
int  bytesSend = 0;
int  messagesReceived = 0;
int  messagesSend = 0;

int  connections = 0;
int  gamesPlayed = 0;

int  badMessages = 0;
int  dobaBehu = 0;

/* velikost hraci plochy */
int boardSize = 16;

/* nazev logovaciho souboru */
char logfile[] = "log.txt";

pthread_mutex_t logZamek;


/* ____________________________________________________________________________

        char *createText(void){

	Alokuje pamet o velikosti 100 bytu a vrati na ni pointer.
   ____________________________________________________________________________
*/
char *createText(void){
	char *pole = (char*)malloc(sizeof(char)*100);
	return pole;
}

/* ____________________________________________________________________________

        char *createTextLong(int velikost){

	Alokuje pamet pozadovane delky a vrati na ni pointer.
   ____________________________________________________________________________
*/
char *createTextLong(int velikost){
	char *pole = (char*)malloc(sizeof(char)*velikost);
	return pole;
}

/* ____________________________________________________________________________

	void loguj(char **log)

	Zaloguje text v parametru do logovaciho souboru.
   ____________________________________________________________________________
*/
void makeLog(char **log){
	if(*log != NULL){
		pthread_mutex_lock(&logZamek);
		FILE *f = fopen(logfile,"a");
		fprintf(f, "%s",*log);
		printf("%s",*log);
		fclose(f);
		pthread_mutex_unlock(&logZamek);
		
		free(*log);
		*log = NULL;
	}
}


