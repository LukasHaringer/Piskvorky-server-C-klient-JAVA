#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <netdb.h> 

typedef struct message {
    /*text zpravy*/
    char *buf[1024];
    /*adresa zpravy*/
    struct sockaddr_in clientaddr;
    /*delka adresy zpravy*/
    int clientlen;
    /*odkaz na dalsi zpravu*/
    struct message *nextMessage;
} Message;


typedef struct player {
    /*Adresa hrace*/
    struct sockaddr_in adresa;
    /*nick hrace*/
    char *nick;
    /*stav hrac*/
    int stav;
    /*indikator zda je pripojen*/
    int odpojen;
    /*zablokovani hrace pro odesilani*/
    int blocked;
    /*jeho index ve structure game*/
    int indexHrace;
    /*odkaz na struck game, ve ktere se nachazi*/
    struct game *partie;
    /*pocet prijatych zprav od toho klienta*/
    int packetNumberRecv;
     /*pocet odeslanych zprav pro tohoto klienta*/
    int packetNumberSent;
    int aktualizace;
    /*cas posledniho kontaktu*/
    time_t checked;
    /*odkaz na dalsiho hrace v seznamu*/
    struct player *dalsiHrac;
} Player;


typedef struct game {
    /*id hry*/
    int idGame;
    pthread_mutex_t zamekPartie;
    // 0 = prazdna, 1 - jen zakladajici hrac, 2 - oba hraci, 3 - probiha hra, 4 - konec;
    int stavGame;
    Player *tahneHrac;
    /*pole hraci plochy, 0 - kolecko, 1 krizek, 2 prazdne pole*/
    char hraciPlocha[16][16];
    /*pocet zaplnenych policek*/
    int zaplnenoPolicek;
    /*odkaz na hrace ve hre*/
    Player *hraci[2];
    /*odkaz na dalsi hru v seznamu*/
    struct game *dalsiPartie;
} Game;

#endif
