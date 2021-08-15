//线程回调函数实际上是真正的消费者，触发任务的消费
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "0609_thread_pool_module.h"
//把节点管理的接口也加进来
#define LIST_ADD(item, list) do{ 	\
	item->prev = NULL;				\
	item->next = list;				\
	list = item;					\
}while(0)

#define LIST_REMOVE(item, list) do{		\
	if(item->prev != NULL)				\
		item->prev->next = item->next;	\
	if(item->next != NULL)				\
		item->next->prev = item->prev;	\
	if(list == item)					\
		list = item->next;				\
	item->prev = item->next = NULL;		\
}while(0)


static void * nthreads_callback(void * arg)
{
	NWORKER * onethread = (NWORKER*)arg;
	while(1)
	{
		pthread_mutex_lock(&onethread->manager->tasks_mutex);
		while(onethread->manager->task_t == NULL)
		{
			if(onethread->is_shutdown == 1)
			{
				break; 
			}
			pthread_cond_wait(&onethread->manager->tasks_cond, &onethread->manager->tasks_mutex);
		}

		if(onethread->is_shutdown == 1)
		{
			pthread_mutex_unlock(&onethread->manager->tasks_mutex);
			break;
		}

		NTASKS* task = onethread->manager->task_t;
		if(task != NULL)
		{
			LIST_REMOVE(task, onethread->manager->task_t);
			// if(task->prev != NULL)
			// 	task->prev->next = task->next;
			// if(task->next != NULL)
			// 	task->next->prev = task->prev;
			// if(task == onethread->manager->task_t)
			// 	onethread->manager->task_t = task->next;
			// task->prev = task->next = NULL;
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
			LIST_ADD(onethread, onethread->manager->threads_t);
			//这实际上是链表的一种技巧 头插法
			// onethread->prev = NULL;
			// onethread->next = onethread->manager->threads_t;
			// onethread->manager->threads_t = onethread;

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

	NWORKER * onethread = NULL;
	for(onethread = manager->threads_t; onethread != NULL; onethread= onethread->next)
	{
		onethread->is_shutdown = 1;
	}
	//并唤醒所有的线程 
	pthread_mutex_lock(&manager->tasks_mutex);
	manager->threads_t = NULL;

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
		LIST_ADD(task, manager->task_t);
		// task->prev = NULL;
		// task->next = manager->task_t;
		// manager->task_t = task;
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
	pthread_mutex_lock(&manager->tasks_mutex);
	onetask = manager->task_t;
	NTASKS * tmp = NULL;
	while(onetask != NULL)
	{
		tmp = onetask;
		printf("destory_not_exec_task : [%d] \n", *(int*)tmp->data_para);
		onetask = onetask->next;
		free(tmp);
	}
	pthread_mutex_unlock(&manager->tasks_mutex);
	return 0;
}