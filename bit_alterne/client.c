#include "client.h"
#include "lib.h"
#include "md5.h"
#include "packet.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



int main(int argc, char* argv[])
{
    struct sockaddr *dist_addr = malloc(sizeof(struct sockaddr));

    uint8_t *buffer = malloc(PACKET_SIZE * sizeof(uint8_t));
    int sockfd;

    FILE *file;
    char *filename;
    int local_port, distant_port;
    char *distant_host;

    // ipv4 ou ipv6?
    int domain = AF_INET;

    int nbytes;

    // calcul md5
    char *md5sum_local;
    char md5sum_remote[33];
    struct packet_header *header; // pour la reception du md5 distant

    // boucle d'envoi du fichier

    int i; // compteur d'envoi de paquets

    // Parsing des arguments
    if (argc < 5) {
        fprintf(stderr, "Utilisation : %s <version IP> <filename> <distant host> <distant port> "
                "[<local port>]\n", argv[0]);
        exit(1);
    }
    
    if (argv[1][0] == '4') {
        domain = AF_INET;
    } else if (argv[1][0] ==  '6') {
        domain = AF_INET6;
    }
    
    filename = argv[2];
    distant_host = argv[3];
    if ((distant_port = string2port(argv[4])) < 0) {
        fprintf(stderr, "Port invalide: %s\n", argv[4]);
        exit(INVALID_PORT_ERROR);
    }

    if (argc > 5) {
        local_port = string2port(argv[5]);

        if (local_port < 0) {
            fprintf(stderr, "Port invalide : %s\n", argv[5]);
            exit(1);
        }

        if ((sockfd = S_openAndBindSocket(local_port, domain)) < 0)
            exit(SOCK_BINDING_FAILED);
    }

    else {
        if ((sockfd = S_openSocket(domain)) < 0)
            exit(SOCK_CREATION_FAILED);
    }
  
    // socket non bloquant
//    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
//        perror("passer socket en non blockant : ");
//        exit(1);
//    }


    if ((file = fopen(filename, "r")) == NULL) {
        perror("ouverture fichier ");
        exit(FILE_OPEN_ERROR);
    }
        

    if (S_distantAddress(distant_host, distant_port, &dist_addr, domain) < 0) {
        fprintf(stderr, "erreur adresse\n");
        exit(ADDRESS_ERROR);
    }
        

    printf("Sending...\n");
    

    i = 0;
    do {
        nbytes = fread(buffer + sizeof(struct packet_header), 1, PAYLOAD_SIZE, file);
        if (nbytes == 0 && !feof(file)) {
            fprintf(stderr, "Erreur de lecture: '%s'", filename);
            exit(FILE_READ_ERROR);
        }            
        if (!send_packet(sockfd, dist_addr, buffer, nbytes, i % 2, 0)) {
            printf("Envoi du paquet %d échoué\n", i);
            break;
        }

        i++;

    } while (nbytes == PAYLOAD_SIZE);

    printf("%d paquets envoyés\n", i - 1);
    printf("Attente du md5...\n");
    if ((S_receiveMessage(sockfd, dist_addr, buffer, sizeof(struct packet_header) + 33) < 0)) {
        fprintf(stderr, "Réception de la somme md5 échouée\n");
        exit(1);
    }

    header = (struct packet_header *) buffer;

    if (send_ack(sockfd, dist_addr, header->seq, header->cmd) < 0) {
        fprintf(stderr, "Impossible d'envoyer le ACK. Abandon.\n");
        exit(1);
    }

    // somme md5 : 32 octets ASCII + '\0' de fin de chaine
    memcpy(md5sum_remote, buffer + sizeof(struct packet_header), 33);
    printf("Somme MD5 distante : %s\n", buffer);

    md5sum_local = compute_md5(file);
    printf("Somme MD5 locale : %s\n", md5sum_local);

    if (strcmp(md5sum_local, md5sum_remote) == 0)
        printf("MD5 OK !\n");
    else
        fprintf(stderr, "MD5 différent... erreur de transmission !\n");
        
    return 0;
}
