
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <errno.h>

//实现最简单的tcp的客户端的实现
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
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
	bzero(&ser_addr,sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(6666);
	ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//inet_pton(AF_INET, ip, &addr.sin_addr);
	//将参数sockfd 的socket 连至参数serv_addr 指定的网络地址
	//返回失败非0 GetLastError()错误码=>这是windows
	printf("start connect : \n");
	int con_ret = connect(cli_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
	if (con_ret <0 && (errno == EAGAIN ||errno == EWOULDBLOCK ||  errno == EINPROGRESS)) {
		printf("connect ser_addr socket error. error is %d", errno);
		close(cli_fd);
		return -1;
	} 

	//这里用select对其进行接收
	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(cli_fd, &rset);

	int maxfd = cli_fd+1;
	// struct timeval timeoutSelect = {0, 0};
	// timeoutSelect.tv_sec = 2; //秒
	// timeoutSelect.tv_usec = 0;
	while(1)
	{
		//int sel_ret = select(maxfd, &rset, NULL, NULL, &timeoutSelect); //这是设置了select的超时  这里不用
		int sel_ret = select(maxfd, &rset, NULL, NULL,  (struct timeval *) 0);
		if (sel_ret == -1)
			printf("Error reading from socket \n");
		if (sel_ret == 0) //超时,没有数据可以读
		{
			printf("Error reading timeout \n");
			continue;
		}else
		{
			//读数据 
			char recv_buff[21] = {0};
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

			char * send_buff = "client data test .";
			int send_len = -1;
			send_len = send(cli_fd, send_buff, strlen(send_buff), 0);
			if(send_len < 0)
			{
				printf("send buff error. \n");
			}else
			{
				printf("send buff success is [%s], len is [%d] \n", send_buff, send_len);
			}
		}

	}

	close(cli_fd);
	return 0;
}