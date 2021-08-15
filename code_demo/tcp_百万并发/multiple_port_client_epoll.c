#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

/*************************************************
作为测试百万并发的客户端
创建多个socket，一直进行连接。
注意，如果服务器的端口监听多个，这里也要遍历去进行连接

设置端口可重用主要是针对服务端，Address already in use

这里也是大量的客户端：涉及epoll监听的大小
***************************************************/
#define MAX_EPOLLSIZE	(384*1024)
#define LISTEN_MAX_PORT 100
#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

static int SetNonblock(int fd) {
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) return -1;
	return 0;
}

static int SetReUseAddr(int fd) {
	int reuse = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
}

//输入服务端的ip和port开始进行连接
int main(int argc, char* argv[])
{

	if(argc <= 2)
	{
		printf("Usage : %s ip port\n", argv[0]);
		return -1;
	}

	//获取ip port
	const char * ip = argv[1];
	int port = atoi(argv[2]);

	//epoll相关
	struct epoll_event events[MAX_EPOLLSIZE];
	int epfd = epoll_create(1);

	//服务器地址相关
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);


	//相关参数
	int connect_nums = 0; //连接数量的判断
	int port_index = 0;   //监听端口的判断

	int sockfd = 0;
	struct epoll_event ev;

	char buffer[128] = {0};


	//为了获取间隔时间做准备 gettimeofday接口
	struct timeval tv_begin;
	gettimeofday(&tv_begin, NULL);
	//循环遍历塞入不同的服务端监听port
	while(1)
	{
		if(++port_index >= LISTEN_MAX_PORT) port_index = 0;

		//socket，连接并进行发送,加入epoll_ctl
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) 
		{
			perror("socket create error \n");
			goto err;
		}
		//连接服务端
		addr.sin_port = htons(port+port_index);
		if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) 
		{
			perror("connect error\n");
			goto err;
		}

		//设置非阻塞和可重用，并进行发送探测：
		SetNonblock(sockfd);
		SetReUseAddr(sockfd);
		sprintf(buffer, "Hello Server: client num --> %d\n", connect_nums);
		send(sockfd, buffer, strlen(buffer), 0);
		connect_nums++;

		ev.data.fd = sockfd;
		ev.events = EPOLLIN | EPOLLOUT;
		epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
		//每间隔999个epoll就去开始监听一次，进行发送和接收

		if(connect_nums%1000 == 999)
		{
			//获取当前时间，替换时间
			struct timeval tv_cur;
			memcpy(&tv_cur, &tv_begin, sizeof(struct timeval));
			gettimeofday(&tv_begin, NULL);
			int time_used = TIME_SUB_MS(tv_begin, tv_cur);
			printf("connect_nums: %d, sockfd:%d, time_used:%d\n", connect_nums, sockfd, time_used);
			//使用epoll进行监听等待
			int nready = epoll_wait(epfd, events, connect_nums, 100);
			for(int i=0; i<nready; i++)
			{
				//获取监听到的fd，进行发送
				int clientfd = events[i].data.fd;
				if (events[i].events & EPOLLOUT) {
					//这里是客户端连接服务端的fd 自己的port和服务端信息的五元组
					sprintf(buffer, "data from fd:%d\n", clientfd);
					send(clientfd, buffer, strlen(buffer), 0);
				}else if(events[i].events & EPOLLIN)
				{
					char recv_buff[128] = {0};
					ssize_t recv_length = recv(clientfd, recv_buff, 128, 0);
					if(recv_length >0)
					{
						printf("recv buff is [%s] \n", recv_buff);
						// if (!strcmp(recv_buff, "quit")) {
						// 	isContinue = 0;
						// }
					}else if (recv_length == 0) 
					{
						printf(" Disconnect clientfd:%d\n", clientfd);
						connect_nums --;
						close(clientfd);
					}else
					{
						if (errno == EINTR ||errno == EAGAIN) continue;
						//errno代码为11(EAGAIN)  对非阻塞socket而言，EAGAIN不是一种错误
						printf(" Error clientfd:%d, recv errno:%d\n", clientfd, errno);
						close(clientfd);
					}
				}else 
				{
					//监听到其他异常
					printf(" clientfd:%d, epoll_wait  errno:%d\n", clientfd, errno);
					close(clientfd);
				}

			}
		}
		usleep(1 * 1000);
	}


	return 0;
err:
	printf("return error : %s\n", strerror(errno));
	return 0;
}