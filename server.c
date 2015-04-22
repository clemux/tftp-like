#include "lib.h"
#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAYLOAD_SIZE 512


int main(int argc, char *argv[])
{

    // gestion réseau
    struct sockaddr dist_addr;
    uint8_t *buffer = malloc(PACKET_SIZE * sizeof(uint8_t));
    int sockfd;
    int nbytes;
    int local_port;
    struct packet_header *header;
    int domain = AF_INET; // ipv4 ou ipv6

    // gestion du fichier
    char *filename;
    FILE *file;
    struct stat *stat_p = NULL;
    char *input_buf = NULL;
    
    // calcul de la somme md5
    char *md5sum;


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
        printf("Le fichier %s existe. Continuer? (Y/n) ", filename);
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


    if ((sockfd = S_openAndBindSocket(local_port, domain)) < 0)
        exit(SOCK_BINDING_FAILED);

    do {
        nbytes = S_receiveMessage(sockfd, &dist_addr, buffer, PACKET_SIZE);
        if (nbytes < 0)
            exit(SOCK_RECV_FAILED);
        
        header = (struct packet_header *)buffer;
        fwrite(buffer + sizeof(*header), header->payload_size, 1, file);
    } while (header->payload_size == PAYLOAD_SIZE);
    
    printf("%d paquets reçus\n", header->seq);
    printf("Calcul du md5...\n");
    fclose(file);
    if ((file = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Impossible d'ouvrir '%s' en lecture", filename);
    }

    md5sum = compute_md5(file);
    printf("%s\n", md5sum);

    S_sendMessage(sockfd, &dist_addr, (unsigned char *) md5sum,
                  strlen(md5sum) * sizeof(unsigned char) + 1);

    return 0;
}
