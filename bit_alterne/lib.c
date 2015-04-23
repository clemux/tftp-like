#include "lib.h"
#include "md5.h"
#include "packet.h"
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
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

    memset((char *)addr, 0, sizeof(*addr));

    if(domain == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;
        if (!inet_pton(domain, IP_address, &(addr_in->sin_addr.s_addr))) {
            printf("Erreur: mauvaise adresse %s\n", IP_address);
            return -1;
        }
        addr_in->sin_port = htons(port);
        addr_in->sin_family = domain;

    } else if (domain == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *) addr;

        if (!inet_pton(domain, IP_address, &(addr_in6->sin6_addr.s6_addr))) {
            printf("Erreur: mauvaise adresse %s\n", IP_address);
            return -1;
        }


        addr_in6->sin6_port = htons(port);
        addr_in6->sin6_family = domain;

    } else {
        fprintf(stderr, "Domaine %d inconnu\n", domain);
        return -1;
    }

    return 0;
}

int S_receiveMessage(int sockfd, struct sockaddr *dist_addr,
                     void *msg, int length) {
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
                   void *msg, int length) {

    int nbytes; // nombre d'octets envoyés
    struct sockaddr_storage *addr = (struct sockaddr_storage *) dist_addr;

    
    nbytes = sendto(sockfd, msg, length, 0, dist_addr, sizeof(*addr));
    if (nbytes < 0) {
        perror("sendto");
        return SOCK_SENDTO_FAILED;
    } else {
//        printf("Envoyé %d bytes\n", nbytes);
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

int timeout_ack(int sockfd, long seconds) {
    // 1 si des données ont été reçues, 0 sinon
    struct pollfd fd;
    int retval;

    fd.fd = sockfd;
    fd.events = POLLIN;

    retval = poll(&fd, 1, seconds * 1000);

    if (retval == -1)
        perror("poll ");
    else if (retval) {
        return 1;
    }

    return 0;
}

int send_packet(int sockfd, struct sockaddr *dist_addr, void *buffer,
                int nbytes, uint8_t seq, uint8_t cmd) {
    struct packet_header *header;
    struct packet_header *ack_header = malloc(sizeof(struct packet_header));
    int received_ack;
    int nb_tries = 0;
    int acked = 0;

    header = (struct packet_header *) buffer;
    header->seq = seq;
    header->cmd = cmd;
    header->payload_size = nbytes;

    while (!acked) {
        if (S_sendMessage(sockfd, dist_addr, buffer,
                          sizeof(*header) + header->payload_size) < 0)
            exit(SOCK_SENDTO_FAILED);


        received_ack = timeout_ack(sockfd, TIMEOUT); // on attend un ACK
        
        if (received_ack) {
            if ((S_receiveMessage(sockfd, dist_addr, ack_header,
                                  sizeof(struct packet_header)) < 0)) {
                fprintf(stderr, "Lecture de l'ack a échouée\n");
                exit(1);
            }
            
            if (ack_header->seq == seq && ack_header->cmd == cmd)
                acked = 1;
        }

        // pas très propre, permet de quitter au bout d'un moment
        // et de vérifier quand même le checksum du fichier
        
        nb_tries++;
        if (nb_tries > NB_TRIES) {
            fprintf(stderr, "Abandon après %d essais. Les données seront corrompues :(\n",
                    NB_TRIES);
            return 0;
        
        }
    }

    return 1;
}

int send_ack(int sockfd, struct sockaddr *addr, uint8_t seq, uint8_t cmd) {
    struct packet_header header;
    header.seq = seq;
    header.cmd = cmd;
    header.payload_size = 0;
    if (S_sendMessage(sockfd, addr, &header, sizeof(header)) < 0) {
        fprintf(stderr, "Erreur envoi de l'acquittement\n");
        return -1;
    }
    return 0;
}
