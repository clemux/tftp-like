#ifndef LIB_H
#define LIB_H

#include <sys/socket.h>
#include <stdio.h>
#include <stdint.h>

#define SOCK_CREATION_FAILED -128
#define SOCK_BINDING_FAILED -127
#define SOCK_SENDTO_FAILED -126
#define SOCK_RECV_FAILED -125
#define ADDRESS_ERROR -64
#define INVALID_PORT_ERROR -32
#define FILE_BUF_SIZE 4096 // for compute_md5

#define TIMEOUT 2
#define NB_TRIES 20



int S_openAndBindSocket(int local_port, int domain);

int S_openSocket(int domain);

int S_distantAddress(char *IP_address, int port,
                     struct sockaddr **dest_addr, int domain);

int S_receiveMessage(int sock_fd, struct sockaddr *dest_addr,
                     void *msg, int length);

int S_sendMessage (int sock_fd, struct sockaddr *dest_addr, 
                   void *msg, int length);

int string2port(char* s);

char* compute_md5(FILE *file);

int timeout_ack(int sockfd, long seconds);
int send_packet(int sockfd, struct sockaddr *dist_addr, void *buffer,
                int nbytes, uint8_t seq, uint8_t cmd);
int send_ack(int sockfd, struct sockaddr *addr, uint8_t seq, uint8_t cmd);



#endif /* LIB_H */


