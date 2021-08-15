#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <stdarg.h>
//统一的函数执行体 
//size 保存了子结构地址和需要的数据结构
typedef struct 
{
	size_t size;
	void* (*ctor)(void*_self, va_list *params);
	void* (*dtor)(void*_self);
}Base_Struct;
 
void* New(const void* _class, ...);
void Delete(void* _class);



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

//这个对象报错了数据结构和固定的操作函数  可以在此基础+设定New函数的使用实现类似单例的功能
extern const void *module_test;
#endif
