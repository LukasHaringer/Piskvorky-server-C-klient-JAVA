#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "structures.h"
#include "game.h"
#include "statistics.h"
#include "main.h"

Game *gameList = NULL;
pthread_mutex_t seznamZamek;
pthread_mutex_t idZamek;
int gameID = 1;

void lockGames() {
    pthread_mutex_lock(&seznamZamek);
}

void unlockGames() {
    pthread_mutex_unlock(&seznamZamek);
}

/* ____________________________________________________________________________

        char *createGamesList() {

        Vytvori retezec obsahujici seznam vsech otevrenych her.
   ____________________________________________________________________________
 */
char *createGamesList() {


    int delkaTextu = 100;
    char *seznam = createTextLong(delkaTextu);
    char *pomString = NULL;


    int nalezenoPartii = 0;

    Game *pom = gameList;

    while (pom != NULL) {
        if (pom->hraci[1] == NULL) {
            nalezenoPartii++;
        }
        pom = pom->dalsiPartie;
    }

    if (nalezenoPartii == 0) {
        sprintf(seznam, "0");
        return seznam;
    } else {
        sprintf(seznam, "%d%s", nalezenoPartii, separator);
    }

    pom = gameList;

    while (pom != NULL) {
        if (pom->hraci[1] == NULL) {

            int delkaZaznamu = strlen(pom->hraci[0]->nick) + strlen(seznam);


            if (delkaZaznamu + 20 > delkaTextu) {
                delkaTextu = delkaTextu * 2;
                pomString = createTextLong(delkaTextu);
                sprintf(pomString, "%s", seznam);
                free(seznam);
                seznam = pomString;
            }

            char *id = createText();
            sprintf(id, "%d", pom->idGame);
            strcat(seznam, id);
            free(id);
            strcat(seznam, separator);

            strcat(seznam, pom->hraci[0]->nick);
            strcat(seznam, separator);

        }

        pom = pom->dalsiPartie;

    }

    return seznam;

}

/* ____________________________________________________________________________

        int newGameID() {

        Vygeneruje nove unikatni id hry.
   ____________________________________________________________________________
 */
int newGameID() {
    pthread_mutex_lock(&idZamek);
    int value = gameID;
    gameID++;

    pthread_mutex_unlock(&idZamek);

    return value;
}

/* ____________________________________________________________________________

        void removeGame(Game *partie) {

        Vyjme danou hru ze seznamu her.
   ____________________________________________________________________________
 */
void removeGame(Game *partie) {
    if (partie == NULL) {
        return;
    }

    Game *akt = gameList;
    Game *predchozi = NULL;

    while (akt != NULL) {

        if (partie == gameList) {
            //hledana partie je na zacatku seznamu
            printf("Partie cislo: \"%d\" vyjmuta ze seznamu  \n", akt->idGame);
            if (akt->dalsiPartie != NULL) {
                gameList = gameList->dalsiPartie;

            } else {
                gameList = NULL;
            }
            break;

        } else {
            if (akt == partie) {

                predchozi->dalsiPartie = akt->dalsiPartie;
                printf("Partie cislo: \"%d\" vyjmuta ze seznamu  \n", akt->idGame);
                break;
            } else {
                predchozi = akt;
                akt = akt->dalsiPartie;
            }
        }
    }

    printListOfGames();
}

/* ____________________________________________________________________________

        Game *findGame(int idPartie) {

        V seznamu her najde hru podle jejiho id.
   ____________________________________________________________________________
 */
Game *findGame(int idPartie) {
    Game *pom = gameList;
    //pthread_mutex_lock(&seznamZamek);
    while (pom != NULL) {

        if (pom->idGame == idPartie) {
            return pom;
            break;
        }
        else{pom = pom->dalsiPartie;}
    }
    //pthread_mutex_unlock(&seznamZamek);	
    return NULL;
}

/* ____________________________________________________________________________

        void freeGame(Game **partie) {

        Uvolni pamet alokovanou danou hrou.
   ____________________________________________________________________________
 */
void freeGame(Game **partie) {
    if (*partie != NULL) {
        free(*partie);
        *partie = NULL;
    }
}

/* ____________________________________________________________________________

        void addGame(Game *partie) {

        Prida danou partii do seznamu partii.
   ____________________________________________________________________________
 */
void addGame(Game *partie) {
    if (partie) {
        //pthread_mutex_lock(&seznamZamek);
        if (gameList == NULL) {
            gameList = partie;
        } else {
            Game *pom = gameList;

            while (pom ->dalsiPartie != NULL) {
                pom = pom->dalsiPartie;
            }
            pom->dalsiPartie = partie;
        }
        //pthread_mutex_unlock(&seznamZamek);
    }
    printListOfGames();
}

/* ____________________________________________________________________________

        Game *newGame(Player *hrac) {

        Vytvori novou partii a vrati na ni pointer.
   ____________________________________________________________________________
 */
Game *newGame(Player *hrac) {
    Game *nova = (Game*) malloc(sizeof (Game));
    nova->idGame = newGameID();
    nova->dalsiPartie = NULL;
    nova->hraci[0] = hrac;
    nova->hraci[1] = NULL;
    nova->stavGame = 0;
    nova->tahneHrac = NULL;
    nova->zaplnenoPolicek = 0;

    //vynulovani hraci plochy == nastaveni vsech poli na znak 2
    int i, j;
    for (i = 0; i < boardSize; i++) {
        //nova->hraciPlocha[i] = (char *) malloc(velikostHraciPlochy * sizeof(char));

        for (j = 0; j < boardSize; j++) {
            nova->hraciPlocha[i][j] = '2';
        }
    }


    return nova;
}

/* ____________________________________________________________________________

        void printListOfGames() {

        Vypise seznam do konzole seznam vsech partii a informace o hracich v kazde partii.
   ____________________________________________________________________________
 */
void printListOfGames() {
    if (gameList != NULL) {

        printf("\n *** SEZNAM HER ***\n");
        Game *pom = gameList;

        while (pom != NULL) {
            if (pom->hraci[1] == NULL) {
                printf("Otevrena  id: \"%d\" \t zalozil: \"%s\" \n", pom->idGame, pom->hraci[0]->nick);
                pom = pom->dalsiPartie;
            } else {
                printf("Spustena  id: \"%d\" \t hraji: \"%s\"  vs  \"%s\"\n", pom->idGame, pom->hraci[0]->nick, pom->hraci[1]->nick);
                pom = pom->dalsiPartie;
            }
        }
        printf("*******************\n\n");

    } else {
        printf("Seznam her je prazdny!\n");
    }


}






























