// typedef unsigned long int uint64_t;
// static uint64_t GetCurrentTime()
// {
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return tv.tv_sec * 1000 + tv.tv_usec / 1000;
// }


/***************************
typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event
{
  uint32_t events;     Epoll events 
  epoll_data_t data;   User data variable 
} __EPOLL_PACKED;

从epoll_event 结构体可以看出，可以使用data中的ptr，或者fd进行事件触发的判断
****************************/

//一般的epoll用fd进行判断，这里我们用ptr封装实现



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


/*****************************************************
梳理实现思路：
	借用epoll实现事件触发，一个epoll可以管理一个reactor处理器
	epoll事件需要包括：
		fd, 回调函数，参数

*****************************************************/

//注意这里的回调函数定义没有带* typedef int (*callFunc)(void *data);
typedef int CALLBACK(int, int, void*);
typedef struct my_event{
	int fd;
	int events;   //保存监听的事件类型 in out 还是其他
	int (*callback)(int fd, int events, void * arg);
	void* arg;

	int status;
	char buffer[1024]; //这里直接当作接收和处理的buffer，实际上要分开
	int length;
}MY_EPOLL_EVENT;


//管理节点，每个epoll对应管理一个
typedef struct reactor_manager
{
	int epfd;
	MY_EPOLL_EVENT* events;
}REACTOR_MANAGER;

//初始化对应的事件  保存相关的参数
void epoll_event_set(MY_EPOLL_EVENT * ev, int fd, CALLBACK callback, void *arg)
{
	ev->fd = fd; //触发的fd
	ev->events = 0; //需要监听的事件
	ev->callback = callback; //对应的回调函数
	ev->arg = arg; //把管理节点传进来，后续操作

	return ;
}

//使用epoll进行监听
int epoll_event_add(int epfd, int events, MY_EPOLL_EVENT* ev)
{
	struct epoll_event ep_ev = {0, {0}};
	ep_ev.data.ptr = ev; //epoll这里可以存
	ep_ev.events = ev->events = events; // epoll和这里的事件都改变监听类型

//业务处理 epoll_ctl 参数标识 只是为了监听add和mod的状态
	int op;
	if(ev->status == 1)
	{
		op = EPOLL_CTL_MOD;
	}else
	{
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

//加入epoll的监听中
	if(epoll_ctl(epfd, op, ev->fd, &ep_ev) < 0)
	{
		printf("event add failed [fd = %d], events[%d] \n",ev->fd, events);
		return -1;
	}

	return 0;
}

//删除监听的事件
int epoll_event_del(int epfd, MY_EPOLL_EVENT *ev)
{
	//证明就没加入过
	if(ev->status != 1)
	{
		return -1;
	}
	struct epoll_event ep_ev = {0, {0}};
	ep_ev.data.ptr = ev;
	ev->status = 0;
	ev->events = 0;
	epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &ep_ev);
	return 0;
}


//传入端口，实现监听  端口号用short
int init_sock(unsigned short port);
//直接申请了数组大小的事件
int reactor_init(REACTOR_MANAGER * reator);

//加入监听事件中 自己的fd对应自己的回调函数
int reactor_addlistener(REACTOR_MANAGER* reactor, int fd, CALLBACK *callback);
int reactor_run(REACTOR_MANAGER* reactor);
int reactor_destory(REACTOR_MANAGER* reactor);

//三种回调函数
int accept_callback(int fd, int events, void * arg);
int send_callback(int fd, int events, void * arg);
int recv_callback(int fd, int events, void * arg);

int main(int argc, char* argv[])
{
	unsigned short port = 8888;
	if(argc == 2)
	{
		port = atoi(argv[1]);
	}

	REACTOR_MANAGER * reactor = (REACTOR_MANAGER*)malloc(sizeof(REACTOR_MANAGER));
	reactor_init(reactor);

	//可以监听多个fd，先创建fd，再加入监听事件中
	int listenfd[100] = {0};
	for(int i=0; i<100; i++)
	{
		listenfd[i] = init_sock(port+i);
		reactor_addlistener(reactor, listenfd[i], accept_callback);
	}

	//事件的触发以及循环进行处理
	reactor_run(reactor);

	//对reactor进行销毁
	reactor_destory(reactor);

	//关闭对应的端口
	for(int i=0; i<100; i++)
	{
		close(listenfd[i]);
	}
	free(reactor);
	return 0;
}
//创建socket，设置非阻塞 设置对应的回调
//accept进行监听 
//监听到设置非阻塞，设置对应的回调函数

int init_sock(unsigned short port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(fd, F_SETFL, O_NONBLOCK);

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	//研究源码了解一下listen的第二个参数
	if(listen(fd, 20) < 0)
	{
		printf("fd [%d] listen failed: %s \n", fd, strerror(errno));
	}
	printf("listen port: [%d] fd: [%d] \n", port, fd);
	return fd;
}


//这里直接申请一定数据的event，是限制
//为管理节点进行初始化
int reactor_init(REACTOR_MANAGER * reactor)
{
	if(reactor == NULL)
	{
		return -1;
	}
	memset(reactor, 0, sizeof(REACTOR_MANAGER));
	reactor->epfd = epoll_create(1);
	if(reactor->epfd < 0)
	{
		printf("create epfd in [%s] error [%s]\n ",__func__, strerror(errno));
		return -2;
	}
	reactor->events = (MY_EPOLL_EVENT*)malloc(1024* sizeof(MY_EPOLL_EVENT));
	if(reactor->events == NULL)
	{
		printf("create epfd in [%s] error [%s]\n ",__func__, strerror(errno));
		close(reactor->epfd);
		return -3;
	}
	return 0;
}

int reactor_destory(REACTOR_MANAGER* reactor)
{
	if(reactor != NULL)
	{
		close(reactor->epfd);
		free(reactor->events);
	}
	return 0;
}

/**********************************************************************************
void epoll_event_set(MY_EPOLL_EVENT * ev, int fd, CALLBACK callback, void *arg);
int epoll_event_add(int epfd, int events, MY_EPOLL_EVENT* ev);
int epoll_event_del(int epfd, MY_EPOLL_EVENT *ev);

int accept_callback(int fd, int events, void * arg);
int send_callback(int fd, int events, void * arg);
int recv_callback(int fd, int events, void * arg);
**********************************************************************************/
//把对应的fd加入到epoll事件中
//默认规则是fd就是事件下标
int reactor_addlistener(REACTOR_MANAGER* reactor, int fd, CALLBACK *callback)
{
	if(reactor == NULL ||reactor->events == NULL)
	{
		return -1;
	}
	//epoll的监听类型和epoll_ctl是两回事
	epoll_event_set(&reactor->events[fd], fd, callback, reactor);
	epoll_event_add(reactor->epfd, EPOLLIN, &reactor->events[fd]);
	return 0;

}

int reactor_run(REACTOR_MANAGER* reactor)
{
	if(reactor == NULL || reactor->events ==NULL || reactor->epfd <0)
	{
		return -1;
	}
	//定义事件数组，接收epoll_wait参数做处理
	struct epoll_event event_waits[102400];
	int checkpos = 0, i;
	while(1)
	{
		int nready = epoll_wait(reactor->epfd, event_waits, 102400, 1000);//超时时间
		if(nready < 0) //超时返回0 错误返回-1
		{
			printf("epoll_wait error. exit \n");
			continue;
		}

		//触发到对应的事件，执行对应的回调
		for( i = 0;i <nready; i++)
		{
			MY_EPOLL_EVENT* ev = (MY_EPOLL_EVENT*)event_waits[i].data.ptr;
			if((event_waits[i].events & EPOLLIN) && (ev->events & EPOLLIN))
			{
				ev->callback(ev->fd, event_waits[i].events, ev->arg);
			}
			if((event_waits[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
			{
				ev->callback(ev->fd, event_waits[i].events, ev->arg);
			}
			
		}
	}
	return 0;
}
//MY_EPOLL_EVENT  REACTOR_MANAGER
//accept监听回调
int accept_callback(int fd, int events, void * arg)
{
	//通过参数获取到管理节点，然后进行事件处理
	REACTOR_MANAGER * reactor = (REACTOR_MANAGER*)arg;
	if(reactor == NULL)
	{
		return -1;
	}

	//开始进行accept监听，监听到的fd设置非阻塞塞入reactor事件中
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int clientfd;
	if((clientfd = accept(fd, (struct sockaddr*)&client_addr, &len)) == -1)
	{
		//出错
		if (errno != EAGAIN && errno != EINTR)
		{
			//应该跳过， 重试或者中断逻辑
			printf("errno not EAGAIN or EINTR \n");
		}
		printf("accept: %s\n", strerror(errno));
		return -1;
	}

	int flag = 0;
	if ((flag = fcntl(clientfd, F_SETFL, O_NONBLOCK)) < 0) 
	{
		printf("%s: fcntl nonblocking failed, %d\n", __func__, clientfd);
		return -1;
	}
	epoll_event_set(&reactor->events[clientfd], clientfd, recv_callback, reactor);
	epoll_event_add(reactor->epfd, EPOLLIN, &reactor->events[clientfd]);
	// printf("new connect [%s:%d][time:%ld], pos[%d]\n", 
	// 	inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), reactor->events[i].last_active, i);
	printf("new connect [%s:%d]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	return 0;
}
// void epoll_event_set(MY_EPOLL_EVENT * ev, int fd, CALLBACK callback, void *arg);
// int epoll_event_add(int epfd, int events, MY_EPOLL_EVENT* ev);
// int epoll_event_del(int epfd, MY_EPOLL_EVENT *ev);

//接收监听 开始执行接收，并设置发送的事件
int recv_callback(int fd, int events, void * arg)
{
	REACTOR_MANAGER * reactor = (REACTOR_MANAGER*)arg;
	if(reactor == NULL)
	{
		return -1;
	}
	MY_EPOLL_EVENT* ev = reactor->events +fd;

	int len = recv(fd, ev->buffer, 1024, 0);
	epoll_event_del(reactor->epfd, ev);

	if(len>0)
	{
		ev->length = len;
		ev->buffer[len] = '\0';

		printf("client [%d]: [%s] \n", fd, ev->buffer);
		epoll_event_set(&reactor->events[fd], fd, send_callback, reactor);
		epoll_event_add(reactor->epfd, EPOLLOUT, ev);
	}else if(len == 0)
	{
		close(ev->fd);
		printf("[fd=%d]  closed\n", fd);
	}else{
		close(ev->fd);
		printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));	
	}
}

//发送监听 完了监听接收
int send_callback(int fd, int events, void * arg)
{
	REACTOR_MANAGER * reactor = (REACTOR_MANAGER*)arg;
	if(reactor == NULL)
	{
		return -1;
	}
	MY_EPOLL_EVENT* ev = reactor->events +fd;

	int len = send(fd, ev->buffer, ev->length, 0);
	if(len > 0)
	{
		printf("send[fd=%d], [%d]%s\n", fd, len, ev->buffer);
		epoll_event_del(reactor->epfd, ev);

		epoll_event_set(ev, fd, recv_callback, reactor);
		epoll_event_add(reactor->epfd, EPOLLIN, ev);
	}else
	{
		close(ev->fd);
		epoll_event_del(reactor->epfd, ev);
		printf("send[fd=%d] error %s\n", fd, strerror(errno));
	}
	return len;
}

