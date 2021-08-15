#ifndef _TEST_MODULE_H_
#define _TEST_MODULE_H_

#include <stdarg.h>

//直接在基础定义模型使用，不扩展其他的函数接口

//定义我们的结构体 我们实际的数据
typedef struct _t_test_struct
{
	int id;
	char name[8];
}TEST_STRUCT;

//定义符合结构的结构体
typedef struct _t_test_module
{
	const void *_;
	TEST_STRUCT * data;
}TEST_MODULE;

//结构定义入参 以此为标准申请内存和指针指向
extern const void *module_test;

#endif
