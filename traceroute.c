#include "traceroute.h"

#include "Socket.h"
#include "Logger.h"
#include "Icmp.h"
#include "Timer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/* forward decl */

extern int inet_aton(__const char *__cp, struct in_addr *__inp);

int socketSendRecvNTimesFrom(Socket *send, Socket *recv, int nTimes, const struct sockaddr_in *sendTo, const char *msg);

/*
 *  Perform a Traceroute to Supplied Address
 */

int perform_traceroute(const char * address) {
    Socket *udp;
    Socket *icmp;
    struct hostent *hp = NULL;
    struct sockaddr_in addr_srv;


    int unreachablePort = 65433;
    const char *msg = "message in a bottle";

    /* build the target address */
    memset(&addr_srv, 0, sizeof addr_srv);
    addr_srv.sin_port = ntohs(unreachablePort);
    addr_srv.sin_family = AF_INET;

    hp = gethostbyname(address);
    if (hp == NULL) { /* didn't find in dns */
        log_errv(0, "coudn't find (%s) in dns\n", address);
        if (inet_aton(address, (struct in_addr *) &addr_srv.sin_addr) == 0) {
            log_errv(1, "unable to resolve address (%s)\n", address);
        }
    } else { /* found in dns, use first element of address of list */
        memcpy(&addr_srv.sin_addr, hp->h_addr_list[0], 4);
    }

    printf("tracing route to %s (%s)\n", address, (hp == NULL ? address : hp->h_name));

    if (socket_Create(&icmp, 0, 0, RAW) != EXIT_SUCCESS) {
        log_errv(1, "unable to create raw socket for %s\n", address);
        return EXIT_FAILURE;
    }

    if (socket_Create(&udp, 0, 0, DATAGRAM) != EXIT_SUCCESS) {
        log_errv(1, "unable to create udp socket for %s\n", address);
        return EXIT_FAILURE;
    }

    /* allow for timeouts from noncooperative devices*/
    socket_SetTimeout(icmp, 3); /* 3 seconds */

    /*socketSetRecordRoute(udp); */

    int msgType = 0;
    int ttl = 0;
    while (msgType != ICMP_PORT_UNREACH && ttl < 64) {

        socket_SetTTL(udp, ++ttl);

        printf("%d ", ttl);

        msgType = socketSendRecvNTimesFrom(udp, icmp, 3, (const struct sockaddr_in *) &addr_srv, msg);

        /* flush stdout */
        printf("\n");
        /* if (msgType == EXIT_FAILURE) {
            log_errv(1, "error in socketSendRecvNTimesFrom. Exiting.\n");
        } */

    }
    socket_Release(icmp);
    socket_Release(udp);
    return EXIT_SUCCESS;
}

int socketSendRecvNTimesFrom(Socket *send, Socket *recv, int count, const struct sockaddr_in *sendTo, const char *msg) {

    char packet[4096];
    int printedServer = 0;
    int icmpMsgType = -1;


    int i;
    for (i = 0; i < count; i++) {
        struct timeval before;
        struct timeval after;
        struct sockaddr_in recvFrom;
        
        memset(&recvFrom, 0, sizeof recvFrom);

        gettimeofday(&before, 0);

        if (socket_SendTo(send, sendTo, msg, strlen(msg)) != EXIT_SUCCESS) {
            /* char server[1024];
               inet_ntoa(server,&sendTo->sin_addr); */
            log_errv(0, "can't send to server %s\n", inet_ntoa(sendTo->sin_addr));
            return EXIT_FAILURE;
        }

        int len = sizeof packet;
        if (socket_RecvFrom(recv, &recvFrom, packet, &len) == EXIT_SUCCESS) {

            gettimeofday(&after, 0);
            float timeMs = timeDiffMillis(&after, &before);

            if (!printedServer) {
                char server [1024];
                 strncpy(server, inet_ntoa(recvFrom.sin_addr), sizeof server);
                struct hostent *hp = gethostbyaddr(&recvFrom.sin_addr,sizeof recvFrom.sin_addr, recvFrom.sin_family);
                printf(" %s (%s) ", server, hp == NULL ? server : hp->h_name);
                printedServer = !printedServer;
            }
            printf(" %4.3fms ", timeMs);

            icmpMsgType = icmp_GetTypeFromIpFrame(packet, sendTo->sin_port);
        } else {
            printf(" * ");
            fflush(stdout);
        }
    }

    fflush(stdout);

    return (icmpMsgType == -1 ? EXIT_FAILURE : icmpMsgType);

}


