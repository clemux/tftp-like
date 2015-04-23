#include "lib.h"
#include "packet.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>



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
    int i; // compteur de paquets reçus

    // gestion du fichier
    char *filename;
    FILE *file;
    struct stat *stat_p = NULL;
    char *input_buf = NULL;
    
    // calcul de la somme md5
    char *md5sum;
    struct packet_header *md5_header;


    // Parsing des arguments
    if (argc < 4) {
        fprintf(stderr, "Utilisation : %s <version ip> <filename> <local port>\n", 
                argv[0]);
        exit(1);
    }

    if (argv[1][0] == '4') {
        domain = AF_INET;
    } else if (argv[1][0] ==  '6') {
        domain = AF_INET6;
    }


    filename = argv[2];

    if ((local_port = string2port(argv[3])) < 0) {
        fprintf(stderr, "Port invalide : '%s'\n", argv[3]);
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
        
        header = (struct packet_header *) buffer;
        if (header->cmd != 0) {
            fprintf(stderr, "Reçu une commande CHECKSUM avant la fin de la transmission. Abandon");
            exit(1);
        }

        if (send_ack(sockfd, &dist_addr, header->seq, header->cmd) < 0) {
            fprintf(stderr, "Impossible d'envoyer le ACK. Abandon.\n");
            exit(1);
        }

        fwrite(buffer + sizeof(*header), header->payload_size, 1, file);
        
        i++;
    } while (header->payload_size == PAYLOAD_SIZE);
    
    printf("%d paquets reçus\n", i - 1);
    fclose(file);
    if ((file = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Impossible d'ouvrir '%s' en lecture", filename);
    }

    md5sum = compute_md5(file);
    printf("Somme MD5 du fichier reçu: %s\n", md5sum);
    md5_header = (struct packet_header *) buffer;
    memcpy(buffer + sizeof(*md5_header), md5sum, 33);
    if (!send_packet(sockfd, &dist_addr, buffer, sizeof(*md5_header) + 33, 0, 1)) {
        fprintf(stderr, "Pas reçu d'ACK pour le md5. Abandon.\n");
        exit(1);
    }

    return 0;
}
