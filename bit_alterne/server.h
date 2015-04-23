#ifndef SERVER_H
#define SERVER_H

int send_ack(int sockfd, struct sockaddr *addr, uint8_t seq);
int main(int argc, char *argv[]);


#endif /* SERVER_H */
