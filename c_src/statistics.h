#ifndef INFO_H
#define INFO_H

extern int bytesReceived;
extern int bytesSend;
extern int messagesReceived;
extern int messagesSend;

extern int connections;
extern int gamesPlayed;

extern int badMessages;


extern char logfile[];
extern int boardSize;

char *createText(void);
char *createTextLong(int velikost);
void makeLog(char **log);

#endif
