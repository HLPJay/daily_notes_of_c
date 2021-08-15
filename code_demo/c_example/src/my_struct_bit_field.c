#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************
结构体位域的理解：
	有些信息存储时，并不需要8字节，使用几个或者1个bit位，可以通过位域实现。

其他概念：
	1：操作系统 char对其为1， short为2，int,float,double类型，其自身对齐值为4，单位字节
		32bit的操作系统，每次可以读取32bit,即4字节
	2：可以用#pragma pack 控制对齐方式
*********************************************************/


//要注意位域 如果一个字节存储不在一个字节，不能跨过两个字节，从下一单元开始。

//看看占多少字节 4
typedef struct _t_test_bit
{
	int a:4 ;
	int b:12;
}BIT_TEST_T;
//1
typedef struct _t_test_bit_best1
{
	char a:1;
	char b:3;
	char c:4;
}BIT_BEST_T1;

//2 
typedef struct _t_test_t
{
	char a:4;
	char b:5;
}TEST_T;
#pragma pack (1)

//最好的使用方法： 凑够8字节  1字节
typedef struct _t_test_bit_best
{
	char a:1;
	char b:3;
	char c:4;
}BIT_BEST_T;

//2
typedef struct _t_test_bit1
{
	int a:4 ;
	int b:12;
}BIT_TEST_T1;
//2
typedef struct _t_test_bit2
{
	int a:4 ;
	int b:4;
	int c:8;
}BIT_TEST_T2;

//可以有空域 2
typedef struct _t_test_bit3
{
	char a:3;
	char :5;
	char b;
}BIT_TEST_T3;
#pragma pack ()


int print_struct()
{
	printf(" 4 size:%lu \n", sizeof(BIT_TEST_T));
	printf(" 1 size:%lu \n", sizeof(BIT_BEST_T1));
	printf(" 2 size:%lu \n", sizeof(TEST_T));

	printf("1 size: %lu \n", sizeof(BIT_BEST_T));
	printf("2 size: %lu \n", sizeof(BIT_TEST_T1));

	printf("2 sizeof %lu \n", sizeof(BIT_TEST_T2));
	printf("2 sizeof %lu \n", sizeof(BIT_TEST_T3));
	return 0;
}

int main()
{
	print_struct();
	return 0;
}