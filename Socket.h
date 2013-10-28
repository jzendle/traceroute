/* 
 * File:   Socket.h
 * Author: jzendle
 *
 * Created on October 5, 2013, 7:55 AM
 */

#ifndef SOCKET_H
#define	SOCKET_H

#include <netinet/in.h>

typedef struct SocketStruct Socket;
typedef Socket *pSocket;

enum Protocol {
    STREAM, DATAGRAM, RAW
};

typedef enum Protocol Protocol;


/* null host implies INET_ADDR_ANY for a server address */
extern int socketCreate(Socket **ps, const char *hostName, int port, Protocol protocol);

extern int socketListen(Socket *ps, int backlog);

extern int socketSetTTL(Socket *ps, int ttl);

extern int socketSetTimeout(Socket *ps, int seconds);

extern int socketSetRecordRoute(Socket *ps);

extern int socketGetRecordRoute(Socket *ps, char *options, int *pLen);

extern int socketSendTo(Socket *ps, const struct sockaddr_in *sendTo, const char *data, int dataLen);

/* null host implies INET_ADDR_ANY for a server address */
extern int socketRecvFrom(Socket *ps, char *server, int *port, char *data, int *dataLen);

extern int socketRelease(Socket *ps);



#endif	/* SOCKET_H */

