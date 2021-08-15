#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//广播   实现给该网段下所有ip发送，该网段下相关端口都能接收到
//发送时开启广播发送就好
// 注意 广播地址
// ./test_udp_broadcast_send 192.168.11.255 1234
int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("use ./send broadcast_ip dst_port \n");
		return -1;
	}

	int port = atoi(argv[2]);
	if(port < 1025 || port > 65535)
	{
		printf("port number range 1025 to 65535 \n");
		return -1;
	}

	//1.创建UDP  socket 
    int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd == -1)
    {
        printf("create socket failed ! error message :%s\n", strerror(errno));
        return -1;
    }
		
	
	//2.开启发送广播数据功能
	int on = 1; //开启
	int ret = setsockopt(udp_socket_fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	if(ret < 0)
	{
		perror("setsockopt fail\n");
		return -1;
	}
	
	//3.设置当前网段的广播地址 
    struct sockaddr_in bro_addr = {0};
    bro_addr.sin_family = AF_INET;
    bro_addr.sin_port = htons(port);
    bro_addr.sin_addr.s_addr = inet_addr(argv[1]);  //设置为广播地址
	
    char buf[1024] = {0};//消息缓冲区
	
	//4 发送数据  
	while(1)
	{
		printf("Please input broadcast msg:");
		scanf("%s", buf);//获取要发送的消息
		sendto(udp_socket_fd, buf, strlen(buf), 0, (struct sockaddr *)&bro_addr, sizeof(bro_addr)); 
		if(strcmp(buf, "exit") == 0)
		{
			break;//退出循环
		}
		bzero(buf, sizeof(buf));
	}
	
	//5.关闭网络通信
	close(udp_socket_fd);
	return 0;
}