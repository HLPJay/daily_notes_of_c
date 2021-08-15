#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//组播 加入组中，实现给组发送，则组中该端口都能收到

//可以填组的ip  port  msg进行发送
int main(int argc, char* argv[])
{
	//1 创建UDP通信socket
    int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd == -1)
    {
        printf("create socket failed ! error message :%s\n", strerror(errno));
        return -1;
    }
    
	//地址信息
    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;
	
	int dest_port = 0;
	char dest_ip[32] = {0};
    char buf[1024] = {0};
	
	//2 发送消息
    while (1)
    {
		
		printf("ip port msg:");
		scanf("%s %d %s", dest_ip, &dest_port, buf);
		//设置目的IP端口
		dest_addr.sin_port	= htons(dest_port);   //网络通信都使用大端格式
		dest_addr.sin_addr.s_addr =  inet_addr(dest_ip);  //32位的整形   
	
        if (sendto(udp_socket_fd, buf, strlen(buf), 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) == -1)
        {
            printf("sendto failed ! error message :%s\n", strerror(errno));
            break;
        }
		if(strcmp(buf, "exit") == 0)
		{
			break;
		}
        memset(buf,0,sizeof(buf));
    }
	
	//3 关闭通信socket
    close(udp_socket_fd);
    return 0;
}