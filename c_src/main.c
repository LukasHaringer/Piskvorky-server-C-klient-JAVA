#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <regex.h>        
#include "structures.h"
#include "main.h"
#include "statistics.h"
#include "player.h"
#include "process.h"
#include "game.h"
#include "message.h"


#define DEFAULTPORT 10005;
time_t start;
time_t end;


char *ipAdresa;
struct hostent *he;

struct timeval timeout;

pthread_t serverThread;
pthread_t klientThread;
char separator[] = "|";
pthread_mutex_t odesilani;

int priznakKonec = 0;
int zadano = 0;
char *adresa;

/* ____________________________________________________________________________

        int send(char **zprava, Player *hrac) {

        Posle zpravu danemu hraci a zavola metodu pro jeji zalogovani
   ____________________________________________________________________________
 */
int sendMessage(char **zprava, Player *hrac) {
    while (hrac->blocked == 1) {
    }
    hrac->blocked = 1;

    if (*zprava && hrac) {
        strcat(*zprava, "|");
        char *zpravaCislo = createText();
        sprintf(zpravaCislo, "%d|", hrac->packetNumberSent);
        strcat(zpravaCislo, *zprava);
        int clientlen = sizeof (hrac->adresa);
        bytesSend += sendto(serverSocket, zpravaCislo, strlen(zpravaCislo) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
        messagesSend++;
        char *log = createText();
        sprintf(log, "Posilam pro: \"%s\" zprava: %s \n", hrac->nick, zpravaCislo);
        makeLog(&log);

        Message *message;
        time_t start;
        time_t now;
        time_t send;
        time(&start);
        time(&send);
        double waiting;
        double lastSend;
        while (1) {
            time(&now);
            waiting = difftime(now, start);
            lastSend = difftime(now, send);
            if (lastSend > 1) {
                bytesSend += sendto(serverSocket, zpravaCislo, strlen(zpravaCislo) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
                messagesSend++;
                time(&send);
                badMessages++;
                printf("Posilam znovu\n");
            }

            if (waiting > 8) {
                hrac->blocked = 0;
                hrac->odpojen = 1;
                return 1;
            }
            message = findMessage(hrac->adresa);
            if (message == NULL) {
                continue;
            }
            removeMessage(message);
            char **pole = parseMessageForAck((char *) message->buf);

            if (atoi(pole[0]) == hrac->packetNumberRecv - 1) {
                sendAckAgain(hrac);
            }
            if (pole[0][0] == '0') {
                if (pole[1][0] == 'p') {
                    if (atoi(pole[2]) == hrac->packetNumberSent) {
                        freeMessage(message);
                        time(&hrac->checked);
                        break;
                    }
                }
            }
            freeMessage(message);
        }
        hrac->packetNumberSent++;
        free(*zprava);
        free(zpravaCislo);
    }
    hrac->blocked = 0;
    return 0;
}


/* ____________________________________________________________________________

        int sendReconnect(Player *hrac) 

        Posle zpravu danemu hraci s informacemi pro reconnect
   ____________________________________________________________________________
 */
int sendReconnect(Player *hrac) {
    while (hrac->blocked == 1) {
    }
    hrac->blocked = 1;

    if (hrac) {
        char *zpravaCislo = createTextLong(1000);
        sprintf(zpravaCislo, "%d|h|", hrac->packetNumberSent);

        if (hrac->partie->tahneHrac == hrac) {
            strcat(zpravaCislo, "0|");
        } else {
            strcat(zpravaCislo, "1|");
        }
        if (hrac->partie->hraci[0] == hrac) {
            strcat(zpravaCislo, "0|");
        } else {
            strcat(zpravaCislo, "1|");
        }

        int i;
        for (i = 0; i < 16; i++) {
            int j;
            for (j = 0; j < 16; j++) {
                char str[5];
                sprintf(str, "%c|", hrac->partie->hraciPlocha[i][j]);
                strcat(zpravaCislo, str);
            }
        }

        int clientlen = sizeof (hrac->adresa);
        bytesSend += sendto(serverSocket, zpravaCislo, strlen(zpravaCislo) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
        messagesSend++;

        char *log = createText();
        sprintf(log, "Posilam pro: \"%s\" reconnect \n", hrac->nick);
        makeLog(&log);

        Message *message;
        time_t start;
        time_t now;
        time_t send;
        time(&start);
        time(&send);
        double waiting;
        double lastSend;
        while (1) {
            time(&now);
            waiting = difftime(now, start);
            lastSend = difftime(now, send);
            if (lastSend > 1) {
                bytesSend += sendto(serverSocket, zpravaCislo, strlen(zpravaCislo) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
                messagesSend++;
                time(&send);
                badMessages++;
                printf("Posilam znovu\n");
            }

            if (waiting > 7) {
                hrac->blocked = 0;
                hrac->odpojen = 1;
                return 1;
            }
            message = findMessage(hrac->adresa);
            if (message == NULL) {
                continue;
            }
            removeMessage(message);
            char **pole = parseMessageForAck((char *) message->buf);
            if (atoi(pole[0]) == hrac->packetNumberRecv - 1) {
                sendAckAgain(hrac);
            }
            if (pole[0][0] == '0') {
                if (pole[1][0] == 'p') {
                    if (atoi(pole[2]) == hrac->packetNumberSent) {
                        time(&hrac->checked);
                        break;
                    }
                }
            }
        }
        hrac->packetNumberSent++;
        free(zpravaCislo);
    }
    hrac->blocked = 0;
    return 0;
}

/* ____________________________________________________________________________

        void alive( Hrac *hrac)

        Posle zpravu danemu hraci pro overeni, ze je stale pripojen a zavola metodu pro jeji zalogovani
   ____________________________________________________________________________
 */
int alive(Player *hrac) {
    while (hrac->blocked == 1) {
    }
    hrac->blocked = 1;

    if (hrac) {
        char *zpravaCislo = createText();
        sprintf(zpravaCislo, "%d|v", hrac->packetNumberSent);
        int clientlen = sizeof (hrac->adresa);
        bytesSend += sendto(serverSocket, zpravaCislo, strlen(zpravaCislo) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
        messagesSend++;
        char *log = createText();
        sprintf(log, "Posilam pro: \"%s\" zprava: %s \n", hrac->nick, zpravaCislo);
        makeLog(&log);

        Message *message;
        time_t start;
        time_t now;
        time_t send;
        time(&start);
        time(&send);
        double waiting;
        double lastSend;
        while (1) {
            time(&now);
            waiting = difftime(now, start);
            lastSend = difftime(now, send);
            if (lastSend > 1) {
                bytesSend += sendto(serverSocket, zpravaCislo, strlen(zpravaCislo) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
                messagesSend++;
                time(&send);
                printf("Posilam znovu\n");
            }

            if (waiting > 8) {
                hrac->blocked = 0;
                hrac->odpojen = 1;
                return 1;
            }
            message = findMessage(hrac->adresa);
            if (message == NULL) {
                continue;
            }
            removeMessage(message);
            char **pole = parseMessageForAck((char *) message->buf);
            if (atoi(pole[0]) == hrac->packetNumberRecv - 1) {
                sendAckAgain(hrac);
                freeMessage(message);
            }
            if (pole[0][0] == '0') {
                if (pole[1][0] == 'p') {
                    if (atoi(pole[2]) == hrac->packetNumberSent) {
                        freeMessage(message);
                        break;
                    }
                }
            }
            freeMessage(message);
        }
        hrac->packetNumberSent++;
        free(zpravaCislo);
    }
    hrac->blocked = 0;
    return 0;
}

/* ____________________________________________________________________________

        void posliAck(Hrac *hrac)

        Posle potvzeni o prijeti zpravy
   ____________________________________________________________________________
 */

void sendAck(Player *hrac) {
    char *zpravaCislo = createText();
    sprintf(zpravaCislo, "%d|", hrac->packetNumberRecv);
    int clientlen = sizeof (hrac->adresa);
    char *zprava = createText();
    sprintf(zprava, "0|p|");
    strcat(zprava, zpravaCislo);
    bytesSend += sendto(serverSocket, zprava, strlen(zprava) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
    messagesSend++;
    printf("%s ack\n", zprava);
    free(zprava);
    free(zpravaCislo);
}

/* ____________________________________________________________________________

        void posliAck(Hrac *hrac)

        Posle potvzeni o prijeti zpravy pokud prvni nedorazilo
   ____________________________________________________________________________
 */

void sendAckAgain(Player *hrac) {
    char *zpravaCislo = createTextLong(100);
    sprintf(zpravaCislo, "%d|", hrac->packetNumberRecv - 1);
    int clientlen = sizeof (hrac->adresa);
    char *zprava = createText();
    sprintf(zprava, "0|p|");
    strcat(zprava, zpravaCislo);
    bytesSend += sendto(serverSocket, zprava, strlen(zprava) * sizeof (char), 0, (struct sockaddr *) &hrac->adresa, clientlen);
    messagesSend++;
    printf("%s ack\n", zprava);

    free(zprava);
    free(zpravaCislo);
}

/* ____________________________________________________________________________

        void *clientThread(void *novyHrac)

        Funkce vlakana klienta. Ceka na zpravu od klienta a zaridi jeji zpracovani.
   ____________________________________________________________________________
 */
void *clientThread(void *novyHrac) {
    Player *hrac = (Player*) novyHrac;

    Message *message;

    time_t nowTime;
    time(&hrac->checked);

    while (1) {
        time(&nowTime);
        if (hrac->odpojen == 1) {
            break;
        }

        if (hrac->aktualizace == 1) {
            sendListOfOpensGames(hrac);
            hrac->aktualizace = 0;
        }

        if (hrac->stav == 1 && difftime(nowTime, hrac->checked) > 15) {
            if (alive(hrac) == 0) {
                time(&hrac->checked);
            } else {
                disconnectFromServer(hrac);
            }
        }

        if (hrac->stav == 3 && difftime(nowTime, hrac->checked) > 10) {
            if (alive(hrac) == 0) {
                time(&hrac->checked);
            }
        }

        while (hrac->blocked == 1) {
        }
        message = findMessage(hrac->adresa);

        if (message == NULL) {
            continue;
        }

        removeMessage(message);

        char *zpravaPaket = createText();
        strcpy(zpravaPaket, (char *) message->buf);
        int realPacketNum = getSequenceNumber(zpravaPaket);
        if (realPacketNum == hrac->packetNumberRecv - 1 && hrac->packetNumberRecv - 1 != 0) {
            sendAckAgain(hrac);
        }
        if (realPacketNum == hrac->packetNumberRecv) {
            printf("%s -zpracovano \n", (char *) message->buf);
            sendAck(hrac);
            hrac->packetNumberRecv++;

            char *log = createTextLong(100);
            sprintf(log, "\nPrijato od: \"%s\" \tzprava: %s\n", hrac->nick, (char *) message->buf);
            makeLog(&log);

            char **pole = parseMessage((char *) message->buf);
            processMessage(pole, novyHrac);
        }
    }

    if (hrac->stav == 3) {
        char *notif = createText();
        sprintf(notif, "z|");
        if (hrac->partie->hraci[0] == hrac) {
            sendMessage(&notif, hrac->partie->hraci[1]);
        } else {
            sendMessage(&notif, hrac->partie->hraci[0]);
        }
    }

    if (hrac->stav != 3) {
        disconnectFromServer(hrac);
        removePlayer(hrac);
        freePlayer(hrac);
    }

    return NULL;
}

/* ____________________________________________________________________________

        void *connectThread(struct sockaddr_in * clientaddr)

        Funkce zajistuje pocatecni komunikaci s hracem, zaridi jeho pridani do 
        seznamu hracu a spusti vlakno ktere ho bude dale obsluhovat.
   ____________________________________________________________________________
 */
void *connectThread(struct sockaddr_in * clientaddr) {

    Message *message;
    struct sockaddr_in adresa;
    memcpy(&adresa, clientaddr, sizeof (struct sockaddr_in));
    char *zprava = createTextLong(64);
    memset(zprava, '\0', 64);
    message = findMessage(adresa);
    removeMessage(message);
    printListOfPlayers();
    printf("prijato: %s\n", (char *) message->buf);
    if (message->buf != NULL) {
        char **pole = parseMessageForAck((char *) message->buf);
        char *druhZpravy = pole[1];
        char *jmenoHrace = "default";
        

        Player *hrac = newPlayer(adresa, jmenoHrace);

        if (strcmp(druhZpravy, "1") == 0) {
            if (pole[2] != NULL) {
                if (findPlayer(pole[2]) == NULL) {
                    if (strlen(jmenoHrace) > 1) {
                        hrac->nick = pole[2];
                        sendAck(hrac);
                        hrac->packetNumberRecv++;
                        //zalogovani pridani hrace
                        char *log = createText();
                        sprintf(log, "Hrac: \"%s\" uspesne pripojen.\n", hrac->nick);
                        makeLog(&log);

                        //pridej hrace do spojaku
                        addPlayer(hrac);
                        printf("pridavam hrace do spojaku a chystam se vypsat seznam\n");
                        printListOfPlayers();

                        sendListOfOpensGames(hrac);
                        //spust mu vlakno
                        pthread_create(&klientThread, NULL, clientThread, (void*) hrac);

                    }
                } else {
                    char *log = createText();
                    sprintf(log, "Neplatny format jmena: %s\nHrac nebyl pridan\n", jmenoHrace);
                    makeLog(&log);

                    badMessages++;
                }

            }
        } else if (strcmp(druhZpravy, "r") == 0) {
            if ((jmenoHrace = pole[2]) != NULL) {

                Player *recHrac;
                recHrac = findPlayer(jmenoHrace);
                if (recHrac != NULL) {
                    if (recHrac->odpojen == 1) {
                        recHrac->adresa = hrac->adresa;
                        recHrac->packetNumberRecv = 1;
                        recHrac->packetNumberSent = 1;
                        sendAck(recHrac);
                        recHrac->packetNumberRecv++;
                        sendReconnect(recHrac);
                        recHrac->odpojen = 0;
                        pthread_create(&klientThread, NULL, clientThread, (void*) recHrac);
                    }
                }
            }

        } else {

            char *log = createText();
            sprintf(log, "Chybny format zpravy: %s\n Hrac nebyl prihlasen", zprava);
            makeLog(&log);
            badMessages++;
        }

    }
    return NULL;
}

/* ____________________________________________________________________________

        void *receiveThread(void *port) {

        Funkce vlakna serveru. Vytvori socket a ceka na prichozi spojeni od klientu.
        Novemu klientovi spusti obsluzne vlakno.
   ____________________________________________________________________________
 */
void *receiveThread(void *port) {
    int portServer = *((int*) port);

    free(port);
    struct sockaddr_in serverAdresa;
    char buf[128];
    socklen_t clientlen; /* byte size of client's address */
    struct sockaddr_in clientaddr; /* client addr */


    if ((serverSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("Socket nelze otevrit, server se nepodarilo spustit\n");
        exit(1);
    } else {
        int on = 1;

        //umozni opetovne pouziti portu po padu serveru
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));
        printf("OK... socket uspesne vytvoren\n");

        serverAdresa.sin_family = AF_INET;
        serverAdresa.sin_port = htons(portServer);
        if (zadano == 1) {
            serverAdresa.sin_addr.s_addr = inet_addr(adresa);
        } else {
            serverAdresa.sin_addr.s_addr = INADDR_ANY;
        }

        memset(&(serverAdresa.sin_zero), '\0', 8);
        clientlen = sizeof (clientaddr);

        if (bind(serverSocket, (struct sockaddr *) &serverAdresa, sizeof (struct sockaddr_in)) == -1) {
            close(serverSocket);
            exit(1);
        } else {
            printf("OK... bind uspesny\n");
            Message *message;
            while (1) {
                bytesReceived += recvfrom(serverSocket, buf, 1024, 0, (struct sockaddr *) &clientaddr, &clientlen);
                messagesReceived++;
                message = newMessage(buf, clientaddr, clientlen);
                if (check((char*) message->buf) == 0) {
                    if (confirmMessage(clientaddr)) {
                        connections++;
                        printf("server received %s\n", buf);
                        struct sockaddr_in *adresa = &clientaddr;
                        pthread_create(&serverThread, NULL, (void*) &connectThread, adresa);
                        addMessage(message);
                    } else {
                        printf("server received %s\n", buf);
                        addMessage(message);
                    }
                } else {
                    badMessages++;
                }
                memset(buf, 0, 128 * sizeof (char));
            }
            return NULL;

        }
        return NULL;
    }
}

/* ____________________________________________________________________________

        void serverInfo()

        Funkce vypise statistiky serveru.
   ____________________________________________________________________________
 */
void serverInfo() {
    end = time(NULL);
    double diff = difftime(end, start);

    printf("\n\n*** STATISTIKA ***\n");
    printf("Prijato bytu: %d \n", bytesReceived);
    printf("Odeslano bytu: %d \n", bytesSend);
    printf("Prijato zprav: %d \n", messagesReceived);
    printf("Odeslano zprav: %d \n", messagesSend);
    printf("Chybne zprav: %d \n\n", badMessages);
    printf("Pripojeno lidi: %d \n", connections);
    printf("Odehrano her: %d \n", gamesPlayed);
    printf("Server bezi %d sekund.\n", (int) diff);
}

/* ____________________________________________________________________________

        void zkontroluj(char *zprava)

        Zkontroluje prijatou zpravu, zda neopsahuje nepovolene znaky
   ____________________________________________________________________________
 */
int check(char *zprava) {
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^[a-zA-Z0-9| ]*$", 0);

    reti = regexec(&regex, zprava, 0, NULL, 0);
    if (!reti) {
        return 0;
    } else if (reti == REG_NOMATCH) {
        return 1;
    } else {
        exit(1);
    }
}

/* ____________________________________________________________________________

        void zkontrolujIP(char *zprava)

        Zkontroluje zadanou IP, zda neopsahuje nepovolene znaky
   ____________________________________________________________________________
 */
int checkIP(char *zprava) {
    regex_t regex2;
    int reti2;
    reti2 = regcomp(&regex2, "^[0-9.]*$", 0);
    reti2 = regexec(&regex2, zprava, 0, NULL, 0);
    if (!reti2) {
        return 0;
    } else if (reti2 == REG_NOMATCH) {
        return 1;
    } else {
        exit(1);
    }
}

/* ____________________________________________________________________________

        int main(int argv, char *args[])

        Hlavni funkce aplikace, pokud vstupni parametry obsahuji platne cislo portu
        vytvori na nem server, jinak bude server vytvoren na defaultnim portu.
        Nasledne spusti vlakno serveru. Po prikazu ke skonceni oznami hracum vypnuti
        serveru a uvolni vsechnu pamet.
   ____________________________________________________________________________
 */
int main(int argv, char *args[]) {

    printf("Start serveru \n");

    //cas
    start = time(NULL);
    //vytvoreni logovaciho souboru, respektive smazani predchoziho	
    FILE *logSoubor = fopen(logfile, "w");
    fclose(logSoubor);

    if (argv > 1) {
        if (args[1][0] == 'h') {
            printf("Spusteni serveru: ./server <cislo adresy> <cislo portu>\n");
            exit(0);
        }
    }

    if (argv > 1) {
        if (args[1][0] == '-') {
            if (args[1][1] == 'h') {
                printf("Spusteni serveru: ./server <cislo adresy> <cislo portu>\n");
                exit(0);
            }
        }
    }

    char *log = createText();
    // zjisteni a nastaveni portu
    int port = 0;
    if (argv == 3) {
        port = atoi(args[2]);
        if (checkIP(args[1]) == 0) {
            adresa = args[1];
            zadano = 1;
            if (port > 1024 && port < 65536) {
                printf("Server pobezi na adrese %s a portu: %d \n", adresa, port);
            }
        } else {
            port = DEFAULTPORT;
            printf("Adresa nebo port zadany spatne. Pro napovedu spuste s parametrem h. Server pobezi na defaultni adrese inaddr_any a portu %d\n", port);
            zadano = 0;
        }

    } else {
        port = DEFAULTPORT;
        printf("Adresa nebo port zadany spatne. Pro napovedu spuste s parametrem h. Server pobezi na defaultni adrese inaddr_any a portu %d\n", port);
        zadano = 0;
    }
 
    //spusteni serveru
    int *portCislo = (int*) malloc(sizeof (int));
    *portCislo = port;
    pthread_create(&serverThread, NULL, receiveThread, (void*) portCislo);

    char vstup[1023];

    while (scanf("%s", vstup) != -1) {
        if (strcmp(vstup, "q") == 0)
            break;

        if (strcmp(vstup, "s") == 0) {
            serverInfo();
        }
    }

    /* posle vsem hracum zpravu o konci, odpoji je a uvolni pamet struktur*/
    Player *hrac = playerList;

    while (hrac != NULL) {
        char *zprava = createText();
        sprintf(zprava, "q");
        sendMessage(&zprava, hrac);
        hrac = hrac->dalsiHrac;
    }

    hrac = playerList;
    while (hrac != NULL) {
        hrac = hrac->dalsiHrac;
    }

    pthread_cancel(serverThread);
    sleep(2);

    log = createText();
    sprintf(log, "Server konci\n");
    makeLog(&log);

    //vypis statistiky
    serverInfo();

    printf("Server vypnut\n");

    return 0;

}










