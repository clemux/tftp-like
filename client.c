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
    if (argc < 1)
        return 1;

    uint8_t *buffer = malloc(BUFSIZE * sizeof(uint8_t));
    int sockfd = S_openSocket();
    struct packet_header *header;
    FILE *file;
    char *filename = argv[1];

    int nbytes;

    if ((file = fopen(filename, "r")) == NULL) {
        perror("ouverture fichier ");
        return FILE_OPEN_ERROR;
    }
        

    if (S_distantAddress("127.0.0.1", 30000, &dist_addr) < 0)
        printf("erreur adresse\n");

    printf("Sending...\n");
    
    header = (struct packet_header *) buffer;
    header->seq = 0;
    do {
        nbytes = fread(buffer + sizeof(struct packet_header), 1, PAYLOAD_SIZE, file);
        if (nbytes == 0 && !feof(file)) {
            printf("Erreur de lecture: %s", filename);
            return FILE_READ_ERROR;
        }            
            
        printf("Lu %d du fichier\n", nbytes);
        header->payload_size = nbytes;
        S_sendMessage(sockfd, dist_addr, buffer,
                      sizeof(header) + header->payload_size);
        header->seq++;

    } while (nbytes == PAYLOAD_SIZE);
    
    return 0;
}
