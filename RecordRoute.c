//#include <sys/inet.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NROUTES 9
int main()
{
  int s;
  int optlen=0;
  struct sockaddr_in target;
  int res;
  char rspace[3+4*NROUTES+1];

  char sendbuf[]= "help me";

  target.sin_family = AF_INET;
  target.sin_port=htons(35000);
  inet_aton("23.56.103.214",&target.sin_addr);


  s = socket(AF_INET, SOCK_DGRAM, 0);

  if (s<0)
    perror("socket");

  memset(rspace, 0, sizeof(rspace));
  rspace[0] = IPOPT_NOP;
  rspace[1+IPOPT_OPTVAL] = IPOPT_RR;
  rspace[1+IPOPT_OLEN] = sizeof(rspace)-1;
  rspace[1+IPOPT_OFFSET] = IPOPT_MINOFF;
  optlen=40;
b  if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, rspace, sizeof(rspace))<0) {
      perror("record route\n");
      exit(2);
    }


  res = sendto(s, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&target, sizeof(struct sockaddr_in));
  perror("sendto: ");

}
