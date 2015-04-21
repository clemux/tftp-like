#include "lib.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1024

int main()
{
    struct sockaddr *dist_addr; 
    char buffer[BUFSIZE];
    int sockfd = S_openSocket();
    printf("%d\n", sockfd);
    if (S_distantAddress("127.0.0.1", 30000, &dist_addr) < 0)
        printf("erreur adresse\n");

    printf("Sending...\n");
    if (S_sendMessage(sockfd, dist_addr, "coucou", 7) < 0)
        printf("erreur envoi\n");
    
    if (S_receiveMessage(sockfd, dist_addr, buffer, BUFSIZE) < 0)
        printf("erreur reception\n");
    
    printf("Reponse reçue=%s\n",buffer);
    return 0;
}
