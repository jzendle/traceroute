#ifndef INETDB_H
#include "InetDb.h"
#include "Logger.h"
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

const char *hstrerror(int err);

int inet_aton(const char *__cp, struct in_addr *__inp);

/* 
 * out: sockaddr_in *addr.sin_addr will be populated.
 * out: resolvedName will be populated with either inet addr or dotted decimal
 */
int inetDb_ResolveAddress(const char *server, struct in_addr *addr, char *resolvedName, int resolvedLen) {
    struct hostent *hp = NULL;
    int ret = EXIT_SUCCESS;

    assert(server != NULL);

    /* assume correct dotted decimal passed in for now */
    strncpy(resolvedName, server, resolvedLen);

    hp = gethostbyname(server);
    if (!hp) { /* didn't find in dns so assume address is in dotted decimal format */
        log_errv(0, "coudn't find (%s) in dns\n", server);
        if (inet_aton(server, addr) == 0) {
            /* exit */
            log_errv(0, "unable to resolve address (%s)\n", server);
            ret = EXIT_FAILURE;
        }
    } else { /* found in dns, use first element of address of list */
        memcpy(addr, hp->h_addr_list[0], 4);
        /* override our earlier assumption */
        strncpy(resolvedName, inet_ntoa(*addr), resolvedLen);
    }

    return ret;

}

int inetDb_GetHostForAddress(const struct sockaddr_in *addr,
        char *unresolvedName, int unresolvedLen,
        char *resolvedName, int resolvedLen) {

    strncpy(unresolvedName, inet_ntoa(addr->sin_addr), unresolvedLen);
    struct hostent *hp = gethostbyaddr(&addr->sin_addr, sizeof addr->sin_addr, addr->sin_family);
    strncpy(resolvedName, hp == NULL ? unresolvedName : hp->h_name, resolvedLen);

    return EXIT_SUCCESS;

}


#endif

