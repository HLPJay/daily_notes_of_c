#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
//实现select的简单测试
//使用select对ser_fd,已经连接上的fd进行监听，实现事件触发
//select 进行监听受限于单个进程所能打开的最大fd
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

	//创建select                
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(ser_fd, &readfds);
	//循环 对fd进行监听 

	fd_set tempfds;
	int res;
	int maxfd = ser_fd+1;
	while(1)
	{
		tempfds = readfds;
		printf("select waiting: \n");
		//监控这些最大fd 可读 可写 出错 时间（为NULL且前面无可读，一直阻塞）
		res = select(maxfd, &tempfds, (fd_set *)0,(fd_set *)0, (struct timeval *) 0);
		if(res == -1){
			printf(" select error");
			continue;
		}

		if(res == 0){
			printf("select timeout. \n");
		}//res>0是就绪描述符的数目

		//处理连接 加入队列中
		if(FD_ISSET(ser_fd, &tempfds)){
			struct sockaddr_in cli_addr;
			socklen_t client_len = sizeof(cli_addr);
			int cli_fd = accept(ser_fd, (struct sockaddr*)&cli_addr, &client_len);
			if(cli_fd <=0) continue;
			FD_SET(cli_fd, &readfds);
			if(cli_fd > maxfd)
			{
				maxfd = cli_fd+1; //这里要不要加1，保险一点？
			}
			char * send_buff = "hello client";
			int sendlen = send(cli_fd, send_buff, strlen(send_buff), 0);
			printf("sockfd %d, maxfd:%d, clientfd: %d \n", ser_fd, maxfd, cli_fd);
			if(--res == 0)  continue;
		}
		// 遍历其他节点
		for(int i=0; i<maxfd; i++)
		{
			if(FD_ISSET(i, &tempfds))
			{
				//客户端发来消息 进行接收
				char buff[21] = {0};
				int ret = recv(i, buff, 20, 0);
				if(ret < 0){
					if(errno == EAGAIN ||errno == EWOULDBLOCK)//多线程操作可能别的已经读取了
					{
						printf("read all Data\n");
					}else{
						FD_CLR(i, &readfds); //异常移除该连接
						close(i);
					}
				}else if(ret == 0){//断开 收到对方fin
					printf("disconnect %d \n", i);
					FD_CLR(i, &readfds); //关闭移除该连接
					close(i);
				}else
				{
					printf("recv :[%s], len:[%d] \n", buff, res);
					char * send_buff = "hello send client";
					int sendlen = send(i, send_buff, strlen(send_buff), 0);
					printf("send :[%s] len [%ld]", send_buff, strlen(send_buff));
				}
				if(--res == 0)  continue; //只监听到这么多节点有变化
			}
		}
	}

	close(ser_fd);
	return 0;
}