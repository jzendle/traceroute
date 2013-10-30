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

extern int icmpGetTypeFromIpFrame(char *ipFrame) {

  struct iphdr *iphdr = (struct iphdr *) ipFrame;
  struct icmphdr *icmp = NULL;


  assert(iphdr != 0);

  /* do some sanity checks */

  if ( iphdr->version != 4 && iphdr->version != 6 ) {
    errv(0,"ip version mismatch: %d\n", iphdr->version);
    return EXIT_FAILURE;
  }

  /* check that this is an icmp msg */
  if ( iphdr->protocol !=  IPPROTO_ICMP ) {
    errv(0,"ip protocol mismatch: %d\n" , iphdr->protocol );
    return EXIT_FAILURE;
  }

  /* find offset to first byte of icmp which happens to be the msg type */
  icmp = (struct icmphdr *)&ipFrame[iphdr->ihl * 4];

  /* use char * to allow nice bytewise pointer arithmetic - been a long time */
  char *tmp = (char *) icmp;
  int type = icmp->type;
  int code = icmp->code;

  printf("icmp type: %d code: %d\n",type, code);

  /* lets find the offending port (should be our port) if we have received 'port unreachable' */
  
  if ( type == ICMP_DEST_UNREACH && code == ICMP_PORT_UNREACH ) {
    /* offending ip header should follow icmp header */
    struct iphdr *echoIphdr =  (struct iphdr *) (tmp + sizeof (struct icmphdr));
    /* once again, use char * for nice pointer math */
    tmp = (char *) echoIphdr;
    /* offending udp message should follow this ip header */
    struct udphdr *badUdp = ( struct udphdr * ) (tmp + echoIphdr->ihl * 4);
  
    printf ("dest port %d\n" , ntohs(badUdp->dest));
    printf ("source port %d\n" , ntohs(badUdp->source));

  }
  
  return type;

}



#endif
