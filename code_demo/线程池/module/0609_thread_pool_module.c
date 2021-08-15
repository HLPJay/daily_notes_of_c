//对接口实现封装  实现模块化  这里可以思考nginx的模块化实现

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <unistd.h>
#include "0609_thread_pool_module.h"

/*****************************
模块：其实就是定义一套可以公用的模型。
	1：定义基础的可共用的结构，供模块调度初始化公共使用
	2：定义可公用的函数，这里只涉及了构造函数和析构函数
	3：通过基础结构，可以获取真正结构的地址和数据结构 
******************************/

//定义一个基础结构，定义对应的方法，相关函数可以用可变参数传参
typedef struct 
{
	//一个指向真正接口结构首地址标识 +存储要用到的数据结构
	size_t size; 

	//构造函数 返回一个地址 为标识真正定义子类结构的地址
	void* (*ctor)(void*_self, va_list *params);
	//析构函数
	void* (*dtor)(void*_self);
	//其他有必要的函数可以增加
}Base_Struct;

//通过基础结构 初始化实际结构，返回实际结构的地址+数据结构
void* New(const void* _class, ...)
{
	//同样的构造   可以类型强转 统一调度
	const Base_Struct *class = _class;
	//申请结构体的内存 包括存储真正执行的首地址和数据结构
	void * p = calloc(1, class->size); //calloc会自己清空
	assert(p); //做校验

	//p的首位置存储 子类的首地址
	*(const Base_Struct**)p = class;
	//通过可变参数  实现真正子结构的初始化函数
	if(class->ctor)
	{
		va_list params;
		va_start(params, _class); 
		p = class->ctor(p, &params);
		va_end(params); 
	}
	return p;
}

//_class 存储着实际执行子结构的首地址标识和数据结构 
//取出真正的位置  调用析构
void Delete(void* _class)
{
	const Base_Struct **class = _class;

	if(_class && (*class) && (*class)->dtor)
	{
		_class = (*class)->dtor(_class);
	}
	free(_class);
}

//模块中实现的
/**********************************************
使用上文的模块化，调用线程池的逻辑
定义必要的结构 
	1:标识实际执行的结构地址  
	2：标识使用到的数据结构
**********************************************/
//存储的实际函数的地址和要用到的数据结构 
typedef THREAD_MANAGER nthreadpool;
typedef struct _t_thread_pool_module
{
	const void *_; //执向的值必须是常量，不能修改这个常量  可以重新指向
	nthreadpool *pool;
}FUNCADDRESS_AND_DATA;

//定义存储用到的数据结构和相关执行的函数 
typedef struct _t_thread_pool_opera{
	size_t size;  //存储FUNCADDRESS_AND_DATA的大小 实际地址+数据结构 然后返回申请的这个内存
	
	void* (*ctor)(void*_self, va_list *params);
	void* (*dtor)(void*_self);
	//在基础结构的基础上可以新增自己的结构：
	void (*addJob)(void *_self, void *task);
}THREAD_POOL_OPERAS;


void* threadpool_ctor(void *_self, va_list *params);
void* threadpool_dtor(void *_self);
void threadpool_addtask(void *_self, void *task);
//初始化实际的结构和执行函数
const THREAD_POOL_OPERAS thread_pool_operas = {
	sizeof(FUNCADDRESS_AND_DATA), //这个const的地址+数据结构
	threadpool_ctor,  //该函数模块的初始化
	threadpool_dtor,  //该函数模块的析构
	threadpool_addtask, //该函数模块的执行
};

//实现相关的执行函数  实际执行的初始化
//由new函数调用地址的设置和该函数的触发
void* threadpool_ctor(void *_self, va_list *params)
{
	FUNCADDRESS_AND_DATA* data_t = (FUNCADDRESS_AND_DATA*)_self;

	data_t->pool = (nthreadpool*)malloc(sizeof(nthreadpool));
	//这里需要对data_t->pool 中相关的结构做必要的初始化
	int arg = va_arg(params, int);
	printf("get thread nums is %d \n", arg);

	threads_pool_create(data_t->pool, arg);
	return data_t;
}

void* threadpool_dtor(void *_self)
{	
	FUNCADDRESS_AND_DATA* data_t = (FUNCADDRESS_AND_DATA*)_self;
	threads_pool_shutdown(data_t->pool);
	printf("destory thread pool and remove not exec task. \n");
	destory_not_exec_task(data_t->pool);
	//这里可以对剩余的任务进行删除
	free(data_t->pool);
}

void threadpool_addtask(void *_self, void *task) 
{
	FUNCADDRESS_AND_DATA* data_t = (FUNCADDRESS_AND_DATA*)_self;
	NTASKS* jobs = (NTASKS*)task;
	add_to_threads_tasks(data_t->pool, jobs);
}


//真正的main函数中应该实现的
//new和delete在基础构造中已经实现  addJob的调用也要通过接口

int push_to_thread_tasks(void *self,void *task)
{
	THREAD_POOL_OPERAS **thread_opera = self;

	if (self && (*thread_opera) && (*thread_opera)->addJob) {
		(*thread_opera)->addJob(self, task);
		return 0;
	}
	return 1;
}
#define THREADS_NUMS 5
#define THREADS_TASKS_NUMS 20

void task_callback(void* arg)
{
	int data = *(int*)arg;
	printf(" task :%d threadid: %lu \n", data, pthread_self());
	free(arg);
	sleep(1);
}

//实际的调用
const void * operas = &thread_pool_operas;
//可以定义成单例
static void* pthreadPool = NULL; //由new函数申请，真正的调用都是函数内部转换类型
int main()
{
	//通过new函数申请 
	pthreadPool = New(operas, THREADS_NUMS);
	for(int i=0; i<THREADS_TASKS_NUMS; i++)
	{
		NTASKS* task = (NTASKS*) malloc(sizeof(NTASKS));
		if(task == NULL)
		{
			printf("error: malloc task error");
			break;
		}
		task->task_function = task_callback;
		task->data_para = malloc(sizeof(int));
		*(int *)task->data_para = i;

		push_to_thread_tasks(pthreadPool, task);
	}
	sleep(2);
	//销毁
	threadpool_dtor(pthreadPool);
	getchar();
	return 0;
}