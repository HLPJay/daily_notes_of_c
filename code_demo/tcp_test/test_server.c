//了解一下tcp调用的流程
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //open close


#include <netinet/in.h>
#include <arpa/inet.h>
/***************************
SOCK_STREAM： 提供面向连接的稳定数据传输，即TCP协议。
OOB：		  在所有数据传送前必须使用connect()来建立连接状态。
SOCK_DGRAM：  使用不连续不可靠的数据包连接。
SOCK_SEQPACKET： 提供连续可靠的数据包连接。
SOCK_RAW： 提供原始网络协议存取。
SOCK_RDM： 提供可靠的数据包连接。
SOCK_PACKET： 与网络驱动程序直接通信。
****************************/
//实现最简单的tcp 服务端的流程
int main()
{
	//创建socket  ==》自己的fd
	int ser_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(ser_fd < 0)
	{
		printf("create server socket fd error. \n");
		return -1;
	}
	//这里可以用setsockopt 设置socket的一些属性

	//绑定端口和ip ==》fd做配置
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6666); //转成网络字节序（高字节存在低位），大端
	//INADDR_ANY 其实就是 0.0.0.0 监听所有ip
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);//主机的无符号长整形数转换成网络字节顺序
	//serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(bind(ser_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in))<0)
	{
		printf("bind server socket error \n");
		return -1;
	}
	//开始监听socket  ==》自己的fd
	if(listen(ser_fd, 5) < 0)
	{
		printf("listen server socket error \n");
		return -1;
	}

	//accept是阻塞等待连接socket ==》返回fd
	struct sockaddr_in cli_addr; 
	int cli_len = sizeof(struct sockaddr_in);
	int conn_fd = -1;
	conn_fd = accept(ser_fd, (struct sockaddr*)&cli_addr, &cli_len);
	if(conn_fd < 0)
	{
		printf("accept error \n");
		close(ser_fd);
		return -1;
	}

//这里仅仅做测试，1对1进行发送消息
	//先发送  再接收
	char * buff = "hello client \0";
	int len  = strlen(buff);
	printf("send buff [%s], len [%d] \n", buff, len);
	char * recv_buff = (char *)malloc(20);
	//这样设计的话  client崩溃，就会挂
	while(1)
	{
		//发送 到连接的fd
		int send_len = -1;
		send_len = send(conn_fd, buff, len, 0);
		if(send_len < 0)
		{
			printf("send error %d. \n", send_len);
		}else
		{
			printf("send buff [%s] sendlen[%d]. \n", buff, send_len);
		}

		//接收 到连接的fd
		int recv_len = -1;
		recv_len = recv(conn_fd, recv_buff, 20, 0);
		if(recv_len < 0)
		{
			printf("recv error %d \n ", recv_len);
		}else
		{
			printf("recv buff [%s] sendlen[%d]. \n", recv_buff, recv_len);
		}
		sleep(1);
	}
	free(recv_buff);
	close(conn_fd);
	close(ser_fd);
	//有连接后开始发送消息
	//死循环等待 发送和接收

	return 0;
}