/*************************************************
使用et,一定要设置成非阻塞
	1：处理accept
	2：处理发送和接受，发送
**************************************************/
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
    //这里用的是ET
    struct epoll_event listen_fd_event;
    listen_fd_event.data.fd = sockfd;
    listen_fd_event.events = EPOLLIN;
    listen_fd_event.events |= EPOLLET; //ET

    if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &listen_fd_event) == -1)
    {
    	printf("epoll ctl listen fd error. \n");
    	close(sockfd);
    	return -1;
    }

    //ET模式下的处理，一定要是非阻塞的。
    //et模式处理时  要进行循环达到一次接受完，阻塞的话最后一次就会阻塞住
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
    	int connfd = -1;
    	for(int i =0; i<nready; i++)
    	{
    		if(epoll_events[i].events & EPOLLIN) //有可读事件
    		{
    			if(epoll_events[i].data.fd == sockfd)
    			{
    				//因为是et模式，所以这里要用while

    				struct sockaddr_in client_addr;
    				socklen_t client_len = sizeof(client_addr);
    				while((connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len))>=0)
    				{
    					int oldSocketFlag = fcntl(connfd, F_GETFL, 0);
	    				int newSocketFlag = oldSocketFlag | O_NONBLOCK;
	    				if (fcntl(connfd, F_SETFD, newSocketFlag) == -1)
	    				{
	    					 close(connfd);
	    					 printf("fcntl set nonblock error. fd [%d] \n", connfd);
	    					 continue;
	    				}
	    				{
	    					struct epoll_event client_fd_event;
	    					client_fd_event.data.fd = connfd;
	    					client_fd_event.events = EPOLLIN | EPOLLOUT;
	    					client_fd_event.events |= EPOLLET; //ET
	    					if(epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &client_fd_event) == -1)
	    					{
	    						printf("add clientfd to epoll error \n");
	    						close(connfd);
	    						continue;
	    					}
							printf("new client accept, client fd is %d. \n",connfd);
	    				}
    				}
    			}else
    			{
    				printf("clinetfd [%d], recv data :\n", epoll_events[i].data.fd);
    				//开始接受
    				char  recvbuff[1024] = {0};
    				int recvsize = -1;
    				//一次性读完
    				while((recvsize = recv(epoll_events[i].data.fd, recvbuff, 1024, 0))>0)
    				{
    					printf("recvbuff:[%s] recvsize:[%d] \n", recvbuff, recvsize);
    				}
    				if(recvsize == 0)
    				{
    					if(epoll_ctl(epfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, 0) == -1)
    					{
    						printf("client disconnection error from epoll \n");
    						close(epoll_events[i].data.fd);
    						continue;
    					}
    					printf("client disconnected,clientfd is [%d] \n", epoll_events[i].data.fd);
    					close(epoll_events[i].data.fd);
    				}else if(recvsize < 0)
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
    				}
    			}
    		}else if(epoll_events[i].events & EPOLLOUT)
    		{
    			if(epoll_events[i].data.fd == sockfd)
    			{
    				continue;
    			}
    			//这里et，应该只触发一次
    			printf("EPOLLOUT send buff\n ");
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
/**************************************
if (events[i].events & EPOLLIN) 
{
    n = 0;
    // 一直读直到返回0或者 errno = EAGAIN
    while ((nread = read(fd, buf + n, BUFSIZ-1)) > 0) 
    {
        n += nread;
    }
    if (nread == -1 && errno != EAGAIN) 
    {
        perror("read error");
    }
    ev.data.fd = fd;
    ev.events = events[i].events | EPOLLOUT;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

ssize_t socket_write(int sockfd, const char* buffer, size_t buflen)
{
    ssize_t tmp;
    size_t total = buflen;
    const char* p = buffer;
    while(1)
    {
        tmp = write(sockfd, p, total);
        if(tmp < 0)
        {
            // 当send收到信号时,可以继续写,但这里返回-1.
            if(errno == EINTR)
                return -1;
            // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
            // 在这里做延时后再重试.
            if(errno == EAGAIN)
            {
                usleep(1000);
                continue;
            }
            return -1;
        }
        if((size_t)tmp == total)
            return buflen;
        total -= tmp;
        p += tmp;
    }
  return tmp;//返回已写字节数
}
**************************************/