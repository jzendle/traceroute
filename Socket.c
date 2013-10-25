#ifndef SOCKET_H
#include "Socket.h"
#include "Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

extern const char *hstrerror(int err);


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
    PERROR;
    return EXIT_FAILURE;
  }

  Socket *newSocket = calloc(1, sizeof (Socket));
  if (newSocket == NULL) {
    PERROR;
    return EXIT_FAILURE;
  }

  newSocket->socket = fd;
  newSocket->bytesIn = 0;
  newSocket->bytesOut = 0;

  *ps = newSocket;

  adr_len = sizeof (adr_inet);
  memset(&adr_inet, 0, sizeof (adr_inet));
  adr_inet.sin_family = AF_INET;
  adr_inet.sin_port = ntohs(port);

  if (host == NULL)
    adr_inet.sin_addr.s_addr = ntohl(INADDR_ANY);
  else {
    if (inet_aton(host, &adr_inet.sin_addr) == 0) {
      PERROR;
      return EXIT_FAILURE;
    }

  }

  /* Now bind the address to the socket */
  if (bind(fd,
	   (struct sockaddr *) &adr_inet,
	   adr_len) == -1) {
    PERROR;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

}

int socketListen(Socket *ps, int backlog) {
  return listen(ps->socket, backlog);
}

int socketSetTTL(Socket *ps, int ttl) {
  if (setsockopt(ps->socket, IPPROTO_IP, IP_TTL, &ttl, sizeof (int)) != EXIT_SUCCESS) {
    PERROR;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


int socketSetRecordRoute(Socket *ps) {
  static const int numRoutes = 9;
  char routeSpace[3+4*numRoutes+1];
  int ret = EXIT_SUCCESS;

  memset(routeSpace, 0, sizeof(routeSpace));
  routeSpace[0] = IPOPT_NOP;
  routeSpace[1+IPOPT_OPTVAL] = IPOPT_RR;
  routeSpace[1+IPOPT_OLEN] = sizeof(routeSpace)-1;
  routeSpace[1+IPOPT_OFFSET] = IPOPT_MINOFF;
  
  if (setsockopt(ps->socket, IPPROTO_IP, IP_OPTIONS, routeSpace, sizeof(routeSpace))<0) {
    PERROR;
    ret = EXIT_FAILURE;
  }

  return ret;
}

int socketGetRecordRoute(Socket *ps, char *options, int *pLen) {
 
  int ret = EXIT_SUCCESS;
  if (getsockopt(ps->socket, IPPROTO_IP, IPOPT_RR, options, (socklen_t *)pLen)<0) {
    PERROR;
    ret = EXIT_FAILURE;
  }

  return ret;
}

int socketSetTimeout(Socket *ps, int seconds) {
  struct timeval tv;

  tv.tv_sec = seconds;
  tv.tv_usec = 0;

  if (setsockopt(ps->socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof (struct timeval)) != EXIT_SUCCESS) {
    PERROR;
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
    struct hostent *hp;
    hp = gethostbyname(server);
    if (!hp) {
      /* Report lookup failure */
      fprintf(stderr,
	      "%s: host '%s'\n",
	      hstrerror(h_errno),
	      server);
      return EXIT_FAILURE;

    } else {
      memcpy(&serv_addr.sin_addr, hp->h_addr_list[0], 4);
    }
  }

  if (sendto(ps->socket, data, dataLen, 0, (const struct sockaddr *) &serv_addr, sizeof (serv_addr)) != dataLen) {
    PERROR;
    return EXIT_FAILURE;

  }

  /* update internal counter */
  ps->bytesOut += dataLen;

  return EXIT_SUCCESS;

}

int socketRecvFrom(Socket *ps, char *server, int *port, char *data, int *dataLen) {

  struct sockaddr_in from_addr;
  int numRead;
  memset(&from_addr, 0, sizeof from_addr);

  socklen_t recvLen = sizeof from_addr;
  if ((numRead = recvfrom(ps->socket, data, *dataLen, 0, (struct sockaddr *) &from_addr, &recvLen)) < 0) {
    if (errno != 11) /* Resource temporarily unavailable - timeout */
      PERROR;
    return EXIT_FAILURE;

  }

  strcpy(server, inet_ntoa(from_addr.sin_addr));
  *dataLen = numRead;
  /* update internal counter */
  ps->bytesIn += numRead;

  return EXIT_SUCCESS;

}

int socketRelease(Socket *ps) {
  if (close(ps->socket) != 0) {
    PERROR;
  }
  free(ps);
  return EXIT_SUCCESS;
}


#endif
