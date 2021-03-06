#include "client.h"
#include "lib.h"
#include "md5.h"
#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[])
{
    struct sockaddr *dist_addr;

    uint8_t *buffer = malloc(PACKET_SIZE * sizeof(uint8_t));
    int sockfd;
    struct packet_header *header;
    FILE *file;
    char *filename;
    int local_port, distant_port;
    char *distant_host;

    // ipv4 ou ipv6?
    int domain = AF_INET;

    int nbytes;

    // calcul md5
    char *md5sum;

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
  


    if ((file = fopen(filename, "r")) == NULL) {
        perror("ouverture fichier ");
        exit(FILE_OPEN_ERROR);
    }
        

    if (S_distantAddress(distant_host, distant_port, &dist_addr, domain) < 0) {
        fprintf(stderr, "erreur adresse\n");
        exit(ADDRESS_ERROR);
    }
        

    printf("Sending...\n");
    
    header = (struct packet_header *) buffer;
    header->seq = 0;
    do {
        nbytes = fread(buffer + sizeof(struct packet_header), 1, PAYLOAD_SIZE, file);
        if (nbytes == 0 && !feof(file)) {
            fprintf(stderr, "Erreur de lecture: '%s'", filename);
            exit(FILE_READ_ERROR);
        }            
            
        header->payload_size = nbytes;
        if (S_sendMessage(sockfd, dist_addr, buffer,
                          sizeof(*header) + header->payload_size) < 0)
            exit(SOCK_SENDTO_FAILED);

        header->seq++;

    } while (nbytes == PAYLOAD_SIZE);

    printf("%d paquets envoyés\n", header->seq - 1);
    printf("Attente du md5...\n");
    if ((S_receiveMessage(sockfd, dist_addr, buffer, 33) < 0)) {
        fprintf(stderr, "Réception de la somme md5 échouée\n");
        exit(1);
    }
    printf("Somme MD5 reçue : %s\n", buffer);

    printf("Calcul du MD5 local...\n");

    md5sum = compute_md5(file);
    printf("MD5 local : %s\n", md5sum);

    if (strcmp((char*) md5sum, (char*) buffer) == 0)
        printf("MD5 OK !\n");
    else
        printf("MD5 différent... erreur de transmission !\n");
    
    
    return 0;
}
