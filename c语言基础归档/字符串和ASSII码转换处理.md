## 字符串转ASSII字符编解码

最近在做tetra网络相关的项目时，涉及到字符串转ASSII的方式进行消息的编码方式，以及对应的解码，就整理一下：

​		1：其实就是把字符串中每个字符，取高四位，存储对应的十六进制值对应的字符串。

​		2：反向解析，把ASSII字符串每两个做拼接，转成十进制求出assii对应的字符进行存储。

## 涉及到一个没有关注到的知识点：位运算符：左移（<<）和右移（>>）

注意：左移和右移并不会改变指针指向，配合&，|，^可以对数据做一些列操作

​	>>  右移符号，对应的二进制位，正数左边补0，负数左边补1

​	<<  左移符号，对应的二进制位，右边补0，

## 字符处理：

​	取字符c高四位对应的数值：    (c>>4) & 0xF   (先右移4位，即左边二进制补齐4位，求得高四位对应数值)

​	取字符c低四位对应的数值：           c  & 0x0F

## 源码demo：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//最近在做tetra网络相关的项目时，涉及到字符串转ASSII的方式进行消息的编码方式，以及对应的解码，就整理一下
int str2ASSIIHex(const char* str, const int data_len, char* out_buff);
int ASSII2StrHex(const char* assii_str, const int assii_len, char * data, int len);

int main()
{
	const char* str= "ABCJKL";
	char * assi_str = (char*)malloc((strlen(str)*2));
	if(assi_str == NULL)
	{
		printf("malloc error \n");
		return -1;
	}
	printf("src str:%s \n", str);
	str2ASSIIHex(str, strlen(str), assi_str);
	printf("ASSII_STR:%s %lu\n", assi_str,strlen(assi_str));

	char * dst_str = (char*)malloc(strlen(str));
	if(dst_str == NULL)
	{
		printf("malloc dst str error \n");
		free(assi_str);
		return -1;
	}
	ASSII2StrHex(assi_str, strlen(assi_str), dst_str, strlen(str));
	printf("dst str:%s %lu \n", dst_str, strlen(dst_str));
	free(assi_str);
	free(dst_str);
	return 0;
}

//把字符串转为assii对应的十六进制内容进行存储   ==》取高四位 存对应的字符    取低四位  存对应的字符
//实际上字符串的长度扩大了2倍
int str2ASSIIHex(const char* str, const int data_len, char* out_buff)
{
	if(str == NULL || data_len <= 0 || out_buff == NULL)
	{
		return -1;
	}

	char hexval[16] = {	'0', '1', '2', '3', '4', '5', '6', '7', 
						'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
						
	//对字符串数据进行处理 取高四位，低四位然后拼接
	for(int j =0; j<data_len; j++)
	{
		out_buff[j*2] = hexval[(str[j]>>4) & 0xF]; //取高四位
		out_buff[j*2+1] = hexval[(str[j]) & 0x0F]; //取低四位
	}
	//out_buff就是最终转出来的十六进制的数
	return 0;
}

//直接把字符转成数字  然后求十六进制
static int char2num(char c)
{
	int ret = -1;
	switch(c)
	{
		case '0': ret = 0; break;
		case '1': ret = 1; break;
		case '2': ret = 2; break;
		case '3': ret = 3; break;
		case '4': ret = 4; break;
		case '5': ret = 5; break;
		case '6': ret = 6; break;
		case '7': ret = 7; break;
		case '8': ret = 8; break;
		case '9': ret = 9; break;
		case 'a':
		case 'A': ret = 10; break;
		case 'b':
		case 'B': ret = 11; break;
		case 'c':
		case 'C': ret = 12; break;
		case 'd':
		case 'D': ret = 13; break;
		case 'e':
		case 'E': ret = 14; break;
		case 'f':
		case 'F': ret = 15; break;
	}
	return ret;
}
//把上文存储的 ASSII转为字符串
int ASSII2StrHex(const char* assii_str, const int assii_len, char * data, int len)
{
	if(assii_str == NULL || data == NULL || assii_len/2 >len)
	{
		printf("paras is null.");
		return -1;
	}

	int assi_num;
	for(int i = 0; i<assii_len/2; i++)
	{
		assi_num = char2num(assii_str[2*i])*16 + char2num(assii_str[2*i+1]);
		data[i] = assi_num;
	}
	return 0;
}
/****************************
//运行结果   注意 这里其实对应的是assii字符对应的十六进制的值字符
src str:ABCJKL 
ASSII_STR:4142434a4b4c 12
dst str:ABCJKL 6 
*****************************/
```

