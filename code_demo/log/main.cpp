#include <stdio.h>
#include "log.h"
#include "socketWrap.h"

// int test_func1()
// {
// 	LOG_INFO(LOG_DEBUG, "%s", "hello world!!"); 
// 	return 0;
// }

// void test1()
// {
// 	LOG_SetPrintDebugLogFlag(1);//打印调试信息
// 	//LOG_SetPrintLogPlaceFlag(1);//保存打印信息到文件
// 	//LOG_Init("info", 8000);
	
// 	LOG_INFO(LOG_DEBUG, "%s", "Init log!!");
	
// 	test_func1();
	
// 	//LOG_INFO(LOG_DEBUG, "%s", "Destroy log!!");
// 	LOG_Destroy();
// }

void test2()
{
	//封装的tcp接口的测试
	int ser_fd = CreateTcpSocket();
	if(ser_fd < 0)
	{
		return -1;
	}
	//是不是应该对ip和port进行校验？
	if(!BindIpAndPort(ser_fd, "0.0.0.0", 6666))
	{
		return -1;
	}

	if(!Listen(ser_fd, 20))
	{
		return -1;
	}

	//不用设置阻塞非阻塞，直接交给epoll
	//创建epoll
	int epfd = CreateEpoll();
	//epoll_ctl 加入epoll
	EpollAdd(epfd, ser_fd);

	struct epoll_event events[1024] = {0};
	while(1)
	{
		int nready = EpollWait(epfd, events, 1024, 100); //这里超时等待
		if(nready == -1)
		{
			LOG_INFO(LOG_ERROR, "EpollWait return failed nready:[%d]", nready);
			continue;
		}

		for(int i=0; i<nready; ++i)
		{
			if(ser_fd == events[i].data.fd)
			{
				AcceptET(epfd, ser_fd);
			}else
			{
				//进行消息接收
				char buff[1024] = {0};
				int clientfd = events[i].data.fd;
				//因为时et模式，所以要考虑读全的问题
				if(events[i].events & EPOLLIN) //可读
				{
					int size = RecvET(epfd, clientfd, recv_buff, recv_len);
					printf("RecvET  buff:[%s], recvsize:[%d], buff_size: [%d]\n", recv_buff, size, recv_len);
				}else if(events[i].events & EPOLLOUT) //可写
				{
					int size = SendET(epfd, clientfd, send_buff, send_len);
					printf("SendET buff:[%s], bufflen:[%d],sendsize:[%d]\n", buff, send_len, size);
				}else
				{
					LOG_INFO(LOG_ERROR, "clientfd:%d, errno:%d .", clientfd, errno);
					Close(clientfd);
				}
			}
		}
		// sleep(1);
	}
}
int main(int argc, char *argv[])
{
	// test1();
	test2();
	return 0;
}
