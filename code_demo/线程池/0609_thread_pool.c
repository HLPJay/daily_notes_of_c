#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdarg.h>
#include <pthread.h>

/************************
生产者消费者模型。
	一个线程生产，另外一个线程消费
线程池：
	可以控制线程的生成，终结，个数
	可以管理线程从任务队列中取任务去执行
		线程通过回调函数去取任务执行，实现了线程池的功能
		没有执行完的线程，是不会去取下一个任务的
分析实现：
	1：实现管理线程的结构
	2：实现管理任务的结构
	3：实现统一进行管理的结构

结构分析：
	1:定义相关的结构
	2:实现相关的函数==》固定池子里对任务进行操作
		1：线程池的创建，线程回调函数，实际上是任务的消费
		2：线程池的关闭
		3：任务队列的入队
	3:如果想要管理池子，可以新建一个线程专门管理，实现线程的创建/删除并加入/移除池子中
************************/

//直接用链表控制了线程池的增加与删除，这里没有实现相关功能
typedef struct nthreads_t
{
	//线程池的控制，实际上每个线程回调函数会执行
	//这里用条件变量实现控制唤醒
	pthread_t tid;
	struct nthreads_t *prev;
	struct nthreads_t *next;

	//线程的销毁标志
	int is_shutdown;
	//从管理节点中取队列执行任务
	struct nthread_manager * manager;
}NWORKER;

//用链表控制任务的增加与删除，然后通过线程实现取
typedef struct ntasks_t
{
	//任务队列的执行函数和参数 因为涉及到任务队列本身
	// void (* task_function)(struct ntasks_t *task);
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

//线程回调函数实际上是真正的消费者，触发任务的消费
static void * nthreads_callback(void * arg)
{
	//这里实际上就是任务的消费，取任务
	//1：无任务时要等待
	//2：线程终止要终止线程 ==>
	//3：开始取任务进行执行
	NWORKER * onethread = (NWORKER*)arg;
	while(1)
	{
		//取任务的时候要加锁
		pthread_mutex_lock(&onethread->manager->tasks_mutex);
		//当任务队列为null时，一直等待被唤醒
		while(onethread->manager->task_t == NULL)
		{
			if(onethread->is_shutdown == 1)
			{
				break; //跳出等待循环 
			}
			pthread_cond_wait(&onethread->manager->tasks_cond, &onethread->manager->tasks_mutex);
		}
		//关注线程的关闭
		if(onethread->is_shutdown == 1)
		{
			pthread_mutex_unlock(&onethread->manager->tasks_mutex);
			break;
		}
		//真正的取任务，然后执行回调
		NTASKS* task = onethread->manager->task_t;
		if(task != NULL)
		{
			//取出任务后要从任务列表中移除
			if(task->prev != NULL)
				task->prev->next = task->next;
			if(task->next != NULL)
				task->next->prev = task->prev;
			if(task == onethread->manager->task_t)
				onethread->manager->task_t = task->next;
			task->prev = task->next = NULL;
		}

		pthread_mutex_unlock(&onethread->manager->tasks_mutex);

		if(task == NULL) continue;
		task->task_function(task->data_para);
		free(task);
	}
	printf("pthread exit :%lu \n", onethread->tid);
	//释放关闭线程
	free(onethread);
	pthread_exit(NULL);
}

//创建线程池 定义管理结构体，然后申请线程
int threads_pool_create(THREAD_MANAGER * manager, int thread_nums)
{
	if(manager == NULL)
	{
		printf("nthreads manager is null. \n");
		return -1;
	}
	if(thread_nums < 1)
	{
		thread_nums = 1;
	}
	//对manager 进行必要的初始化
	memset(manager, 0, sizeof(THREAD_MANAGER));
	//静态: 可以把常量PTHREAD_COND_INITIALIZER给静态分配的条件变量.
	//动态: pthread_cond_init函数, 是释放动态条件变量的内存空间之前, 要用pthread_cond_destroy对其进行清理.
	//这里用静态初始化的方式  不关注释放
	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
	memcpy(&manager->tasks_cond, &blank_cond, sizeof(pthread_cond_t));

	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
	memcpy(&manager->tasks_mutex, &blank_mutex, sizeof(pthread_mutex_t));
	printf("cond size: [%lu] mutex size: [%lu] \n", sizeof(pthread_cond_t), sizeof(pthread_mutex_t));
	manager->threads_t = NULL;
	manager->task_t = NULL;
	//开始进行线程创建
	for(int i=0; i<thread_nums; i++)
	{
		//实际就是构造线程结构体，然后适配在管理节点中
		NWORKER * onethread = (NWORKER*)malloc(sizeof(NWORKER));
		if(onethread == NULL)
		{
			printf("create thread struct error \n");
			return -1;
		}
		memset(onethread, 0, sizeof(NWORKER));
		onethread->manager = manager;

		//这里回调函数的参数得传自己的结构
		int ret = pthread_create(&onethread->tid, NULL, nthreads_callback, onethread);
		if(ret != 0)
		{
			printf("create thread error. \n");
			free(onethread);
			return -1;
		}

		printf("create thread [tid = %lu] success \n", onethread->tid);
		{
			//这实际上是链表的一种技巧 头插法
			onethread->prev = NULL;
			onethread->next = onethread->manager->threads_t;
			onethread->manager->threads_t = onethread;
		}
	}
	return 0;
}
//关闭线程池
int threads_pool_shutdown(THREAD_MANAGER * manager)
{
	if(manager == NULL)
	{
		printf("ERROR manager is null. \n");
		return -1;
	}

	//循环给threads池子关闭标识置为1
	//管理节点一直指向的是 线程池的第一个节点
	NWORKER * onethread = NULL;
	for(onethread = manager->threads_t; onethread != NULL; onethread= onethread->next)
	{
		onethread->is_shutdown = 1;
	}
	//并唤醒所有的线程 
	pthread_mutex_lock(&manager->tasks_mutex);
	manager->threads_t = NULL;
	// manager->task_t = NULL; 这个其实在关闭后应该做释放的
	// 可以放在主线程中去做剩余任务和manager的释放
	pthread_cond_broadcast(&manager->tasks_cond);
	pthread_mutex_unlock(&manager->tasks_mutex);
}

//加入任务队列
int add_to_threads_tasks(THREAD_MANAGER * manager, NTASKS* task)
{
	if(manager == NULL)
	{
		printf("error add_to_threads_tasks, manager is null. \n");
		return -1;
	}
	pthread_mutex_lock(&manager->tasks_mutex);
	{
		task->prev = NULL;
		task->next = manager->task_t;
		manager->task_t = task;
	}
	pthread_cond_signal(&manager->tasks_cond);
	pthread_mutex_unlock(&manager->tasks_mutex);
	return 0;
}

int destory_not_exec_task(THREAD_MANAGER * manager)
{
	if(manager == NULL)
	{
		printf("destory_not_exec_task error manager is null \n");
		return -1;
	}
	NTASKS * onetask = NULL;
	// for(onetask = manager->task_t; onetask != NULL; onetask=onetask->next)
	// {
	// 	printf("destory_not_exec_task : [%d] \n", *(int*)onetask->data_para);
	// 	free(onetask->data_para);
	// }
	onetask = manager->task_t;
	NTASKS * tmp = NULL;
	while(onetask != NULL)
	{
		tmp = onetask;
		printf("destory_not_exec_task : [%d] \n", *(int*)tmp->data_para);
		onetask = onetask->next;
		free(tmp);
	}
	return 0;
}

#define THREADS_NUMS 5
#define THREADS_TASKS_NUMS 20
//实现线程池的逻辑

void task_callback(void* arg)
{
	int data = *(int*)arg;
	printf(" task :%d threadid: %lu \n", data, pthread_self());
	free(arg);
	sleep(1);
}

int main(int argc, char* argv)
{
	THREAD_MANAGER* manager = (THREAD_MANAGER*)malloc(sizeof(THREAD_MANAGER));
	//THREAD_MANAGER manager; //用&manager传参
	//初始化线程池
	threads_pool_create(manager, THREADS_NUMS);

	//塞任务，其实新建一个线程专门做
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

		add_to_threads_tasks(manager, task);
	}

	sleep(2);
	//调用关闭
	threads_pool_shutdown(manager);
	sleep(1);
	destory_not_exec_task(manager);
	manager->task_t = NULL;
	free(manager);
	// getchar();

	return 0;
}
