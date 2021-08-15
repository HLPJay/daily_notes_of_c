#include <stdlib.h>
#include <assert.h>
#include "base.h"

void* New(const void* _class, ...)
{
	const Base_Struct *class = _class;
	void * p = calloc(1, class->size); 
	assert(p); 
	//注意  这里是重点  保存了实际的执行操作结构体的地址
	*(const Base_Struct**)p = class; //两层取值可以取到实际结构体位置
	if(class->ctor)
	{
		va_list params;
		va_start(params, _class); 
		p = class->ctor(p, &params);
		va_end(params); 
	}
	return p;
}

//取地址里存的值 找到对应的地址执行
void Delete(void* _class)
{
	const Base_Struct **class = _class;

	if(_class && (*class) && (*class)->dtor)
	{
		_class = (*class)->dtor(_class);
	}
	free(_class);
}

void Show(void* _class)
{
	const Base_Struct_expand ** class = _class;
	if (_class && (*class) && (*class)->show) {    
        (*class)->show(_class);
    }
}