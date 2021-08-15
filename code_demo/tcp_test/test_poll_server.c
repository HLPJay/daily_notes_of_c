#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <errno.h>

#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define INFTIM 10000
int main()
{
	//创建socket
	int ser_fd;
	ser_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(ser_fd < 0)
	{
		printf("create socket error \n");
		return -1;
	}

	//绑定端口
	struct sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(6666); 
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	if(bind(ser_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) < 0)
	{
		printf("bind server fd error \n");
		close(ser_fd); //这里需要吗？
		return -1;
	}

	//监听端口
	if(listen(ser_fd, 5) < 0)
	{
		printf("listen server fd error \n");
		close(ser_fd); //这里需要吗？
		return -1;
	}

	//定义poll 使用poll进行测试。
	struct pollfd poll_event[1024];
	poll_event[0].fd = ser_fd;
	poll_event[0].events = POLLIN;

	for(int i=1; i<1024; i++)
	{
		poll_event[i].fd = -1;
	}

	int maxfd = 0; //标识当前poll中存储fd最大位置
	int nready;
	int conn_fd;

	while(1)
	{
		nready = poll(poll_event, maxfd+1, INFTIM);
		if(nready < 0)
		{
			printf("poll error . \n");
			return -1;
		}
		// if(nready == 0) //表示超时  INFTIM 一直阻塞 0不阻塞
		// {
		// 	printf("poll timeout . \n");
		// }

		//服务端监听的fd
		if(poll_event[0].revents & POLLIN)
		{
			conn_fd = accept(ser_fd, NULL, NULL);//这里暂时没获取客户端的信息
			printf("connect fd is %d \n", conn_fd);
			int i;
			for(i = 1; i<1024; ++i)
			{
				if(poll_event[i].fd == -1)
				{
					poll_event[i].fd = conn_fd;
					poll_event[i].events = POLLIN;
					break;
				}
			}
			if(i == 1024)
			{
				printf("too many client connect. \n");
				return -1;
			}
			if(i > maxfd)
			{
				maxfd = i ;
			}
			if ( --nready <= 0 )
            	continue ;
		}
		//判断有连接发的内容fd,有数据可读
		for(int i =1; i<=maxfd; i++)
		{
			if(poll_event[i].fd <0)
			{
				continue;
			}
			//可读
			if(poll_event[i].revents &(INFTIM | POLLERR))
			{
				char recv_buff[21] = {0};
				int read_len;
				read_len = read(poll_event[i].fd, recv_buff, 20);
				printf("recv len {%d}\n",read_len);
				if(read_len < 0)
				{
					if(errno == ECONNRESET)
					{
						close(poll_event[i].fd);
						poll_event[i].fd = -1;
					}else
					{
						printf("poll read error \n");
					}
				}
				if(read_len == 0) //断开连接
				{
					close(poll_event[i].fd);
					poll_event[i].fd = -1;
				}

				if(read_len >0)
				{
					printf("poll recv [%s] len [%d]. \n", recv_buff, read_len);
				}
			}
		}
	}
	close(ser_fd);
	return 0;
}
