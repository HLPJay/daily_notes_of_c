#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"
#include "test_module.h"

//new函数真正调用这个函数  这里做真正初始化
//new函数实际做了这里真正的指向 和数据的传递
//可以通过传参初始化这里的结构体
static void *test_module_expand_create(void *_self, va_list *params) {
    TEST_MODULE *data_t = (TEST_MODULE*)_self;
    TEST_STRUCT * data = (TEST_STRUCT*)malloc(sizeof(TEST_STRUCT));
    int arg = va_arg(params, int);
    data->id = arg;
    memcpy(data->name, "mytest", 6);
    printf("test_module_create :%d, %s \n", data->id, data->name);
    data_t->data = data;
    return data_t;
}
 
//Delete 函数真正掉这个函数  做对应的销毁
static void *test_module_expand_destory(void *_self) {
    const Base_Struct_expand **class = _self;
    if(_self && (*class) && (*class)->dtor)
    {
      _self = (*class)->show(_self);
    }
  	//目的是销毁create中对应的结构
    TEST_MODULE *data_t = (TEST_MODULE*)_self;
  	printf("test_module_expand_destory :%d, %s \n", data_t->data->id, data_t->data->name);
    if(data_t->data != NULL)
    {
      free(data_t->data);
      printf("free data success \n");
    }
  	return data_t;
}

static void *test_module_expand_show(void *_self)
{
    TEST_MODULE *data_t = (TEST_MODULE*)_self;
    //目的是销毁create中对应的结构
    printf("test_module_expand_show :%d, %s \n", data_t->data->id, data_t->data->name);
    return data_t;
}

//按照真正的结构进行组织
static const Base_Struct_expand moudle_t = {
  	sizeof(TEST_STRUCT),
  	test_module_expand_create,
  	test_module_expand_destory,
    test_module_expand_show,
};


//把该结构地址共享出去，基类通过地址强行转换执行到这里的函数
const void * module_test_expand = &moudle_t;
