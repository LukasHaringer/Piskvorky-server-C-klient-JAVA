#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#include "structures.h"
#include "player.h"
#include "statistics.h"


//spojovy seznam hracu
Player *playerList = NULL;
pthread_mutex_t seznamZamek;

/* ____________________________________________________________________________

        void freePlayer(Player *hrac) {

        Uvolni pamet zabranou strukturou hrace.
   ____________________________________________________________________________
 */
void freePlayer(Player *hrac) {
    Player *pom;
    pom = hrac;
    //	free(pom->nick);
    free(pom);
    pom = NULL;
}

/* ____________________________________________________________________________

        void removePlayer(Player *hrac) {

        Vyjme daneho hrace ze spojoveho seznamu.
   ____________________________________________________________________________
 */
void removePlayer(Player *hrac) {

    pthread_mutex_lock(&seznamZamek);

    Player *akt = playerList;
    Player *predchozi = NULL;

    while (akt != NULL) {

        if (hrac == playerList) {

            //hledany hrac je na zacatku seznamu

            if (akt->dalsiHrac != NULL) {
                playerList = playerList->dalsiHrac;
            } else {
                playerList = NULL;
            }
            break;
        } else {
            if (akt == hrac) {
                predchozi->dalsiHrac = akt->dalsiHrac;
                printf("Hrac: \"%s\" vyjmut ze seznamu\n", akt->nick);
                break;
            } else {
                predchozi = akt;
                akt = akt->dalsiHrac;
            }
        }
    }

    pthread_mutex_unlock(&seznamZamek);


}

/* ____________________________________________________________________________

        void vypis_seznam_hracu()

        Vypise seznam vsech hracu.
   ____________________________________________________________________________
 */
void printListOfPlayers() {
    pthread_mutex_lock(&seznamZamek);

    if (playerList != NULL) {
        printf("\n*** SEZNAM HRACU ***\n");
        Player *pom = playerList;

        while (pom != NULL) {
            printf("Jmeno: %10s  \t port: %d \n", pom->nick, pom->adresa.sin_port);
            pom = pom->dalsiHrac;
        }
        printf("********************\n\n");

    } else {
        printf("Seznam hracu je prazdny!\n");
    }



    pthread_mutex_unlock(&seznamZamek);
}

/* ____________________________________________________________________________

        Player *findPlayer(char* jmeno) {

        Podle socketu najde hrace a vrati na nej pointer.
   ____________________________________________________________________________
 */
Player *findPlayer(char* jmeno) {

    pthread_mutex_lock(&seznamZamek);

    //pokud neni seznam prazdny prohleda ho
    if (playerList != NULL) {

        Player *pom = playerList;
        while (pom != NULL) {

            if (strcmp(jmeno, pom->nick) == 0) {
                pthread_mutex_unlock(&seznamZamek);
                return pom;
            } else {
                pom = pom->dalsiHrac;
            }
        }
    }

    pthread_mutex_unlock(&seznamZamek);
    return NULL;
}

/* ____________________________________________________________________________

        void pridej_hrace(Hrac *hrac)

        Prida daneho hrace do spojoveho seznamu.
   ____________________________________________________________________________
 */
void addPlayer(Player *hrac) {
    if (hrac) {
        pthread_mutex_lock(&seznamZamek);
        if (playerList == NULL) {
            //pokud je seznam prazdny
            playerList = hrac;
        } else {
            // prida hrace na konec seznamu
            Player *pom = playerList;
            while (pom->dalsiHrac != NULL) {
                pom = pom->dalsiHrac;
            }
            pom->dalsiHrac = hrac;
        }
        pthread_mutex_unlock(&seznamZamek);
    }


}

/* ____________________________________________________________________________

        Hrac *novy_hrac(char clientSocket, char *jmenoHrace)

        Alokuje pamet pro noveho hrace a naplni strukturu daty z parametru funkce.
   ____________________________________________________________________________
 */
Player *newPlayer(struct sockaddr_in clientaddr, char *jmenoHrace) {

    Player *hrac = (Player*) malloc(sizeof (Player));

    hrac->adresa = clientaddr;
    hrac->nick = createText();
    strcpy(hrac->nick, jmenoHrace);

    hrac->packetNumberSent = 1;
    hrac->packetNumberRecv = 1;
    hrac->stav = 0;
    hrac->aktualizace = 0;
    hrac->odpojen = 0;
    hrac->blocked = 0;
    hrac->dalsiHrac = NULL;
    hrac->partie = NULL;
    hrac->indexHrace = -1;
    return hrac;
}













