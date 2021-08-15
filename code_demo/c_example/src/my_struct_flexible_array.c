/*************************************
柔性数组的使用demo：
	一般使用在通信定义协议中。
	结构体最后一个位置标识指针，通过前面的数据长度控制指针数据和长度，
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _t_flexible
{
	int type;
	int len;
	char data[];   //char *data;         //char data[0];
}FLEXIBLE_T;

int struct_test(const char * str, int type, int len);
int exec_data(char * data,  int len);

int main()
{
	const char * str = "my test of data.";
	struct_test(str, 1, strlen(str));
	str = "other test.";
	struct_test(str, 2, strlen(str));
	return 0;
}


int struct_test(const char * str, int type, int len)
{
	FLEXIBLE_T * data_t = (FLEXIBLE_T *)malloc(sizeof(FLEXIBLE_T) +len);
	if(data_t == NULL)
	{
		printf(" malloc error \n");
		return -1;
	}
	memset(data_t, 0, sizeof(FLEXIBLE_T) +len);
	data_t->type = type;
	data_t->len = len;
	memcpy(data_t->data, str, len); //这里直接塞了一个字符串  可以是任何的二进制数据
	// printf("    %s \n", data_t->data);
	//这里其实data_t 就已经构造了我们需要的协议  包含类型，长度，以及我们需要的数据
	//可以对这个结构体指针 转成基本类型 进行处理，如去做网络处理
	int data_len = sizeof(FLEXIBLE_T) +len;
	//这里就是我们的协议头指针和真正数据的长度
	//网络处理场景中  可以用send函数进行发送，长度即为data_len。
	exec_data((char*)data_t, data_len);
	//如果网络通信 这里是要释放结构体内容的
	return 0;
}

//这里的入参一般是recv接收到的数据
int exec_data(char * data,  int len)
{
	//网络场景中 可以进行接收   如 recv函数
	//因为可以确定首地址，可以用类型强转的的方式做头部的解析。
	if(len < sizeof(FLEXIBLE_T))
	{
		printf("error of data len \n");
		return -1;
	}
	FLEXIBLE_T * data_t = (FLEXIBLE_T*)data; //首地址和长度确定，强转类型解析
	printf("struct type:%d len:%d \n", data_t->type, data_t->len);
	char *str_data = (char*)malloc(data_t->len);
	memset(str_data, 0, data_t->len);
	memcpy(str_data, data_t->data, data_t->len);
	printf("struct data is : %s \n", str_data);
	free(str_data);
	if(data != NULL)
	{
		free(data);
	}
	return 0;

}