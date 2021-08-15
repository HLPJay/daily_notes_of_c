#ifndef __UDP_H__
#define __UDP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

typedef struct _CLIENT_TABLE {
	unsigned char addr[6]; 
	unsigned int client_id;
	long stamp;
} client_table;
client_table table[1024] = {0};

typedef enum {
	P2P_STATUS_NULL,
	P2P_STATUS_LOGIN,
	P2P_STATUS_HEARTBEAT,
	P2P_STATUS_CONNECT,
	P2P_STATUS_MESSAGE,
	P2P_STATUS_NOTIFY,
	P2P_STATUS_P2P_CONNECT,
	P2P_STATUS_P2P_MESSAGE,
} P2P_STATUS_SET;

static unsigned long cmpxchg(volatile long *addr, unsigned long _old, unsigned long _new) {
	unsigned char res;

	__asm__ volatile (
        "lock; cmpxchg %3, %1;sete %0;"
        : "=a" (res)
        : "m" (*addr), "a" (_old), "r" (_new)
        : "cc", "memory");

	return res;
}

static long time_genrator(void) {
	static long lTimeStamp = 0;
	static long timeStampMutex = 0;

	if(cmpxchg(&timeStampMutex, 0, 1)) {
		lTimeStamp = time(NULL);
		timeStampMutex = 0;
	}

	return lTimeStamp;
}

static int addr_to_array(unsigned char *array, struct sockaddr_in *p_addr) {
	int i = 0;

	for (i = 0;i < 4;i ++) {
		array[i] = *((unsigned char*)(&p_addr->sin_addr.s_addr) + i);
	}

	for (i = 0;i < 2;i ++) {
		array[4+i] = *((unsigned char*)(&p_addr->sin_port)+i);
	}
}

static int array_to_addr(unsigned char *array, struct sockaddr_in *p_addr) {
	int i = 0;
	
	for (i = 0;i < 4;i ++) {
		*((unsigned char*)(&p_addr->sin_addr.s_addr) + i) = array[i];
	}
	for (i = 0;i < 2;i ++) {
		*((unsigned char*)(&p_addr->sin_port)+i) = array[4+i];
	}
}


int client_count = 0;
static int get_index_by_clientid(int client_id) {
	int i = 0;

	int now_count = client_count;
	for (i = 0;i < now_count;i ++) {
		if (table[i].client_id == client_id) return i;
	}
}

static int send_notify(int sockfd, int client_id, int self_id) {

	unsigned char buffer[512] = {0};

	int index = get_index_by_clientid(self_id);

	buffer[1] = 0x12;
	*(int*)(buffer+4) = self_id;
	memcpy(buffer+8, table[index].addr, 6);

	index = get_index_by_clientid(client_id);
	struct sockaddr_in c_addr;
	c_addr.sin_family = AF_INET;
	array_to_addr(table[index].addr, &c_addr);

	int n = 8 + 6;
	n = sendto(sockfd, buffer, n, 0, (struct sockaddr*)&c_addr, sizeof(c_addr));
	if (n < 0) {
		perror("sendto");
	}

	return n;
}


static int send_p2pconnect(int sockfd, int self_id, struct sockaddr_in *paddr) {

	unsigned char buffer[512] = {0};
	
	buffer[1] = 0x13;
	*(int *)(buffer+4) = self_id;

	int n = 4 + 4;
	n = sendto(sockfd, buffer, n, 0, (struct sockaddr*)paddr, sizeof(struct sockaddr_in));
	if (n < 0) {
		perror("sendto");
	}
	
	return n;
}
static int send_login(int sockfd, int self_id, struct sockaddr_in *paddr) {

	unsigned char buffer[512] = {0};

	buffer[1] = 0x01;
	*(int *)(buffer+4) = self_id;

	int n = 4 + 4;
	// printf("send loging :\n");
	n = sendto(sockfd, buffer, n, 0, (struct sockaddr*)paddr, sizeof(struct sockaddr_in));
	if (n < 0) {
		perror("sendto");
	}
	
	return n;

}


static int send_connect(int sockfd, int self_id, int other_id, struct sockaddr_in *paddr) {
	
	unsigned char  buffer[512] = {0};

	buffer[1] = 0x11;
	*(int *)(buffer+4) = self_id;
	*(int *)(buffer+8) = other_id;

	int n = 8 + 4;
	n = sendto(sockfd, buffer, n, 0, (struct sockaddr*)paddr, sizeof(struct sockaddr_in));
	if (n < 0) {
		perror("sendto");
	}
	
	return n;	
}

//构造发送协议报文
static int client_send_message(int sockfd, int self_id, int other_id, struct sockaddr_in *paddr, unsigned char *msg, int length) {
	unsigned char buffer[512] = {0};

	buffer[1] = 0x21; 
	*(int *)(buffer+4) = self_id;
	*(int *)(buffer+8) = other_id;

	memcpy(buffer+12, msg, length);

	int n = 12 + length;
	*(unsigned short*)(buffer+2) = (unsigned short) n;
	
	n = sendto(sockfd, buffer, n, 0, (struct sockaddr*)paddr, sizeof(struct sockaddr_in));
	if (n < 0) {
		perror("sendto");
	}
	return n;
}

static int send_messageack(int sockfd, int self_id, struct sockaddr_in *paddr) {

	unsigned char buffer[512] = {0};
	
	buffer[1] = 0xA1;
	*(int *)(buffer+4) = self_id;

	int n = 4 + 4;
	n = sendto(sockfd, buffer, n, 0, (struct sockaddr*)paddr, sizeof(struct sockaddr_in));
	if (n < 0) {
		perror("sendto");
	}
	
	return n;

}

static int send_p2pconnectack(int sockfd, int self_id, struct sockaddr_in *paddr) {

	unsigned char buffer[512] = {0};
	
	buffer[1] = 0x93;
	*(int *)(buffer+4) = self_id;

	int n = 4 + 4;
	n = sendto(sockfd, buffer, n, 0, (struct sockaddr*)paddr, sizeof(struct sockaddr_in));
	if (n < 0) {
		perror("sendto");
	}
	
	return n;

}

#endif