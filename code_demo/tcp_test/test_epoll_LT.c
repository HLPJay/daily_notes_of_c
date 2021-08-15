//epoll默认时lt模式，
//lt模式有一直触发的问题，需要写完移除，或者读完移除

//lt模式代码简单，可以根据业务读取固定的字节，直到读完为止


/****************************************
默认就是lt模式： 水平触发
	需要处理，写完移除，读完移除
	https://cloud.tencent.com/developer/article/1636224
*****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define EPOLL_SIZE 1024
int main(int argc, char* argv[])
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd <0){
		printf("create listen socket error \n");
		return -1;
	}

	//设置ip和端口可重用  设置非阻塞
	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));

    //设置非阻塞
    int oldSocketFlag = fcntl(sockfd, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, newSocketFlag) == -1)
    {
    	close(sockfd);
    	printf("set nonblock error. \n");
    	return -1;
    }

    //初始化服务器
    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(6666);

    //绑定端口
    if(bind(sockfd, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) < 0)
    {
    	printf("bind sockfd error \n");
    	close(sockfd);
    	return -1;
    }
    //启动监听
    if(listen(sockfd, SOMAXCONN) < 0)//内核内规定的最大连接数
    {
    	printf("listen sockfd error \n");
    	close(sockfd);
    	return -1;
    }

    //创建epoll 
    int epfd = epoll_create(1);
    if(epfd == -1)
    {
    	printf("create epoll fd error . \n");
    	close(sockfd);
    	return -1;
    }

    //设置相关参数，默认lt，添加fd到epoll中
    struct epoll_event listen_fd_event;
    listen_fd_event.data.fd = sockfd;
    listen_fd_event.events = EPOLLIN; //默认是LT

    if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &listen_fd_event) == -1)
    {
    	printf("epoll ctl listen fd error. \n");
    	close(sockfd);
    	return -1;
    }

    //这里是主要的逻辑区

    struct epoll_event epoll_events[EPOLL_SIZE];
    int nready;
    while(1)
    {
    	nready = epoll_wait(epfd, epoll_events, EPOLL_SIZE, 1000);
    	if(nready < 0) 
    	{
    		if (errno == EINTR)// 信号被中断
                continue;
            printf("epoll_wait error. \n");
            break;
    	}else if(nready == 0) // 超时，继续
    	{
    		continue;
    	}

    	//开始处理响应的事件，这里是LT
    	for(int i =0; i<nready; i++)
    	{
    		if(epoll_events[i].events & EPOLLIN)
    		{
    			//accept判断，默认是LT
    			if(epoll_events[i].data.fd == sockfd)
    			{
    				//accept接收  以及设置非阻塞，放入epoll中
    				struct sockaddr_in client_addr;
    				socklen_t client_len = sizeof(client_addr);
    				int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
    				if(clientfd == -1)
    				{
    					printf("accept error %s \n ", strerror(errno));
    					continue;
    				}

    				//设置非阻塞
    				int oldSocketFlag = fcntl(clientfd, F_GETFL, 0);
    				int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    				if (fcntl(clientfd, F_SETFD, newSocketFlag) == -1)
    				{
    					 close(clientfd);
    					 printf("fcntl set nonblock error. fd [%d] \n", clientfd);
    					 continue;
    				}
    				//加入epoll 监听读和写事件
    				{
    					struct epoll_event client_fd_event;
    					client_fd_event.data.fd = clientfd;
    					client_fd_event.events = EPOLLIN | EPOLLOUT;
    					if(epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &client_fd_event) == -1)
    					{
    						printf("add clientfd to epoll error \n");
    						close(clientfd);
    						continue;
    					}
						printf("new client accept, client fd is %d. \n",clientfd);
    				}
    			}
    			else
    			{
    				printf("clinetfd [%d], recv data :\n", epoll_events[i].data.fd);
    				//连接的客户发来数据
    				char  recvbuff[1024] = {0};
    				int recvsize = recv(epoll_events[i].data.fd, recvbuff, 1024, 0);
    				if(recvsize == 0)// 关闭连接
    				{
    					if(epoll_ctl(epfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, 0) == -1)
    					{
    						printf("client disconnection error from epoll \n");
    						close(epoll_events[i].data.fd);
    						continue;
    					}
    					printf("client disconnected,clientfd is [%d] \n", epoll_events[i].data.fd);
    					close(epoll_events[i].data.fd);
    				}else if(recvsize < 0) //出错情况下也是移除
    				{
    					if (errno == EWOULDBLOCK && errno == EINTR) //不做处理
    					{
    						continue;
    					}
    					if(epoll_ctl(epfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, 0) == -1)
    					{
    						printf("recv client data error.del epoll error \n");
    						close(epoll_events[i].data.fd);
    						continue;
    					}
    					printf("recv client data error, clientfd is [%d] \n", epoll_events[i].data.fd);
    					close(epoll_events[i].data.fd);
    				}else
    				{
    					//正常接收到的数据
    					printf("recv client[%d] data success [%s]. \n", epoll_events[i].data.fd, recvbuff);
    				}

    			}
    		}else if(epoll_events[i].events & EPOLLOUT)
    		{
    			if(epoll_events[i].data.fd == sockfd)
    			{
    				continue;
    			}
    			//只处理客户端的连接 会一直触发
                //还需要接受 没法删除，这应该适合接收后发送的逻辑
    			printf("EPOLLOUT send buff. \n");
    		}else if(epoll_events[i].events & EPOLLERR) //给已经关闭的端口
    		{
    			//应该关闭移除该端口
    			if(epoll_ctl(epfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, 0) == -1)
				{
					printf("recv client data error.del epoll error \n");
					close(epoll_events[i].data.fd);
					continue;
				}
				printf("epoll error . EPOLLERR \n");
				close(epoll_events[i].data.fd);
    		}
    	}
    }
    close(sockfd);
    close(epfd);
	return 0;
}