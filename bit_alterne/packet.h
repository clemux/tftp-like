#ifndef PACKET_H
#define PACKET_H

#define PACKET_SIZE 1024
#define PAYLOAD_SIZE 512



#include <stdint.h>

struct packet_header {
    uint8_t seq : 1; // 0 ou 1
    uint8_t cmd : 1; // 0 (DATA) ou 1 (CHECKSUM)
    uint16_t payload_size;
} __attribute__((__packed__));


#endif /* PACKET_H */    
