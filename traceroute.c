#include "traceroute.h"

#include "Socket.h"
#include "Logger.h"
#include "Icmp.h"
#include "Timer.h"
#include "InetDb.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


/* forward decl */

int socketSendRecvNTimesFrom(Socket *send, Socket *recv, int nTimes, const struct sockaddr_in *sendTo, const char *msg);

/*
 *  Perform a Traceroute to Supplied Address
 */

int perform_traceroute(const char * address) {
    Socket *udp;
    Socket *icmp;
    struct sockaddr_in addr_srv;

    int unreachablePort = 65433;
    char resolvedAddress[512];
    const char *msg = "message in a bottle"; /* sending this out */

    /* build the target address */
    memset(&addr_srv, 0, sizeof addr_srv);
    addr_srv.sin_port = ntohs(unreachablePort);
    addr_srv.sin_family = AF_INET;

    /* fill in addr_srv.sin_addr */
    if ( inetDb_ResolveAddress(address, &addr_srv.sin_addr ,resolvedAddress,sizeof resolvedAddress) != EXIT_SUCCESS) {
        log_err(1, "exiting.\n");
    }
   
    printf("tracing route to %s (%s)\n", address, resolvedAddress);

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
    while (msgType != ICMP_PORT_UNREACH && ttl < 32) {

        socket_SetTTL(udp, ++ttl);

        printf("%d ", ttl);

        msgType = socketSendRecvNTimesFrom(udp, icmp, 3, (const struct sockaddr_in *) &addr_srv, msg);

        /* flush stdout */
        printf("\n");

    }
    
    socket_Release(icmp);
    socket_Release(udp);
    return EXIT_SUCCESS;
}

int socketSendRecvNTimesFrom(Socket *send, Socket *recv, int count, const struct sockaddr_in *sendTo, const char *msg) {

    char packet[4096];
    int printedServer = 0; /* one-shot */
    int icmpMsgType = -1;


    int i;
    for (i = 0; i < count; i++) {
        struct timeval before;
        struct timeval after;
        struct sockaddr_in recvFrom;
        
        memset(&recvFrom, 0, sizeof recvFrom);

        gettimeofday(&before, 0);

        if (socket_SendTo(send, sendTo, msg, strlen(msg)) != EXIT_SUCCESS) {
            /* must be bad address - exit */
            log_errv(1, "can't send to server %s\n", inet_ntoa(sendTo->sin_addr));
            return EXIT_FAILURE; /* unreachable */
        }

        int len = sizeof packet;
        if (socket_RecvFrom(recv, &recvFrom, packet, &len) == EXIT_SUCCESS) {

            gettimeofday(&after, 0);
            float timeMs = timeDiffMillis(&after, &before);

            if (!printedServer) {
                char unresolved [1024];
                char resolved [1024];
                
                inetDb_GetHostForAddress(&recvFrom, unresolved, sizeof unresolved, resolved, sizeof resolved);
                
                printf(" %s (%s) ", unresolved, resolved);
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


