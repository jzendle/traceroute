#ifndef ICMP_H
#include "Icmp.h"
#include "Logger.h"

#include <assert.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <stdlib.h>
#include <stdio.h>

int icmp_GetTypeFromIpFrame(char *ipFrame, unsigned short destPort) {

    struct iphdr *iphdr = (struct iphdr *) ipFrame;
    struct icmphdr *icmp = NULL;


    assert(iphdr != 0);

    /* do some sanity checks */

    if (iphdr->version != 4 && iphdr->version != 6) {
        log_errv(0, "ip version mismatch: %d\n", iphdr->version);
        return EXIT_FAILURE;
    }

    /* check that this is an icmp msg */
    if (iphdr->protocol != IPPROTO_ICMP) {
        log_errv(0, "ip protocol mismatch: %d\n", iphdr->protocol);
        return EXIT_FAILURE;
    }

    /* map to the icmp header  */
    icmp = (struct icmphdr *) &ipFrame[iphdr->ihl * 4];

    int type = icmp->type;
    int code = icmp->code;

    /* lets find the offending port (should be our port) if we have received 'port unreachable' */

    if (type == ICMP_DEST_UNREACH && code == ICMP_PORT_UNREACH) {

        /* according to Stevens the original UDP header should be included in the ICMP data */
        /* lets find it and compare our destination port with that of the returned message */
        /* it could be that this ICMP message is referring to a different application (port) */

        /* offending ip header should follow icmp header */
        /* cast icmp to char * for bytewise pointer math  */
        struct iphdr *echoIphdr = (struct iphdr *) ((char *) icmp + sizeof (struct icmphdr));


        /* offending udp message should follow this ip header */
        /* cast echoIpHder to char * for bytewise pointer math */
        struct udphdr *badUdp = (struct udphdr *) ((char *) echoIphdr + echoIphdr->ihl * 4);

        /* ports should match or ICMP msg was not for us */
        if (destPort != badUdp->dest) {
            log_errv(0, "received port unreachable for another port: %d, not %d\n", ntohs(badUdp->dest), ntohs(destPort));
        }

    }

    return type;

}



#endif
