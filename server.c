#include "lib.h"
#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 1024
#define PAYLOAD_SIZE 512

int main(int argc, char *argv[])
{
    struct sockaddr *dist_addr = NULL;
    uint8_t *buffer = malloc(BUFSIZE * sizeof(uint8_t));
    int sockfd;
    int nbytes;
    char *filename;
    FILE *file;
    int local_port;
    struct packet_header header;
    struct stat *stat_p = NULL;
    char *input_buf = NULL;


    // Parsing des arguments
    if (argc < 3) {
        fprintf(stderr, "Utilisation : %s <filename> <local port>\n", 
                argv[0]);
        exit(1);
    }

    filename = argv[1];

    if ((local_port = string2port(argv[2])) < 0) {
        fprintf(stderr, "Port invalide : '%s'\n", argv[2]);
        exit(1);
    }
   
    printf("Écoute sur le port %d...\n", (int)local_port);

    if ((stat(filename, stat_p)) == 0) {
        printf("Le fichier %s exist. Continuer? (Y/n) ", filename);
        read(0, input_buf, 1);
        if (input_buf[0] == 'n' || input_buf[0] == 'N') {
            printf("Ok, on annule tout.\n");
            exit(0);
        }
    }

    if ((file = fopen(filename, "w")) == NULL) {
        perror("fopen");
        fprintf(stderr, "Impossible d'ouvrir %s.\n", filename);
    }


    if ((sockfd = S_openAndBindSocket(local_port)) < 0)
        exit(SOCK_BINDING_FAILED);

    do {
        nbytes = S_receiveMessage(sockfd, dist_addr, buffer, BUFSIZE);
        if (nbytes < 0)
            exit(SOCK_RECV_FAILED);
        
        memcpy(&header.seq, buffer, sizeof(header.seq));
        memcpy(&header.payload_size, buffer + sizeof(header.seq),
               sizeof(header.payload_size));
        printf("Reçu: %d bytes\n", nbytes);
        printf("Paquet %d reçu: %d octets\n", header.seq,
               header.payload_size);
        fwrite(buffer + sizeof(header), header.payload_size, 1, file);
    } while (header.payload_size == PAYLOAD_SIZE);

    return 0;
}
