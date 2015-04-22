#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

struct packet_header {
    uint32_t seq;
    uint16_t payload_size;
} __attribute__((__packed__));


#endif /* PACKET_H */    
