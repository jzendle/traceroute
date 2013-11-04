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
extern int socket_Create(Socket **ps, const char *hostName, int port, Protocol protocol);

extern int socket_Listen(Socket *ps, int backlog);

extern int socket_SetTTL(Socket *ps, int ttl);

extern int socket_SetTimeout(Socket *ps, int seconds);

extern int socket_SetRecordRoute(Socket *ps);

extern int socket_GetRecordRoute(Socket *ps, char *options, int *pLen);

extern int socket_SendTo(Socket *ps, const struct sockaddr_in *sendTo, const char *data, int dataLen);

/* null host implies INET_ADDR_ANY for a server address */
extern int socket_RecvFrom(Socket *ps, char *server, int *port, char *data, int *dataLen);

extern int socket_Release(Socket *ps);



#endif	/* SOCKET_H */

