#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "structures.h"
#include "process.h"
#include "main.h"
#include "game.h"
#include "player.h"
#include "statistics.h"
#include "winCheck.h"


int pocetCastiZpravy = 5;

/* ____________________________________________________________________________

        void sendListOfOpensGames(Player *hrac) {

        Posle danemu hraci seznam vsech otevrenych her
   ____________________________________________________________________________
 */
void sendListOfOpensGames(Player *hrac) {
    char *seznam = createGamesList();
    char *zprava = createTextLong(strlen(seznam) + 20);
    sprintf(zprava, "a|%s", seznam);
    free(seznam);
    sendMessage(&zprava, hrac);
}

/* ____________________________________________________________________________

        void sendListOfOpenGamesToAll(Player *hrac) {

        Posle vsem hracum pripojenym k serveru seznam otevrenych her
   ____________________________________________________________________________
 */
void sendListOfOpenGamesToAll(Player *hrac) {

    Player *seznam = playerList;
    char *seznamString = createGamesList();

    printf("Posilam vsem hracum seznam her.\n");

    while (seznam != NULL) {
        if (seznam->odpojen == 0) {
            seznam->aktualizace=1;
            seznam = seznam->dalsiHrac;
        } else {
            seznam = seznam->dalsiHrac;
        }
    }
    free(seznamString);
}

/* ____________________________________________________________________________

        void zaloz_partii(Hrac *hrac)

        Zalozi novou partii. Nastavi ukazatele hrace na partii a naopak.
        Hraci zmeni stav na "cekajici na hru" a partii prida do seznamu.
   ____________________________________________________________________________
 */
void zaloz_partii(Player *hrac) {

    Game *nova = newGame(hrac);
    hrac->partie = nova;
    hrac->stav = 1;
    hrac->indexHrace = 0;
    nova->stavGame = 1;

    addGame(nova);
    printf("Hrac \"%s\" zalozil partii. \n", hrac->nick);


    char *zprava = createText();
    sprintf(zprava, "b|1");
    sendMessage(&zprava, hrac);
    sendListOfOpenGamesToAll(hrac);
}

/* ____________________________________________________________________________

        void connectPlayerToGame(char *idString, Player *hrac) {

        Pripoji hrace do zadane hry. Pokud vse uspesne probehne, spusti hru.
   ____________________________________________________________________________
 */
void connectPlayerToGame(char *idString, Player *hrac) {
    printf("pripojuji hrace %s \n", hrac->nick);

    int idPartie = atoi(idString);
    Game *hledana = findGame(idPartie);
    if (idPartie > 0 && hledana != NULL && hledana->hraci[1] == NULL) {


        //nastavenim ukazatele prida hrace do partie 
        hledana->hraci[1] = hrac;
        hrac->partie = hledana;
        hledana->stavGame = 2;
        hrac->indexHrace = 1;

        //odesle zakladajicimu info o protihraci
        char *oznameni = createText();
        sprintf(oznameni, "c|1|%s", hrac->nick);
        if (sendMessage(&oznameni, hledana->hraci[0]) == 0) {

            oznameni = createText();
            sprintf(oznameni, "c|1");
            sendMessage(&oznameni, hledana->hraci[1]);

            //spusteni hry
            startGame(hledana);

            printf("Hrac:  \"%s\" pripojen do partie %d\n", hrac->nick, hledana->idGame);

            sendListOfOpenGamesToAll(NULL);

        } else {
            //odesle pripojovanemu hraci informaci o neuspesnem pripojeni a seznam her
            char *oznameni = createText();
            sprintf(oznameni, "c|0");
            sendMessage(&oznameni, hrac);
            sendListOfOpensGames(hrac);
            printf("hraci %s se nepodarilo pripojit do partie %s\n", hrac->nick, idString);
        }
    } else {
        //odesle pripojovanemu hraci informaci o neuspesnem pripojeni a seznam her
        char *oznameni = createText();
        sprintf(oznameni, "c|0");
        sendMessage(&oznameni, hrac);
        sendListOfOpensGames(hrac);
        printf("hraci %s se nepodarilo pripojit do partie %s\n", hrac->nick, idString);
    }
}

/* ____________________________________________________________________________

        void startGame(Game *partie) {

        Spusti partii predanou parametrem. Oznami hracum zacatek hry, kdo ma krizky a kdo
        kolecka a ktery hrac hraje prvni.
   ____________________________________________________________________________
 */
void startGame(Game *partie) {
    partie->stavGame = 3;
    partie->hraci[0]->stav = 3;
    partie->hraci[1]->stav = 3;


    gamesPlayed++;

    //generator kdo zacne
    //int r = rand()% 2 ;
    int r = 0;
    if (r == 0) {
        char *oznameni = createText();
        sprintf(oznameni, "i|1");
        sendMessage(&oznameni, partie->hraci[0]);

        oznameni = createText();
        sprintf(oznameni, "i|0");
        sendMessage(&oznameni, partie->hraci[1]);

        partie->tahneHrac = partie->hraci[0];
    } else {
        char *oznameni = createText();
        sprintf(oznameni, "i|1");
        sendMessage(&oznameni, partie->hraci[1]);

        oznameni = createText();
        sprintf(oznameni, "i|0");
        sendMessage(&oznameni, partie->hraci[0]);
        partie->tahneHrac = partie->hraci[1];
    }
    printf("Hra id: %d  spustena, hraje hrac: \"%s\" \n", partie->idGame, partie->tahneHrac->nick);
}

/* ____________________________________________________________________________

        void cancelGame(Player *hrac) {

        Zrusi hru zalozenou timto hracem.
   ____________________________________________________________________________
 */
void cancelGame(Player *hrac) {

    Game *partie = hrac->partie;

    if (partie != NULL) {
        int idPartie = partie->idGame;

        if (partie->hraci[1] == NULL && partie->stavGame == 1) {
            //pokud je v partii hrac sam zrusi ji
            //odesle mu zpravu o zruseni a seznam otevrenych her
            removeGame(partie);

            freeGame(&partie);


            //odesle upesne zruseni
            char *oznameni = createText();
            sprintf(oznameni, "d|1");
            sendMessage(&oznameni, hrac);

        } else {
            //hrac v partii neni sam a uz zacala odpoji z ni oba hrace
            Player *druhy = partie->hraci[1];
            druhy->stav = 0;
            druhy->partie = NULL;
            removeGame(partie);
            freeGame(&partie);

            //oznameni o zruseni hry pro hrace
            char *oznameni = createText();
            sprintf(oznameni, "d|1|");
            sendMessage(&oznameni, hrac);

            oznameni = createText();
            sprintf(oznameni, "k|0|");
            sendMessage(&oznameni, druhy);
        }

        hrac->partie = NULL;
        hrac->stav = 0;

        printf("Partie %d zrusena zakladajicim hracem \"%s\"\n", idPartie, hrac->nick);
        sendListOfOpenGamesToAll(NULL);

    } else {
        printf("Hrac %s se pokousi zrusit neexistujici partii. \n", hrac->nick);
    }
}

/* ____________________________________________________________________________

        void odpoj_ze_serveru(Hrac *hrac)

        Odpoji daneho hrace ze serveru, pokud je prave ve hre, zrusi ji a oznami
        protihraci ze hru vyhral.
   ____________________________________________________________________________
 */
void disconnectFromServer(Player *hrac) {
    if (hrac->stav == 1) {
        cancelGame(hrac);
    } else {
        if (hrac->stav == 2 || hrac->stav == 3) {
            giveUp(hrac);
        }
    }
    
    hrac->odpojen = 1;
    hrac->stav = 4;
    printf("Hrac: \"%s\"  se odpojil. \n", hrac->nick);
}

/* ____________________________________________________________________________

        int index_druheho(int indexHrace)

        Pomocna funkce, ktera vrati index opacny tomu v parametru.
   ____________________________________________________________________________
 */
int index_druheho(int indexHrace) {
    if (indexHrace == 0) {
        return 1;
    } else {
        return 0;
    }

}

/* ____________________________________________________________________________

        void processMove(char **zprava, Player *hrac) {

        Vyhodnoti tah hrace - zda je platny, take zkontroluje zda nedoslo k vyhre nebo
        remize. Vysledek vyhodnoceni posle obema hracum.
   ____________________________________________________________________________
 */
void processMove(char **zprava, Player *hrac) {
    int x = atoi(zprava[1]);
    int y = atoi(zprava[2]);

    Game *partie = hrac->partie;
    Player *druhy = NULL;

    if (partie == NULL || x < 0 || x > (boardSize - 1) || y < 0 || y > (boardSize - 1)) {
        printf("spatny tah x:  %d   y:  %d \n", x, y);
        badMessages++;
        //posli neplatny tah
        char *oznameni = createText();
        sprintf(oznameni, "g|0|%s|%s", zprava[1], zprava[2]);
        sendMessage(&oznameni, hrac);
        return;
    } else {


        if (partie->tahneHrac != hrac) {
            printf("CHYBA - hrac:  \"%s\"  neni na rade a presto poslal tah! \n", hrac->nick);
            char *oznameni = createText();
            sprintf(oznameni, "g|0|%s|%s", zprava[1], zprava[2]);
            sendMessage(&oznameni, hrac);
            return;
        }

        //zkontroluj volnost pole
        if (partie->hraciPlocha[y][x] != '2') {
            //posli neplatny tah
            char *oznameni = createText();
            printf("Pole je obsazeno x:  %d   y:  %d \n", x, y);
            sprintf(oznameni, "g|0|%s|%s", zprava[1], zprava[2]);
            sendMessage(&oznameni, hrac);
            return;
        } else {
            //provedeni tahu
            if (hrac->indexHrace == 0) {
                partie->hraciPlocha[y][x] = '0';
            } else {
                partie->hraciPlocha[y][x] = '1';
            }

            //pointer na druheho hrace
            if (hrac->indexHrace == 0) {
                druhy = hrac->partie->hraci[1];
            } else {
                druhy = hrac->partie->hraci[0];
            }

            hrac->partie->tahneHrac = druhy;

            //tah je ok
            char *oznameni = createText();
            sprintf(oznameni, "g|1|%s|%s", zprava[1], zprava[2]);
            sendMessage(&oznameni, hrac);

            //oznam tah protihraci
            char *oznameni2 = createText();
            sprintf(oznameni2, "l|%s|%s", zprava[1], zprava[2]);
            sendMessage(&oznameni2, druhy);

            //vypis hraci plochy
            printBoard(partie);

            if (drawCheck(hrac->partie) == 0) {
                //remiza neni

                //zkontroluj vitezstvi tahu
                if (winCheck(x, y, hrac->partie) == 1) {


                    //konec, hrac vyhral
                    printf("Hrac: \"%s\" vyhral hru.\n", hrac->nick);
                    printBoard(partie);

                    /*
                            KONEC HRY
                            Spatne ukonceni hry
                     */
                    if (hrac->indexHrace == 0) {
                        druhy = hrac->partie->hraci[1];
                        gameOver(hrac->partie, 1);

                    } else {
                        druhy = hrac->partie->hraci[0];
                        gameOver(hrac->partie, 2);

                    }
                }

            } else {
                printf("Hraci remizuji!\n");
                printBoard(partie);
                //odesli remiza a skonci hru
                gameOver(hrac->partie, 0);
            }
        }


    }


}

/* ____________________________________________________________________________

        void giveUp(Player *hrac) {

        Oznami obema hracum ze je konec hry a urci viteze.
   ____________________________________________________________________________
 */
void giveUp(Player *hrac) {
    if (hrac && hrac->partie) {
        printf("Hrac \"%s\" vzdal hru cislo: %d\n", hrac->nick, hrac->partie->idGame);
        Game *partie = hrac->partie;
        if (hrac == partie->hraci[0]) {
            gameOver(hrac->partie, 4);
        } else {
            gameOver(hrac->partie, 3);
        }
    } else {
        printf("Hrac: %s se pokousel zrusit neexistujici partii. \n", hrac->nick);
    }
}

/* ____________________________________________________________________________

        void konec_hry(Partie *partie, int druhKonce)

        Ukonci danou partii a podle parametru druh konce posle hracum kdo zvitezil a z jakeho duvodu.
	
        int druhKonce -case: 0 remiza ,1 vyhral hrac na indexu nula , 2 vyhral hrac na indexu jedna,
                                 3 vyhral nula jednicka hru vzdal, 4 vyhral jedna nula hru vzdal, 5 vyhral nula
                                 jednicka timeout, 6 vyhral jedna nula timeout.
   ____________________________________________________________________________
 */
void gameOver(Game *partie, int druhKonce) {
    if (partie && partie->hraci[0] && partie->hraci[1]) {
        char *oznameni = createText();

        /*
        k|cislo   0 - zrusena zakladajicim
                        1 - souper hru vzdal,2 - hrac hru vzdal 3 - prohral jsi 4 - vyhral jsi,
                        5 - remiza, 6 souper timeout, 7 ty timeout	
         */

        switch (druhKonce) {
            case 0: //posli obema remizu
                sprintf(oznameni, "k|5|");
                sendMessage(&oznameni, partie->hraci[0]);
                oznameni = createText();
                sprintf(oznameni, "k|5|");
                sendMessage(&oznameni, partie->hraci[1]);
                break;

            case 1: //vyhral hrac 0
                oznameni = createText();
                sprintf(oznameni, "k|4|");
                sendMessage(&oznameni, partie->hraci[0]);
                oznameni = createText();
                sprintf(oznameni, "k|3|");
                sendMessage(&oznameni, partie->hraci[1]);
                break;

            case 2: //vyhral hrac 1
                oznameni = createText();
                sprintf(oznameni, "k|4|");
                sendMessage(&oznameni, partie->hraci[1]);
                oznameni = createText();
                sprintf(oznameni, "k|3|");
                sendMessage(&oznameni, partie->hraci[0]);
                break;

            case 3: //vyhral nula, jednicka hru vzdal
                sprintf(oznameni, "k|1|");
                sendMessage(&oznameni, partie->hraci[0]);
                oznameni = createText();
                sprintf(oznameni, "k|2|");
                sendMessage(&oznameni, partie->hraci[1]);
                break;

            case 4: //vyhral jednicka, nula hru vzdal
                sprintf(oznameni, "k|2|");
                sendMessage(&oznameni, partie->hraci[0]);
                oznameni = createText();
                sprintf(oznameni, "k|1|");
                sendMessage(&oznameni, partie->hraci[1]);
                break;

            case 5: //vyhral nula, jednicka neodehral vcas
                sprintf(oznameni, "k|6|");
                sendMessage(&oznameni, partie->hraci[0]);
                oznameni = createText();
                sprintf(oznameni, "k|7|");
                sendMessage(&oznameni, partie->hraci[1]);
                break;

            case 6: //vyhral jedna, nula neodehral vcas		
                sprintf(oznameni, "k|7|");
                sendMessage(&oznameni, partie->hraci[0]);
                oznameni = createText();
                sprintf(oznameni, "k|6|");
                sendMessage(&oznameni, partie->hraci[1]);
                break;

        }

        //zrus hru
        Player *prvni = partie->hraci[0];
        Player *druhy = partie->hraci[1];

        prvni->stav = 0;
        prvni->partie = NULL;
        druhy->stav = 0;
        prvni->partie = NULL;
        if(partie!=NULL){
        removeGame(partie);
        freeGame(&partie);}
    } else {
        printf("Pokus o ukonceni neexistujici hry nebo hry s neexistujicim hracem!!! \n");
    }
}


/*
        Rozparsuje a zaridi zpracovani zpravy od klienta.
 */

/* ____________________________________________________________________________

        void processMessage(char **zprava, Player *hrac) {

        Rozparsuje zpravu od hrace a podle prvniho znaku zjisti jeji typ, nasledne
        zpravu posle prislusne funkci k dalsi zpracovani.
   ____________________________________________________________________________
 */
void processMessage(char **zprava, Player *hrac) {
    if (zprava == NULL || hrac == NULL) {
        return;
    }

    //zamkneme partii i seznam


    char c = zprava[0][0];
    //printf("znak 0 %c \n",c);

    lockGames();
    char *msg = createText();
    switch (c) {

        case 'a': //posli seznam otevrenych her
            sendListOfOpensGames(hrac);
            break;

        case 'b': //zaloz hru
            zaloz_partii(hrac);
            break;

        case 'c': //pripoj k existujici hre
            connectPlayerToGame(zprava[1], hrac);
            break;

        case 'd': //zrus zalozenou hru
            cancelGame(hrac);
            break;

        case 'e': //vzdej hru
            giveUp(hrac);
            break;

        case 'f': //odpoj ze serveru, pokud aktualne hraje zaroven hru vzda
            disconnectFromServer(hrac);
            break;

        case 'g': //tahni na poli x y// odpoved tah je ok
            //kontrola tahu a stavu hry
            processMove(zprava, hrac);
            break;

        case 'i': //odesle hraci zpravu ze je na rade
            sprintf(msg, "i|");
            sendMessage(&msg, hrac);
            break;

        case 'v': //hearhbeat
            break;

        default: //chybna zprava
            printf("default");
            badMessages++;
            break;
    }
    if (msg != NULL) free(msg);
    free(zprava);
    unlockGames();
}

/* ____________________________________________________________________________

        char **parseMessage(char *zprava)

        Rozdeli zpravu podle oddelovaciho znaku na jednotlive casti a kazdou cast
        umisti na jednu radku dvourozmerneho pole znaku krome sequence number.
   ____________________________________________________________________________
 */
char **parseMessage(char *zprava) {
    if (zprava != NULL) {
        char **pole = (char**) malloc(sizeof (char*)*pocetCastiZpravy);
        pole[0] = strtok(zprava, separator);

        int i;
        for (i = 0; i < pocetCastiZpravy; i++) {
            pole[i] = strtok(NULL, separator);
            if (pole[i] == NULL) {
                return pole;
            }
        }
    } else {

    }
    return NULL;

}

/* ____________________________________________________________________________

        char **parseMessageForAck(char *zprava)

        Rozdeli zpravu podle oddelovaciho znaku na jednotlive casti a kazdou cast
        umisti na jednu radku dvourozmerneho pole znaku vcetne sequence number.
   ____________________________________________________________________________
 */
char **parseMessageForAck(char *zprava) {
    if (zprava != NULL) {
        char **pole = (char**) malloc(sizeof (char*)*pocetCastiZpravy);
        pole[0] = strtok(zprava, separator);

        int i;
        for (i = 1; i < pocetCastiZpravy; i++) {
            pole[i] = strtok(NULL, separator);
            if (pole[i] == NULL) {
                return pole;
            }
        }
    } else {

    }
    return NULL;

}

/* ____________________________________________________________________________

        int getSequenceNumber(char *zprava) {

        Vrati sequence number dane zpravy.
   ____________________________________________________________________________
 */
int getSequenceNumber(char *zprava) {
    if (zprava != NULL) {
        char **pole = (char**) malloc(sizeof (char*)*pocetCastiZpravy);
        pole[0] = strtok(zprava, separator);

        return atoi(pole[0]);
    }

    return 0;
}