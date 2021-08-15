#ifndef __THREAD_POOL_MODULE_
#define __THREAD_POOL_MODULE_

#include <pthread.h>
//线程池相关的数据结构
#pragma pack (1) 
typedef struct nthreads_t
{
	pthread_t tid;
	struct nthreads_t *prev;
	struct nthreads_t *next;

	int is_shutdown;
	struct nthread_manager * manager;
}NWORKER;

typedef struct ntasks_t
{
	void (* task_function)(void *task);
	void * data_para;

	struct ntasks_t *prev;
	struct ntasks_t *next;
}NTASKS;

typedef struct nthread_manager
{
	//管理线程和任务
	struct nthreads_t * threads_t; //主要是为了遍历线程关闭线程用的
	struct ntasks_t * task_t;

	//消费任务时需要加锁 
	pthread_mutex_t tasks_mutex;
	//控制唤醒线程的个数
	pthread_cond_t tasks_cond;
}THREAD_MANAGER;
#pragma pack ()


//线程池相关的函数
int threads_pool_create(THREAD_MANAGER * manager, int thread_nums);
int threads_pool_shutdown(THREAD_MANAGER * manager);
int add_to_threads_tasks(THREAD_MANAGER * manager, NTASKS* task);
int destory_not_exec_task(THREAD_MANAGER * manager);

#endif //__THREAD_POOL_MODULE_