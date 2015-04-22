#ifndef LIB_H
#define LIB_H

#include <sys/socket.h>

#define SOCK_CREATION_FAILED -1
#define SOCK_BINDING_FAILED -2

int S_openAndBindSocket(int local_port);

int S_openSocket(void);

int S_distantAddress(char *IP_address, int port,
                     struct sockaddr **dest_addr);

int S_receiveMessage(int sock_fd, struct sockaddr *dest_addr,
                     unsigned char *msg, int length);

int S_sendMessage (int sock_fd, struct sockaddr *dest_addr, 
                   unsigned char *msg, int length);




#endif /* LIB_H */

