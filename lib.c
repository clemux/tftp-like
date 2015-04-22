#include "lib.h"
#include "md5.h"
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>




int S_openAndBindSocket(int local_port, int domain) {
    int sockfd;
    struct sockaddr_storage sock_addr;

    if ((sockfd = S_openSocket(domain)) < 0)
        return -1;

    // préparation de la structure de l'adresse
    memset((char *)&sock_addr, 0, sizeof(sock_addr));

    if(domain == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *) &sock_addr;
        addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
        addr_in->sin_port = htons(local_port);
        addr_in->sin_family = domain;
    } else if (domain == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *) &sock_addr;
        addr_in6->sin6_addr = in6addr_any;
        addr_in6->sin6_port = htons(local_port);
        addr_in6->sin6_family = domain;
    } else {
        close(sockfd);
        return -1;
    }
        

    // on attache localement le socket au port `local_port`
    if (bind(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) { 
        perror("Bind ");
        return SOCK_BINDING_FAILED;
    }

    return sockfd;
}

int S_openSocket(int domain) {
    int sockfd;
    // création du socket
    if ((sockfd = socket(domain, SOCK_DGRAM , 0)) < 0) {
        perror("Creation du socket ");
        return SOCK_CREATION_FAILED;
    }

    return sockfd;
}

int S_distantAddress(char *IP_address, int port,
                     struct sockaddr **dist_addr, int domain) {

    struct sockaddr_storage *addr = malloc(sizeof(struct sockaddr_storage));
    *dist_addr = (struct sockaddr *) addr;

    memset((char *)&addr, 0, sizeof(*addr));

    if(domain == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;
        if (!inet_pton(domain, IP_address, &(addr_in->sin_addr))) {
            printf("Erreur: mauvaise adresse %s\n", IP_address);
            return -1;
        }
        addr_in->sin_port = htons(port);

    } else if (domain == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *) addr;
        if (!inet_pton(domain, IP_address, &(addr_in6->sin6_addr))) {
            printf("Erreur: mauvaise adresse %s\n", IP_address);
            return -1;
        }

        addr_in6->sin6_port = htons(port);

    } else {
        fprintf(stderr, "Domaine %d inconnu\n", domain);
        return -1;
    }

    return 0;
}

int S_receiveMessage(int sockfd, struct sockaddr *dist_addr,
                     unsigned char *msg, int length) {
    struct sockaddr_storage *addr = (struct sockaddr_storage *) dist_addr;
    socklen_t addrlen = sizeof(*addr);
    int nb = recvfrom(sockfd, msg, length, 0, dist_addr, &addrlen);
    if (nb < 0) {
        perror("recvfrom ");
        return SOCK_RECV_FAILED;
    }
    return nb;
}

int S_sendMessage (int sockfd, struct sockaddr *dist_addr, 
                   unsigned char *msg, int length) {

    int nbytes; // nombre d'octets envoyés
    struct sockaddr_storage *addr = (struct sockaddr_storage *) dist_addr;

    
    nbytes = sendto(sockfd, msg, length, 0, dist_addr, sizeof(*addr));
    if (nbytes < 0) {
        perror("sendto");
        return SOCK_SENDTO_FAILED;
    }
    return 0;
}

// On pourrait utiliser atoi, puisque le port 0 est invalide...
int string2port(char* s) {
    long int port;
    char *endptr = NULL;
    errno = 0;
    port = strtol(s, &endptr, 10);
    if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN))
        || (errno != 0 && port == 0)) {
            perror("strtol");
            return INVALID_PORT_ERROR;
    }
    
    if (port == 0 || port > USHRT_MAX  || (endptr == s)) {
        return INVALID_PORT_ERROR;
    }
    return port;
}

char* compute_md5(FILE *file) {
    MD5_CTX context;
    uint8_t buf[FILE_BUF_SIZE];
    uint8_t result[16];
    static char result_str[33];
    int i;
    int nbytes;

    MD5_Init(&context);
    fseek(file, 0, SEEK_SET);
    while ((nbytes = fread(buf, 1, FILE_BUF_SIZE, file)) > 0) {
        MD5_Update(&context, buf, nbytes);
    }

    MD5_Final(result, &context);

    for (i = 0; i < 16; i++)
        sprintf(result_str+(i*2), "%.2x", result[i]);
    result_str[32] = '\0';
    return result_str;

    

}
    
