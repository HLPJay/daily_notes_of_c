#ifndef _BASE_H_
#define _BASE_H_

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

typedef struct 
{
	size_t size;
	void* (*ctor)(void*_self, va_list *params);
	void* (*dtor)(void*_self);
	void* (*show)(void*_self);
}Base_Struct_expand;

void Show(void* _class);
#endif
