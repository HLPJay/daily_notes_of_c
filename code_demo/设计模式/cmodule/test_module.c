#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "test_module.h"

//new函数真正调用这个函数  这里做真正初始化
//new函数实际做了这里真正的指向 和数据的传递
//可以通过传参初始化这里的结构体
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
