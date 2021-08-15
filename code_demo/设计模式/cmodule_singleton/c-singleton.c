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


static void *test_module_create(void *_self, va_list *params) {
    TEST_MODULE *data_t = (TEST_MODULE*)_self; //这个的实际指向就是moudle_t定义


    TEST_STRUCT * data = (TEST_STRUCT*)malloc(sizeof(TEST_STRUCT));
    data_t->data = data;

    int arg = va_arg(params, int);
    printf("get para is %d. \n",arg);
    data->id = arg;
    memcpy(data->name, "mytest", 6);
    printf("test_module_create :%d, %s \n", data->id, data->name);
    return data_t;
}
 
//Delete 函数真正掉这个函数  做对应的销毁
static void *test_module_destory(void *_self) {
  	TEST_MODULE *data_t = (TEST_MODULE*)_self;
  	//目的是销毁create中对应的结构
  	printf("test_module_destory :%d, %s \n", data_t->data->id, data_t->data->name);
  	if(data_t->data != NULL)
  	{
  		free(data_t->data);
  		printf("free data success \n");
  	}
  	return data_t;
}

//按照真正的结构进行组织
static const Base_Struct moudle_t = {
	sizeof(TEST_MODULE),
	test_module_create,
	test_module_destory,
};

//把该结构地址共享出去，基类通过地址强行转换执行到这里的函数
const void * module_test = &moudle_t;





