#include "lib.h"
#include "md5.h"
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
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
    struct sockaddr_in *addr_in = malloc(sizeof(struct sockaddr_in));

    memset((char *)addr_in, 0, sizeof(*addr_in));
    addr_in->sin_family = domain;
    addr_in->sin_port = htons(port);
    if (!inet_aton(IP_address, &addr_in->sin_addr)) {
        printf("Erreur: mauvaise adresse %s\n", IP_address);
        return -1;
    }

    *dist_addr = (struct sockaddr *) addr_in;

    return 0;
}

int S_receiveMessage(int sockfd, struct sockaddr *dist_addr,
                     unsigned char *msg, int length) {
    socklen_t addrlen = sizeof(*dist_addr);
    int nb = recvfrom(sockfd, msg, length, 0, dist_addr, &addrlen);
    if (nb < 0) {
        perror("recvfrom ");
        return SOCK_RECV_FAILED;
    }
    return nb;
}

int S_sendMessage (int sockfd, struct sockaddr *dist_addr, 
                   unsigned char *msg, int length) {

    struct sockaddr_in *addr_in = (struct sockaddr_in *) dist_addr;
    int nb;
    
    nb = sendto(sockfd, msg, length, 0, dist_addr, sizeof(*addr_in));
    if (nb < 0) {
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
    uint8_t *buf = NULL;
    unsigned char *result = malloc(16 * sizeof(unsigned char));
    char *result_str = malloc(33 * sizeof(char));
    int i;

    MD5_Init(&context);
    fseek(file, 0, SEEK_SET);
    while ((fread(buf, FILE_BUF_SIZE, 1, file)) > 0) {
        MD5_Update(&context, buf, FILE_BUF_SIZE);
    }

    MD5_Final(result, &context);

    for (i = 0; i < 16; i++)
        sprintf(result_str+(i*2), "%.2x", result[i]);
    result_str[32] = '\0';
    return result_str;

    

}
    
