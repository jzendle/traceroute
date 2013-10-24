#include "traceroute.h"

#include "Socket.h"
#include "Logger.h"
#include "Icmp.h"
#include "Timer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* forward decl */
int socketSendRecvNTimesFrom(Socket *send, Socket *recv,
        int nTimes, const char *srvAddress, int srvPort, const char *msg);

/*
 *  Perform a Traceroute to Supplied Address
 */

int perform_traceroute(const char * address) {
    Socket *udp;
    Socket *icmp;
    int unreachablePort = 65433;
    const char *msg = "message in a bottle";

    printf("tracing route to '%s'\n", address);

    if (socketCreate(&icmp, 0, 0, RAW) != EXIT_SUCCESS) {
        errv(1, "unable to create raw socket for %s\n", address);
        return EXIT_FAILURE;
    }

    if (socketCreate(&udp, 0, 0, DATAGRAM) != EXIT_SUCCESS) {
        errv(1, "unable to create udp socket for %s\n", address);
        return EXIT_FAILURE;
    }

    /* allow for timeouts from noncooperative devices*/
    socketSetTimeout(icmp, 3);

    int msgType = 0;
    int ttl = 0;
    while (msgType != ICMP_PORT_UNREACH && ttl < 64) {

        socketSetTTL(udp, ++ttl);

        printf("%d ", ttl);

        msgType = socketSendRecvNTimesFrom(udp, icmp, 3, address, unreachablePort, msg);

        /* flush stdout */
        printf(" %d \n", msgType);

    }
    socketRelease(icmp);
    socketRelease(udp);
    return EXIT_SUCCESS;
}

int socketSendRecvNTimesFrom(Socket *send, Socket *recv, int count, const char *srvAddress, int srvPort, const char *msg) {

    char packet[4096];
    char server[1024];
    int printedServer = 0;
    int icmpMsgType = -1;


    int i;
    for (i = 0; i < count; i++) {
        struct timeval before;
        struct timeval after;

        gettimeofday(&before, 0);

        if (socketSendTo(send, srvAddress, srvPort, msg, strlen(msg)) != EXIT_SUCCESS) {
            errv(0, "can't send to host %s\n", srvAddress);
            return EXIT_FAILURE;
        }

        int len = sizeof packet;
        if (socketRecvFrom(recv, server, 0, packet, &len) == EXIT_SUCCESS) {
            gettimeofday(&after, 0);
            if (!printedServer) {
                printf(" %s ", server);
                printedServer = !printedServer;
            }
            printf (" %4.3fms ", timeDiffMillis(&after, &before));
            icmpMsgType = icmpGetTypeFromIpFrame(packet);
        } else {
            printf(" * ");
            fflush(stdout);
        }
    }
    
       fflush(stdout);

    return (icmpMsgType == -1 ? EXIT_FAILURE : icmpMsgType);

}


