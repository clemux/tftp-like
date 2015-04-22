#include "lib.h"
#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define PAYLOAD_SIZE 512
#define FILE_READ_ERROR 255
#define FILE_OPEN_ERROR 254

int main(int argc, char* argv[])
{
    struct sockaddr *dist_addr; 

    uint8_t *buffer = malloc(BUFSIZE * sizeof(uint8_t));
    int sockfd;
    struct packet_header *header;
    FILE *file;
    char *filename;
    int local_port, distant_port;
    char *distant_host;

    int nbytes;

    // Parsing des arguments
    if (argc < 4) {
        fprintf(stderr, "Utilisation : %s <filename> <distant host> <distant port> ",
                argv[0]);
        fprintf(stderr, "[<local port>]\n");
        exit(1);
    }

    filename = argv[1];
    distant_host = argv[2];
    if ((distant_port = string2port(argv[3])) < 0) {
        fprintf(stderr, "Port invalide: %s\n", argv[3]);
        exit(INVALID_PORT_ERROR);
    }

    if (argc > 4) {
        local_port = string2port(argv[2]);

        if (local_port < 0) {
            fprintf(stderr, "Port invalide : %s\n", argv[2]);
            exit(1);
        }

        if ((sockfd = S_openAndBindSocket(local_port)) < 0)
            exit(SOCK_BINDING_FAILED);
    }

    else {
        if ((sockfd = S_openSocket()) < 0)
            exit(SOCK_CREATION_FAILED);
    }
  


    if ((file = fopen(filename, "r")) == NULL) {
        perror("ouverture fichier ");
        exit(FILE_OPEN_ERROR);
    }
        

    if (S_distantAddress(distant_host, distant_port, &dist_addr) < 0) {
        fprintf(stderr, "erreur adresse\n");
        exit(ADDRESS_ERROR);
    }
        

    printf("Sending...\n");
    
    header = (struct packet_header *) buffer;
    header->seq = 0;
    do {
        nbytes = fread(buffer + sizeof(struct packet_header), 1, PAYLOAD_SIZE, file);
        if (nbytes == 0 && !feof(file)) {
            fprintf(stderr, "Erreur de lecture: %s", filename);
            exit(FILE_READ_ERROR);
        }            
            
        printf("Lu %d du fichier\n", nbytes);
        header->payload_size = nbytes;
        if (S_sendMessage(sockfd, dist_addr, buffer,
                          sizeof(header) + header->payload_size) < 0)
            exit(SOCK_SENDTO_FAILED);

        header->seq++;

    } while (nbytes == PAYLOAD_SIZE);

    printf("%d paquets envoyés\n", header->seq);
    
    return 0;
}
