#include "traceroute.h"

#include "Socket.h"
#include "Logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
        Perform a Traceroute to Supplied Address
 */
int perform_traceroute(const char * address) {
    Socket *udp;
    Socket *icmp;
    const char *msg = "message in a bottle";
    char packet[4096];
    /*    int udpPort = 30000; */


    printf("tracing route to '%s'\n", address);


    if (socketCreate(&icmp, 0, 0, RAW) != EXIT_SUCCESS) {
        errv(1, "unable to create raw socket for %s\n", address);
        return EXIT_FAILURE;
    }

    if (socketCreate(&udp, 0, 0, DATAGRAM) != EXIT_SUCCESS) {
        errv(1, "unable to create udp socket for %s\n", address);
        return EXIT_FAILURE;
    }

    socketSetTTL(udp, 1);
    
    socketSetTimeout(icmp,2);
    
    
    if ( socketSendTo(udp,address, 33000, msg,strlen(msg)) != EXIT_SUCCESS ) {
        errv(0, "can't send to host %s\n", address);
        return EXIT_FAILURE;
    }
    
    int len = sizeof packet;
    socketRecvFrom(icmp, 0,0, packet, &len);
    
    infov("received %d bytes\n", len);
    
    

    socketRelease(icmp);
    socketRelease(udp);

    return EXIT_SUCCESS;
}
