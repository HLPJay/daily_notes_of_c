#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//udp的发送端，传入ip和端口号
int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("use ./send dst_ip dst_port \n");
		return -1;
	}

	int port = atoi(argv[2]);
	if(port < 1025 || port > 65535)
	{
		printf("port number range 1025 to 65535 \n");
		return -1;
	}

	//1：创建端口号
	int udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sock_fd == -1)
	{
		printf("create udp socket error \n");
		return -1;
	}

	//2：直接设置对端的ip和端口号
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port); //主机字节序转为网络字节序
	dest_addr.sin_addr.s_addr = inet_addr(argv[1]); //点分十进制转为无符号长整形

	//3：向设置的对端进行消息发送
	char buff[1024] = {0};
	while(1)
	{
		printf("please input msg :");
		scanf("%s", buff);
		//第四个参数和send的最后一个参数类似 一般设置为0
		sendto(udp_sock_fd, buff, strlen(buff), 0,  (struct sockaddr*)&dest_addr, sizeof(dest_addr));
		if(strcmp(buff, "exit") == 0)
		{
			break;
		}
		memset(buff, 0 ,sizeof(buff));
	}

	close(udp_sock_fd);
	return 0;
}