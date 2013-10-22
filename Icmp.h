#ifndef ICMP_H
#define ICMP_H

/* 
 * Utility Routines for ICMP messages
 */
#include <netinet/ip_icmp.h> /* include for msg type definitions */

extern int icmpGetTypeFromIpFrame(char *ipFrame);



#endif
