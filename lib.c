#include "lib.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>




int S_openAndBindSocket(int local_port) {
    int domain = AF_INET;
    int sockfd;
    struct sockaddr_in sock_addr;

    if ((sockfd = S_openSocket()) < 0)
        return -1;

    // préparation de la structure de l'adresse
    memset((char *)&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = domain;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(local_port);

    // on attache localement le socket au port `local_port`
    if (bind(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) { 
        perror("Bind ");
        return SOCK_BINDING_FAILED;
    }

    return sockfd;
}

int S_openSocket(void) {
    int sockfd;
    int domain = AF_INET;
    // création du socket
    if ((sockfd = socket(domain, SOCK_DGRAM , 0)) < 0) {
        perror("Creation du socket ");
        return SOCK_CREATION_FAILED;
    }

    return sockfd;
}

int S_distantAddress(char *IP_address, int port,
                     struct sockaddr **dist_addr) {

    int domain = AF_INET;
    struct sockaddr_in addr_in;
    memset((char *)&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = domain;
    addr_in.sin_port = port;
    if (!inet_aton(IP_address, &addr_in.sin_addr)) {
        printf("Erreur: mauvaise adresse %s\n", IP_address);
        return -1;
    }

    *dist_addr = (struct sockaddr *) &addr_in;

    return 0;
}

int S_receiveMessage(int sockfd, struct sockaddr *dist_addr,
                     char *msg, int length) {
    socklen_t addrlen = sizeof(*dist_addr);
    recvfrom(sockfd, msg, length, 0, dist_addr, &addrlen);
    return 0;
}

int S_sendMessage (int sockfd, struct sockaddr *dist_addr, 
                   char *msg, int length) {

    sendto(sockfd, msg, length, 0, dist_addr, sizeof(*dist_addr));
    return 0;
}

