#include "lib.h"
#include <stdio.h>
#include <string.h>

#define BUFSIZE 1024
int main()
{
    struct sockaddr *dist_addr;
    char buffer[BUFSIZE];
    int sockfd = S_openAndBindSocket(30000);
    int nb_got;
    nb_got = S_receiveMessage(sockfd, dist_addr, buffer, BUFSIZE);
    printf("Reçu: %d bytes\n", nb_got);
    printf("Message reçu=%s\n",buffer);
    S_sendMessage(sockfd, dist_addr, "Hello", 6);
    return 0;
}
l
