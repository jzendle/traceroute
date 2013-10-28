#ifndef ICMP_H
#include "Icmp.h"
#include "Logger.h"

#include <assert.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <stdio.h>

extern int icmpGetTypeFromIpFrame(char *ipFrame) {

  struct iphdr *frame = (struct iphdr *) ipFrame;

  assert(frame != 0);
  /* do some sanity checks */

  if ( frame->version != 4 && frame->version != 6 ) {
    errv(0,"ip version mismatch: %d\n", frame->version);
    return EXIT_FAILURE;
  }

  /* check that this is an icmp msg */
  if ( frame->protocol !=  IPPROTO_ICMP ) {
    errv(0,"ip protocol mismatch: %d\n" , frame->protocol );
    return EXIT_FAILURE;
  }

  /* find offset to first byte of icmp which happens to be the msg type */
  int msgType = ipFrame[frame->ihl * 4];

  return msgType;

}



#endif
