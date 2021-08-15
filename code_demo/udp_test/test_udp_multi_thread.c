#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void * recv_callback(void *arg);
//使用多线程实现udp的发送和接收
//接收需要绑定本地端口实现接收
//发送要知道对端的ip和端口号
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("use ./test bind_port\n");
		return -1;
	}
	//新建一个线程，专门实现接收
	//主线程实现发送
	int port = atoi(argv[1]);
	if(port < 1025 || port > 65535)
	{
		printf("port number range 1025 to 65535 \n");
		return -1;
	}


	int udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sock_fd < 0)
	{
		printf("create socket error. \n");
		return -1;
	}

	struct sockaddr_in local_addr = {0};
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	int ret = bind(udp_sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr));
	if(ret < 0)
	{
		printf("bind error \n");
		close(udp_sock_fd);
		return -1;
	}

	//新建线程 绑定端口进行接收
	pthread_t pid;
	pthread_create(&pid, NULL, recv_callback, (void*) &udp_sock_fd);
	//主线程实现发送 定义发送的buff 和目标地址
	struct sockaddr_in dest_addr = {0};
	dest_addr.sin_family = AF_INET;
	char buff[1024] = {0};
	char dest_ip [32] = {0};
	int dest_port = 0;
	while(1)
	{
		printf("dest_ip dest_port msg: \n");
		scanf("%s %d %s", dest_ip, &dest_port, buff);
		dest_addr.sin_port= htons(dest_port);
		dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
		sendto(udp_sock_fd, buff, strlen(buff), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
		if(strcmp(buff, "exit") == 0 ||strcmp(buff, "") == 0)
		{
			pthread_cancel(pid);
			break;
		}
		memset(buff, 0, sizeof(buff));
		memset(dest_ip, 0, sizeof(dest_ip));
	}
	close(udp_sock_fd);
	return 0;
}


void *recv_callback(void *arg)
{
	int sockfd = *(int*)arg;

	//专门实现接收，端口已经绑定过
	char buff[1024] = {0};
	struct sockaddr_in src_addr = {0};
	int len = sizeof(src_addr);

	int ret = 0;
	while(1)
	{
		ret = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&src_addr, &len);
		if(ret == -1) //这里其实应该根据状态做一些判断  ==0说明断开
		{
			break;
		}
		printf("src_ip:[%s], src_port[%d] ",inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));
		printf("msg = %s \n", buff);
		if(strcmp(buff, "exit") == 0 || strcmp(buff, "") == 0)
		{
			//结束，通知主线程？
			//通过信号？实现结束主线程？
			break;
		}
		memset(buff, 0, sizeof(buff));
	}
	close(sockfd);
	return NULL;
}