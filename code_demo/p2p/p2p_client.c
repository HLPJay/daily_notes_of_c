// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include "p2p.h"
#include <pthread.h>



typedef void *(*callback)(void*arg);
//服务端的地址
struct sockaddr_in server_addr;
//根据服务端的地址获取到每个client的地址
client_table p2p_clients[1024] = {0};

static int client_selfid = 0x0;

void *send_callback(void* arg);
void *recv_callback(void* arg);
int main(int argc, char* argv[])
{
	//作为udp的客户端，实现通信对ip和端口进行保存
	printf("this is udp client. \n");
	if(argc != 4)
	{
		printf("usage:%s ip port clientid \n", argv[0]);
		exit(1);
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(1);
	}

	pthread_t thread_id[2] = {0};
	callback cb[2] = {send_callback, recv_callback};
	for (int i = 0;i < 2;i ++) {
		int ret = pthread_create(&thread_id[i], NULL, cb[i], &sockfd);
		if (ret) {
			perror("pthread_create");
			exit(1);
		}
		sleep(1);
	}
	//登陆报文 对已经创建的服务端的fd进行注册发送
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	client_selfid = atoi(argv[3]);

	send_login(sockfd, client_selfid, &server_addr);

	for (int i = 0;i < 2;i ++) {
		pthread_join(thread_id[i], NULL);
	}
	
	return 0;
}

static int status_machine = P2P_STATUS_LOGIN;
static int p2p_count = 0;


void *send_callback(void* arg)
{
	int sockfd = *(int *)arg;
	char buffer[512] = {0};

	// printf("send_callback start : \n");
	while(1)
	{
		bzero(buffer, 512);
		scanf("%s", buffer);

		//根据接收到的P2P的状态做不同的处理
		if(status_machine == P2P_STATUS_MESSAGE)
		{
			printf(" --> please enter bt : ");
			int other_id = buffer[1]-0x30;

			if (buffer[0] == 'C') { //这是连接对端客户P2P的标志
				//printf("king_send_connect");
				send_connect(sockfd, client_selfid, other_id, &server_addr);
			} else {
				int length = strlen(buffer);
				//这是直接测试发送
				client_send_message(sockfd, client_selfid, other_id, &server_addr, buffer, length);
			}
		}else if(status_machine == P2P_STATUS_P2P_MESSAGE){ 
			printf("==>please enter message to send : \n");
			//直接获取对端的地址，用fd进行发送
			
			int now_count = p2p_count;
			struct sockaddr_in c_addr;
			c_addr.sin_family = AF_INET;
			array_to_addr(p2p_clients[now_count-1].addr, &c_addr);

 			int length = strlen(buffer);

			client_send_message(sockfd, client_selfid, 0, &c_addr, buffer, length);
		}
	}
}

//对接收到的消息专门做处理
static int client_buffer_parser(int sockfd, unsigned char *buffer, unsigned int length, struct sockaddr_in *addr);
void *recv_callback(void* arg)
{
	int sockfd = *(int *)arg;
	struct sockaddr_in addr;
	int length = sizeof(struct sockaddr_in);
	unsigned char buffer[512] = {0};

	while(1)
	{
		int len = recvfrom(sockfd, buffer, 512, 0, (struct sockaddr*)&addr, &length);
		if (len > 0) {
			buffer[len] = 0;
			printf("get recv : [%d], [%s] \n",len,buffer);
			client_buffer_parser(sockfd, buffer, len, &addr);
		} else if (len == 0) {
			printf("server closed\n");
			close(sockfd);
			break;
		} else if (len == -1) {
			perror("recvfrom");
			close(sockfd);
			break;
		}
	}
}


//根据接收到的消息类型，对消息进行解析
static int client_buffer_parser(int sockfd, unsigned char *buffer, unsigned int length, struct sockaddr_in *addr)
{
	unsigned char status = buffer[1];
	printf("get recvbuff [%x] \n",status);
	switch(status)
	{
		case 0x12: //协议的请求  正常回复 保存对端的地址  并进行回复
		{
			struct sockaddr_in other_addr;
			other_addr.sin_family = AF_INET;
			array_to_addr(buffer+8, &other_addr);

			send_p2pconnect(sockfd, client_selfid, &other_addr);
			break;
		}
		case 0x13:
		{
			int now_count = p2p_count++;

			p2p_clients[now_count].stamp = time_genrator();
			p2p_clients[now_count].client_id = *(int*)(buffer+4);
			addr_to_array(p2p_clients[now_count].addr, addr);
		
			send_p2pconnectack(sockfd, client_selfid, addr);
			printf("Enter P2P Model\n");
			status_machine = P2P_STATUS_P2P_MESSAGE;

			break;
		}
		case 0x93: //构造新的p2p地址，进行保存，并改变状态
		{
			int now_count = p2p_count++;
			
			p2p_clients[now_count].stamp = time_genrator();
			p2p_clients[now_count].client_id = *(int*)(buffer+4);
			addr_to_array(p2p_clients[now_count].addr, addr);
			
			printf("Enter P2P Model\n");
			status_machine = P2P_STATUS_P2P_MESSAGE;
			break;
		}
		case 0x21://对接收到的消息做解析  获取到的id和buffer 正常回复
		{
			unsigned char *msg = buffer+12;
			unsigned int  other_id = *(unsigned int*)(buffer+4);
			
			printf(" from client:%d --> %s\n", other_id, msg);

			send_messageack(sockfd, client_selfid, addr);
			//status_machine = KING_STATUS_P2P_MESSAGE;
			
			break;
		}
		case 0x81://正常的回复状态
		{
			printf(" Connect Server Success\nPlease Enter Message : ");
			status_machine = P2P_STATUS_MESSAGE;
			break;
		}
		case 0x82:
		case 0x91:  
		case 0x92:	break;		
		case 0xA1: 	break;

	}
}
