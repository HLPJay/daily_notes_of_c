#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h> //这个接口中包含了select
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

//需要绑定本机的ip和端口号
//发送的时候需要传入对端的ip和端口号
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("use ./test bind_port\n");
		return -1;
	}

	int port = atoi(argv[1]);
	if(port < 1025 || port > 65535)
	{
		printf("port number range 1025 to 65535 \n");
		return -1;
	}

	//创建socket
	int udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sock_fd == -1)
	{
		printf("create udp socket error \n");
		return -1;
	}
	//绑定端口
	struct sockaddr_in bind_addr = {0};
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(port);
	bind_addr.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(udp_sock_fd, (struct sockaddr*)&bind_addr, sizeof(bind_addr));
	if(ret < 0)
	{
		printf("bind error \n");
		close(udp_sock_fd);
		return -1;
	}else
	{
		printf("recv ready! \n");
	}

	//定义并创建select，加入fd，实现可读可写的监控
	fd_set fds,tmpfds;
	FD_ZERO(&fds);
	FD_SET(0, &fds); //把标准输入加入select
	FD_SET(udp_sock_fd, &fds);


	char send_buff[1024] = {0};
	int dest_port = 0;
	char dest_ip[32] = {0};
	struct sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;


	char recv_buff[1024] = {};
	struct sockaddr_in src_addr;
	int len = sizeof(src_addr);
	while(1)
	{
		tmpfds = fds; 
		//可读可写可执行
		ret = select(udp_sock_fd+1, &tmpfds, NULL, NULL, NULL);
		if(ret < 0)
		{
			printf("select error \n");
			break;
		}

		if(ret == 0)
		{
			printf("Error reading timeout \n");
			continue;
		}
		//键盘有输入就会触发  然后去读，
		if(FD_ISSET(0, &tmpfds)) //有标准输入，则发送
		{

			printf("dest_ip dest_port msg: \n");
			scanf("%s %d %s",dest_ip, &dest_port, send_buff);
			dest_addr.sin_port = htons(dest_port);
			dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
			sendto(udp_sock_fd, send_buff, strlen(send_buff), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
			if(strcmp(send_buff, "exit") == 0 ||strcmp(send_buff, "") == 0)
			{
				break;
			}
			memset(send_buff, 0, sizeof(send_buff));
			memset(dest_ip, 0, sizeof(dest_ip));
		}
		//有消息发送来就会触发
		if(FD_ISSET(udp_sock_fd, &tmpfds)) //有消息收到
		{
			ret = recvfrom(udp_sock_fd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&src_addr, &len);
			if(ret == -1)
			{
				printf("recvfrom error \n");
				break;
			}
			printf("src_ip:[%s] src_port:[%d] ", inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));
			printf("recv_buff=[%s] \n", recv_buff);
			if(strcmp(recv_buff, "exit") == 0 ||strcmp(recv_buff, "") == 0)
			{
				break;
			}
			memset(recv_buff, 0, sizeof(recv_buff));
		}
	}
	close(udp_sock_fd);
}