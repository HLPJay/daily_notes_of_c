#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//./test_udp_multicast_recv 224.12.12.12 1234
//接收端 组的ip 自己要绑定的ip和端口号
int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("use ./send group_ip bind_port\n");
		return -1;
	}

	int port = atoi(argv[2]);
	if(port < 1025 || port>65535)
	{
		printf("port number range 1025 to 65535 \n");
		return -1;
	}

	//创建socket 
	int udp_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
	if(udp_socket_fd < 0 )
	{
		perror("creat socket fail\n");
		return -1;
	}

	//加入组播 组播地址224.0.0.0~239.255.255.255
	struct ip_mreq group = {0};
	group.imr_multiaddr.s_addr = inet_addr(argv[1]);
	group.imr_interface.s_addr = inet_addr("0.0.0.0");
	//设置组播模式
	int ret = setsockopt(udp_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group));
	if(ret < 0)
	{
		printf("setsockopt error . \n");
		close(udp_socket_fd);
		return -1;
	}else
	{
		printf("has add to group [%s],my port is [%d] \n", argv[1], port);
	}

	//绑定端口
	struct sockaddr_in bind_addr = {0};
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(port);
	bind_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

	ret = bind(udp_socket_fd, (struct sockaddr*)&bind_addr, sizeof(bind_addr));
	if(ret < 0)
	{
		printf("bind error \n");
		close(udp_socket_fd);
		return -1;
	}

	struct sockaddr_in src_addr = {0};
	char recv_buff[1024] = {0};
	int len = sizeof(src_addr);
	//组播是udp的内部逻辑 这里直接接收
	while(1)
	{
		recvfrom(udp_socket_fd, recv_buff, sizeof(udp_socket_fd), 0, (struct sockaddr*)&src_addr, &len);
		printf("src_ip:[%s],src_port:[%d]",inet_ntoa(src_addr.sin_addr),ntohs(src_addr.sin_port));//打印消息发送者的IP信息
		printf(" buf=%s\n", recv_buff);//打印消息
		if(strcmp(recv_buff, "exit") == 0)
		{
			break;//当接收到"exit"时退出循环
		}
		bzero(recv_buff,sizeof(recv_buff));
	}
	close(udp_socket_fd);
	return 0;
}