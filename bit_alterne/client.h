#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <sys/socket.h>


#define FILE_READ_ERROR 255
#define FILE_OPEN_ERROR 254
#define TIMEOUT 2

int send_packet(int sockfd, struct sockaddr *dist_addr, void *buffer, int nbytes, uint32_t seq);
int timeout_ack(int sockfd, long seconds);
int main (int argc, char* argv[]);

#endif /* CLIENT_H */
