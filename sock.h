#ifndef _SOCK_H_
#define _SOCK_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 1024

typedef struct{
	int sock;
	struct sockaddr_in addr;
	int addr_size;
} sock_info;

void error_handling(const char* msg);
int initSocketTCPServer(uint16_t port);
int initSocketTCPConnect(const char* ip,  uint16_t port);

#endif
