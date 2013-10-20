#ifndef SOCKET_H
#include "Socket.h"
#include "Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>


extern int inet_aton(const char *__cp, struct in_addr *__inp);

struct SocketStruct {
    int socket;
    long bytesIn;
    long bytesOut;
};

/* private functions */

int makeProto(Protocol prot) {
    switch (prot) {
        case DATAGRAM:
            return SOCK_DGRAM;
        case RAW:
            return SOCK_RAW;
        case STREAM:
            return SOCK_STREAM;
        default:
            errv(0, "unknown protocol %d\n", prot);
            return 0;
    }
}

/* public functions */
int socketCreate(Socket **ps, const char *host, int port, Protocol protocol) {
    int fd;
    struct sockaddr_in adr_inet;
    int adr_len;
    int prot = makeProto(protocol);

    /* use default protocols unless one is not supplied */
    /* for simplicity, assume that RAW -> ICMP, others -> default (0) */
    fd = socket(PF_INET, prot, protocol == RAW ? IPPROTO_ICMP : 0);

    if (fd == -1) {
        perror("socketCreate: socket");
        return EXIT_FAILURE;
    }

    Socket *newSocket = calloc(1, sizeof (Socket));
    if (newSocket == NULL) {
        perror("socketCreate: calloc");
        return EXIT_FAILURE;
    }

    newSocket->socket = fd;

    *ps = newSocket;

    adr_len = sizeof (adr_inet);
    memset(&adr_inet, 0, sizeof (adr_inet));
    adr_inet.sin_family = AF_INET;
    adr_inet.sin_port = ntohs(port);

    if (host == NULL)
        adr_inet.sin_addr.s_addr = ntohl(INADDR_ANY);
    else {
        if (inet_aton(host, &adr_inet.sin_addr) == 0) {
            perror("socketCreate: inet_aton");
            return EXIT_FAILURE;
        }

    }

    /* Now bind the address to the socket */
    if (bind(fd,
            (struct sockaddr *) &adr_inet,
            adr_len) == -1) {
        perror("socketCreate: bind");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

int socketListen(Socket *ps, int backlog) {
    return listen(ps->socket, backlog);
}

int socketSetTTL(Socket *ps, int ttl) {
    if (setsockopt(ps->socket, IPPROTO_IP, IP_TTL, &ttl, sizeof (int)) != EXIT_SUCCESS) {
        perror("socketSetTTL: setsockopt");
        errv(0, "%s:%d\n",__FILE__,__LINE__);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int socketSendTo(Socket *ps, const char *server, int port, const char *data, int dataLen) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof serv_addr);


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = ntohs(port);
    if (inet_aton(server, &serv_addr.sin_addr) == 0) {
        errv(0, "inet_aton: %s, %s:%n\n" , strerror(errno), __FILE__, __LINE__);
        return EXIT_FAILURE;
    }

    if (sendto(ps->socket, data, dataLen, 0, (const struct sockaddr *) &serv_addr, sizeof (serv_addr)) != dataLen) {
        perror("socketSendTo: sendto");
        errv(0, "can't send to %s:%d\n", server, port);
        return EXIT_FAILURE;

    }
    strerror(errno);
    
    ps->bytesOut += dataLen;

    return EXIT_SUCCESS;

}



int socketRecvFrom(Socket *ps, char *server, int *port, char *data, int *dataLen) {
   
    struct sockaddr_in from_addr;
    int numRead;
    memset(&from_addr, 0, sizeof from_addr);

    socklen_t recvLen = sizeof from_addr;
    if ((numRead = recvfrom(ps->socket, data, *dataLen, 0, (struct sockaddr *) &from_addr, &recvLen)) < 0) {
        errv(0, "%s %s:%s:%d\n", strerror(errno), __FUNCTION__,__FILE__,__LINE__);
        return EXIT_FAILURE;

    }
    *dataLen = numRead;
    ps->bytesIn += numRead;

    return EXIT_SUCCESS;

}

int socketRelease(Socket *ps) {
    if (close(ps->socket) == 0) {
        perror("close");
        return -1;
    }
    free(ps);
    return 0;
}


#endif
