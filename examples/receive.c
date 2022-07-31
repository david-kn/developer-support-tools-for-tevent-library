/*
 * It just listens port 321000.
 * Start this program before you launch 'send' and 'subrequest'. This program will receive and print to STDOUT
 * the result.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
   int sockfd, n;
   struct sockaddr_in servaddr, cliaddr;
   socklen_t len;
   char mesg[1000];

   sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port = htons(32100);
   bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd, mesg, 1000, 0, (struct sockaddr *)&cliaddr, &len);
      printf("Received the following:\n");
      mesg[n] = 0;
      printf("-------------------------------------------------------\n");
      printf("%s\n", mesg);
      printf("-------------------------------------------------------\n");
      sleep(1);
   }
}
