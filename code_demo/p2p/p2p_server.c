

// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>

#include "p2p.h"

int parse_buffer(int sockfd, unsigned char *buffer, unsigned int length, struct sockaddr_in *addr);
int main(int argc, char* argv[])
{
	//创建一个udp，用来通信获取客户端的ip相关信息
	printf("this is a udp service \n");

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(0);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}

	char buff[512] = {0};
	struct sockaddr_in client_addr;
	int length = sizeof(struct sockaddr_in);

	int len;
	//开始接受服务端的消息并进行解析
	while(1)
	{
		len = recvfrom(sockfd, buff, 512, 0, (struct sockaddr*)&client_addr, &length);
		if(len > 0)
		{
			buff[len] = 0x0;
			printf("server recv :%d.%d.%d.%d:%d say: %s\n", 
				*(unsigned char*)(&client_addr.sin_addr.s_addr),
				*((unsigned char*)(&client_addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&client_addr.sin_addr.s_addr)+2), 
				*((unsigned char*)(&client_addr.sin_addr.s_addr)+3),													
				client_addr.sin_port, buff);

			//对sockfd收到的消息进行解析
			int ret = parse_buffer(sockfd, buff, len, &client_addr);
			if(ret == -1)
			{
				continue;
			}

			buff[1] += 0x80;
			len = sendto(sockfd, buff, len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
			if (len < 0) {
				perror("sendto");
				break;
			} 
		}else if (len == 0) {
			printf("server closed\n");
		} else {
			perror("recv");
			break;
		}
	}

	return 0;
}

// int client_count = 0;

//对接受到的数据进行解析，然后同样的进行下发
int parse_buffer(int sockfd, unsigned char *buffer, unsigned int length, struct sockaddr_in *addr)
{
	unsigned char status = buffer[1];
	printf("buffer parse status is  ===>:%d \n", status);
	switch(status)
	{
		case 0x01: //有新的客户端进行连接
		{
			//计算连接客户端的个数
			int old = client_count;
			int now = old+1;
			//做交换吧
			if(0 == cmpxchg((volatile long *)&client_count, old, now))
			{
				printf("client_count --> %d, old:%d, now:%d\n", client_count, old, now);
				return -1;
			}
			//客户端地址池 放入到地址池中 存储了ip和port
			unsigned char array[6] = {0};
			addr_to_array(array, addr);
			printf("login --> %d.%d.%d.%d:%d\n", 
					*(unsigned char*)(&addr->sin_addr.s_addr), 
					*((unsigned char*)(&addr->sin_addr.s_addr)+1),													
					*((unsigned char*)(&addr->sin_addr.s_addr)+2), 
					*((unsigned char*)(&addr->sin_addr.s_addr)+3),													
					addr->sin_port);

			table[now].client_id = *(unsigned int *)(buffer+4);
			memcpy(table[now].addr, array, 6);
			break;
		}
		case 0x02: //根据id 获取table表中存的下标
		{
			int client_fd =  *(unsigned int*)(buffer+4);
			int index =  get_index_by_clientid(client_fd);
			table[index].stamp = time_genrator();
			break;
		}
		case 0x11:
		{
			int client_id = *(unsigned int*)(buffer+4);
			int other_id = *(unsigned int*)(buffer+8);

			send_notify(sockfd, other_id, client_id);
			break;
		}
		case 0x21:
		{
			unsigned char *msg = buffer+12;
			int client_id = *(unsigned int*)(buffer+4);
			int other_id = *(unsigned int*)(buffer+8);
			
			printf(" from client:%d --> %s\n", client_id, msg);
			break;
		}
	}
	return 0;

}

