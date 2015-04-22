#include "lib.h"
#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define PAYLOAD_SIZE 512

int main()
{
    struct sockaddr *dist_addr; 
    uint8_t *buffer = malloc(BUFSIZE * sizeof(uint8_t));
    int sockfd = S_openSocket();
    uint8_t *payload = malloc(PAYLOAD_SIZE * sizeof(uint8_t));
    struct packet_header header;
    FILE *file = fopen("chat-peau.jpg", "r");
    int nbytes;

    if (S_distantAddress("127.0.0.1", 30000, &dist_addr) < 0)
        printf("erreur adresse\n");

    printf("Sending...\n");
    
    /* préparation d'un paquet */

    header.seq = 0;
    while (1) {
        header.seq++;
        nbytes = fread(payload, PAYLOAD_SIZE, 1, file);
        printf("Lu %d du fichier\n", nbytes * PAYLOAD_SIZE);
        header.payload_size = nbytes * PAYLOAD_SIZE;
        memcpy(buffer, &header, sizeof(header));
        memcpy(buffer + sizeof(header), payload, sizeof(*payload));


        S_sendMessage(sockfd, dist_addr, buffer,
                      sizeof(header) + header.payload_size);
        if (nbytes == 0) {
            break;
        }
    }
    
    return 0;
}
