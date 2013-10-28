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

extern int inet_aton (__const char *__cp, struct in_addr *__inp);

int socketSendRecvNTimesFrom(Socket *send, Socket *recv,int nTimes, const struct sockaddr_in *sendTo , const char *msg);

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
  memset ( &addr_srv, 0 , sizeof addr_srv);
  addr_srv.sin_port = ntohs(unreachablePort);
  addr_srv.sin_family = AF_INET;

  hp = gethostbyname(address);
  if ( hp == NULL ) { /* didn't find in dns */
    errv(0, "coudn't find (%s) in dns\n", address); 
    if (inet_aton(address, (struct in_addr *)&addr_srv.sin_addr) == 0) {
      errv(1,"unable to resolve address (%s)\n", address);
    }
  }
  else { /* found in dns, use first element of address of list */
    memcpy(&addr_srv.sin_addr, hp->h_addr_list[0], 4);
  }

  printf("tracing route to %s (%s)\n", address, ( hp == NULL ? address: hp->h_name));

	 if (socketCreate(&icmp, 0, 0, RAW) != EXIT_SUCCESS) {
	   errv(1, "unable to create raw socket for %s\n", address);
	   return EXIT_FAILURE;
	 }

	 if (socketCreate(&udp, 0, 0, DATAGRAM) != EXIT_SUCCESS) {
	   errv(1, "unable to create udp socket for %s\n", address);
	   return EXIT_FAILURE;
	 }

	 /* allow for timeouts from noncooperative devices*/
	 socketSetTimeout(icmp, 3); /* 3 seconds */

	 /*socketSetRecordRoute(udp); */

	 int msgType = 0;
	 int ttl = 0;
	 while (msgType != ICMP_PORT_UNREACH && ttl < 64) {

	   socketSetTTL(udp, ++ttl);

	   printf("%d ", ttl);

	   msgType = socketSendRecvNTimesFrom(udp, icmp, 3, (const struct sockaddr_in *) &addr_srv, msg);

	   /* flush stdout */
	   printf("\n");

	 }
	 socketRelease(icmp);
	 socketRelease(udp);
	 return EXIT_SUCCESS;
}

int socketSendRecvNTimesFrom(Socket *send, Socket *recv, int count, const struct sockaddr_in *sendTo, const char *msg) {

  char packet[4096];
  char server[1024];
  int printedServer = 0;
  int icmpMsgType = -1;


  int i;
  for (i = 0; i < count; i++) {
    struct timeval before;
    struct timeval after;

    gettimeofday(&before, 0);

    if (socketSendTo(send, sendTo, msg, strlen(msg)) != EXIT_SUCCESS) {
      /* char server[1024];
	 inet_ntoa(server,&sendTo->sin_addr); */
      errv(0, "can't send to server %s\n", inet_ntoa( sendTo->sin_addr ));
	return EXIT_FAILURE;
      }

      int len = sizeof packet;
      if (socketRecvFrom(recv, server, 0, packet, &len) == EXIT_SUCCESS) {
	  
	/*      char options [100];
		int szOptions = sizeof options;
		memset(options, 0, szOptions); */

	/* socketGetRecordRoute(recv, options, &szOptions); */

     
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


