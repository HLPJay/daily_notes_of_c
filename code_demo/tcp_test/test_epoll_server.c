/****************************************
select: 内部其实fd的存储用的是bimap,大小限制在1024，虽然可以修改，但是有一定的限制。
	修改的是自身的fd_set，需要遍历，内核态和用户态
poll:	不再是bitmap，用的是结构体pollfd，可读修改的也不再是自身。
epoll:	 epoll_create epoll_ctl（决定增加事件，删除事件）  epoll_wait（用户态和内核态共享内存）
		有数据时是置位重排,就绪链表
*****************************************/

/****************************************
分为水平触发和边缘触发，这里暂时实现demo
*****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define BUFFER_LEN 1024
#define EPOLL_SIZE 1024

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

	//epoll_create 参数已经无意义了，只有0和1的差异
	//从 Linux 内核 2.6.8 版本起，size 这个参数就被忽略了，只要求 size 大于 0 即可。
	int epfd = epoll_create(1);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = ser_fd;//用于epoll_wait时传出来
	epoll_ctl(epfd, EPOLL_CTL_ADD, ser_fd, &ev);//根节点 sockefd当key用 
	
	//用于接受返回
	struct epoll_event events[EPOLL_SIZE] = {0};
	while(1)
	{
		//等待接收
		//epoll事件fd 存放就绪事件集合  存放最大事件个数 -1代表阻塞等待

		int nready = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		//<0 出错，可以通过 errno 值获取出错原因
		//=0 超时时间 timeout 到了
		if(nready == -1)
		{
			printf("epoll wait return failed. \n");
			continue;
		}

		printf("epoll_wait nready is %d \n", nready);

		for(int i=0; i<nready; ++i)
		{
			//对传出参数进行判断处理
			//服务端fd 监听连接
			printf("epoll recv fd is [%d]\n",events[i].data.fd );
			if(events[i].data.fd == ser_fd)
			{
				struct sockaddr_in clientaddr;
				memset(&clientaddr, 0,sizeof(struct sockaddr_in));
				socklen_t client_len = sizeof(clientaddr);
				//这里如果clientaddr 相关的一些处理或者打印
				int clientfd = accept(ser_fd, (struct sockaddr*)&clientaddr, &client_len);
				//很多个连接同时连接的时候   会有遗漏？ 用while 执行accept 
				printf("conn is [%d] \n", clientfd);
				if(clientfd<=0)
					continue;
				ev.data.fd = clientfd;
				ev.events = EPOLLIN | EPOLLET; //设置触发方式 可以设置非阻塞和关闭后可重用
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
			}else
			//其他得到fd，进行消息接收
			{
				printf("start recv buff \n");
				int clientfd = events[i].data.fd;
				char buffer[BUFFER_LEN] = {0};
				int ret = recv(clientfd, buffer, BUFFER_LEN, 0);
				printf("recv [%s], len [%d] \n", buffer, ret);
				if(ret<0)
				{
					close(clientfd);

					struct epoll_event ev1;
					ev1.events = EPOLLIN; 
					ev1.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev1);
				}else if(ret ==0)//断开 收到对方fin
				{
					printf("disconnect %d", i);
					close(clientfd);

					struct epoll_event ev;
					ev.events = EPOLLIN; 
					ev.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
				}else{
					printf("RECV :%s, %d bytes from %d \n", buffer, ret, clientfd);
				}
			}
		}
		usleep(1000);
	}
	return 0;
}