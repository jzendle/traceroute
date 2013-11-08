/* 
 * File:   InetDb.h
 */

#ifndef INETDB_H
#define	INETDB_H

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int inetDb_ResolveAddress(const char *server, struct in_addr *addr, char *resolvedName, int resolvedLen);

int inetDb_GetHostForAddress(const struct sockaddr_in *addr,
        char *unresolvedName, int unresolvedLen,
        char *resolvedName, int resolvedLen) ;

#endif	/* INETDB_H */

