#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h> 

#include "structures.h"
#include "message.h"
#include "statistics.h"

Message *messagesList = NULL;
struct sockaddr_in knownAdresses[100];
int KnownAdressesCount = 0;
int serverSocket;
pthread_mutex_t seznamMessageZamek;

/* ____________________________________________________________________________

        void freeMessage(Message *message) {

        Uvolni pamet zabranou strukturou message.
   ____________________________________________________________________________
 */
void freeMessage(Message *message) {
    Message *pom;
    pom = message;
    free(pom);
    pom = NULL;
}

/* ____________________________________________________________________________

        void removeMessage(Message *message) {

        Vyjme daneho hrace ze spojoveho seznamu.
   ____________________________________________________________________________
 */
void removeMessage(Message *message) {

    pthread_mutex_lock(&seznamMessageZamek);

    Message *akt = messagesList;
    Message *predchozi = NULL;

    while (akt != NULL) {

        if (message == messagesList) {

            //hledany hrac je na zacatku seznamu

            if (akt->nextMessage != NULL) {
                messagesList = messagesList->nextMessage;
            } else {
                messagesList = NULL;
            }
            break;
        } else {
            if (akt == message) {
                predchozi->nextMessage = akt->nextMessage;
                printf("Message vyjmuta ze seznamu\n");
                break;
            } else {
                predchozi = akt;
                akt = akt->nextMessage;
            }
        }
    }

    pthread_mutex_unlock(&seznamMessageZamek);
}

/* ____________________________________________________________________________

        Message *findMessage(struct sockaddr_in clientaddr) {

        Podle socketu najde hrace a vrati na nej pointer.
   ____________________________________________________________________________
 */
Message *findMessage(struct sockaddr_in clientaddr) {

    pthread_mutex_lock(&seznamMessageZamek);

    if (messagesList != NULL) {

        Message *pom = messagesList;

        while (pom != NULL) {
            if (inet_ntoa(pom->clientaddr.sin_addr) == inet_ntoa(clientaddr.sin_addr) && pom->clientaddr.sin_port == clientaddr.sin_port) {
                //   printf("nalezeno\n");
                pthread_mutex_unlock(&seznamMessageZamek);
                return pom;
            } else {
                pom = pom->nextMessage;
            }
        }
    }

    pthread_mutex_unlock(&seznamMessageZamek);
    return NULL;

}

/* ____________________________________________________________________________

        void addMessage(Message *message) {

        Prida danou message do spojoveho seznamu.
   ____________________________________________________________________________
 */
void addMessage(Message *message) {
    if (message) {
        pthread_mutex_lock(&seznamMessageZamek);
        if (messagesList == NULL) {
            messagesList = message;
        } else {
            Message *pom = messagesList;
            while (pom->nextMessage != NULL) {
                pom = pom->nextMessage;
            }
            pom->nextMessage = message;
        }
    }
    pthread_mutex_unlock(&seznamMessageZamek);

}

/* ____________________________________________________________________________

        Message *newMessage(char buf[1024], struct sockaddr_in clientaddr, int clientlen) {

        Alokuje pamet pro noveho hrace a naplni strukturu daty z parametru funkce.
   ____________________________________________________________________________
 */
Message *newMessage(char buf[1024], struct sockaddr_in clientaddr, int clientlen) {

    Message *message = (Message*) malloc(sizeof (Message));
    memcpy(message->buf, buf, sizeof (char) * 128);
    memcpy(&message->clientaddr, &clientaddr, sizeof (clientaddr));
    message->clientlen = clientlen;
    return message;
}

/* ____________________________________________________________________________

        void vypis_seznam_message() 

        Vypise seznam messages
   ____________________________________________________________________________*/
void printListOfMessages() {
    pthread_mutex_lock(&seznamMessageZamek);

    if (messagesList != NULL) {
        printf("\n*** SEZNAM HRACU ***\n");
        Message *pom = messagesList;

        while (pom != NULL) {
            char *ip = inet_ntoa(pom->clientaddr.sin_addr);
            printf("%s %d ", ip, pom->clientaddr.sin_port);
            printf("%s\n", (char *)pom->buf);
            pom = pom->nextMessage;
        }
        printf("********************\n\n");

    } else {
        printf("Seznam hracu je prazdny!\n");
    }



    pthread_mutex_unlock(&seznamMessageZamek);
}

/* ____________________________________________________________________________

        void over_message() 

        Overi zda adresa dane messages je jiz v seznamu messages
   ____________________________________________________________________________*/
int confirmMessage(struct sockaddr_in clientaddr) {
    int i;
    for (i = 0; i < KnownAdressesCount; i++) {
        if (inet_ntoa(knownAdresses[i].sin_addr) == inet_ntoa(clientaddr.sin_addr) && knownAdresses[i].sin_port == clientaddr.sin_port) {
            return 0;
        }
    }
    knownAdresses[i] = clientaddr;
    KnownAdressesCount++;
    return 1;
}










