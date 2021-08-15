#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//实现udp消息的接收
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("use ./recv bind_port\n");
		return -1;
	}

	int port = atoi(argv[1]);
	if(port < 1025 || port>65535)
	{
		printf("port number range 1025 to 65535 \n");
		return -1;
	}

	//定义socket，绑定端口 
	int udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sock_fd < 0)
	{
		printf("create socket failed. \n");
		return -1;
	}

	//网络通信都使用大端格式 让系统检测，自动绑定本地ip
	struct sockaddr_in local_addr = {0};
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	int ret = bind(udp_sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr));
	if(ret<0)
	{
		printf("bind port error");
		close(udp_sock_fd);
		return -1;
	}else
	{
		printf("recv ready:\n");
	}


	char buff[1024] ={0};
	struct sockaddr_in src_addr = {0};
	int len  = sizeof(src_addr);
	//使用recvfrom进行接收
	while(1)
	{
		//返回0 连接终止 
		ret = recvfrom(udp_sock_fd, buff, sizeof(buff), 0, (struct sockaddr*)&src_addr, &len);
		if(ret == -1)
		{
			printf("recvfrom error \n");
			break;
		}
		printf("[%s, %d] ", inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));
		printf("buff=%s \n", buff);
		if(strcmp(buff, "exit") == 0)
		{
			break;
		}
		memset(buff, 0, sizeof(buff));
	}
	close(udp_sock_fd);
	return 0;
}