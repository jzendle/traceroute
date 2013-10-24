#include "traceroute.h"

#include "Socket.h"
#include "Logger.h"
#include "Icmp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int socketRecvNtimesFrom(Socket *ps, int count);

/*
 *  Perform a Traceroute to Supplied Address
 */

int perform_traceroute(const char * address) {
    Socket *udp;
    Socket *icmp;
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


        if (socketSendTo(udp, address, 33000, msg, strlen(msg)) != EXIT_SUCCESS) {
            errv(0, "can't send to host %s\n", address);
            return EXIT_FAILURE;
        }

        msgType = socketRecvNtimesFrom(icmp, 3);

        /* flush stdout */
        printf(" %d \n", msgType);

    }
    socketRelease(icmp);
    socketRelease(udp);
    return EXIT_SUCCESS;
}

int socketRecvNtimesFrom(Socket *ps,int count) {

    char packet[4096];
    char server[1024];
    

    int i;
    for (i = 0; i < count; i++) {

        int len = sizeof packet;
        if (socketRecvFrom(ps, server, 0, packet, &len) == EXIT_SUCCESS) {
            printf(" (%s) ", server);
            return icmpGetTypeFromIpFrame(packet);
        } else {
            printf(" * ");
            fflush(stdout);
        }
    }

    return EXIT_FAILURE;
}


