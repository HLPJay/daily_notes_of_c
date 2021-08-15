
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
//实现最简单的tcp的客户端的实现
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{

	//创建socket
	int cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(cli_fd < 0)
	{
		printf("create client fd error. \n");
		return -1;
	}

	//连接远端服务器 定义服务端的东东
	struct sockaddr_in ser_addr;
	memset(&ser_addr, 0, sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(6666);
	ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//inet_pton(AF_INET, ip, &addr.sin_addr);
	//将参数sockfd 的socket 连至参数serv_addr 指定的网络地址
	//返回失败非0 GetLastError()错误码=>这是windows
	printf("start connect : \n");
	if(connect(cli_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) != 0)
	{
		printf("connect ser_addr error \n");
		close(cli_fd);
		return -1;
	}
	//进行发送测试
	char * buff = "client data test .";
	int len = strlen(buff);
	printf("send buff is [%s], len is [%d] \n", buff, len);

	char* recv_buff = (char*)malloc(20);

	while(1)
	{
		memset(recv_buff, 0, 20);
		//cli_fd已经连接上，直接向其中写入数据就好
		int recv_len = -1;
		recv_len = recv(cli_fd, recv_buff, 20, 0);
		if(recv_len < 0)
		{
			printf("recv buff error \n");
		}else
		{
			printf("recv buff success is [%s],[%d] \n", recv_buff, recv_len);
		}

		int send_len = -1;
		send_len = send(cli_fd, buff, len, 0);
		if(send_len < 0)
		{
			printf("send buff error. \n");
		}else
		{
			printf("send buff success is [%s], len is [%d] \n", buff, len);
		}

		
		sleep(1);
	}

	close(cli_fd);
	return 0;
}