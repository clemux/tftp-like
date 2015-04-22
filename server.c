#include "lib.h"
#include "packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define PAYLOAD_SIZE 512

int main()
{
    struct sockaddr *dist_addr = NULL;
    uint8_t *buffer = malloc(BUFSIZE * sizeof(uint8_t));
    int sockfd = S_openAndBindSocket(30000);
    int nbytes;
    struct packet_header header;

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
    } while (header.payload_size == PAYLOAD_SIZE);

    return 0;
}
