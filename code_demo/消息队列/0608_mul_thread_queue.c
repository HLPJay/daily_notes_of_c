
//实现多线程安全的消息队列 入队出队的时候加锁
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>

//定义数据节点 
typedef struct _t_queue_node
{
	struct _t_queue_node * next;
	char data[1024]; //这里应该是要处理的结构体
}QUEUE_NODE;


//定义管理节点 指向头和尾
typedef struct _t_queue
{
	struct _t_queue_node * head;
	struct _t_queue_node * tail;
}QUEUE;

typedef struct _t_queue_manager
{
	QUEUE * queue;
	int count;
	pthread_mutex_t lock;
}QUEUE_MANAGER;

QUEUE_MANAGER * init_queue()
{
	//创建一个哨兵节点吧 实际数据区
	QUEUE_NODE * node = (QUEUE_NODE*)malloc(sizeof(QUEUE_NODE));
	if(node == NULL)
	{
		printf("create sentinel node error");
		return NULL;
	}
	memset(node, 0, sizeof(QUEUE_NODE));
	node->next = NULL;
	//管理队列
	QUEUE * queue = (QUEUE*)malloc(sizeof(QUEUE));
	if(queue == NULL)
	{
		printf("create queue error. \n");
		free(node);
		return NULL;
	}
	queue->head = queue->tail = node;
	queue->head->next = NULL;
	queue->tail->next = NULL;

	//真正的管理节点
	QUEUE_MANAGER* queue_manager = (QUEUE_MANAGER*)malloc(sizeof(QUEUE_MANAGER));
	if(queue_manager == NULL)
	{
		printf("create queue manager error. \n");
		return NULL;
	}
	memset(queue_manager, 0, sizeof(QUEUE_MANAGER));
	queue_manager->queue = queue;
	pthread_mutex_init(&queue_manager->lock , NULL);
	queue_manager->count=0;
	return queue_manager;
};

int Destory_queue(QUEUE_MANAGER *queue_manager)
{
	if(queue_manager == NULL || queue_manager->queue == NULL)
	{
		printf("Destory_queue error. \n");
		return -1;
	}
	QUEUE *queue = queue_manager->queue;
	QUEUE_NODE * node = NULL;
	while(queue->head->next != NULL)
	{
		node = queue->head->next;
		queue->head->next = node->next;
		node->next = NULL;
		free(node);
		node = NULL;
	}
	
	free(queue->head);
	pthread_mutex_destroy(&queue_manager->lock);
	free(queue);
	free(queue_manager);
	return 0;
}
//入队 创建新的节点  加入到tail后面
int en_queue(QUEUE_MANAGER *queue_manager, const char * data, int len)
{
	if(queue_manager == NULL || queue_manager->queue == NULL)
	{
		printf("en_queue manager is null.\n");
		return -1;
	}
	QUEUE_NODE * node = (QUEUE_NODE*)malloc(sizeof(QUEUE_NODE));
	if(len >1024) len =1024;
	memcpy(node->data, data, len);
	node->next = NULL;

	pthread_mutex_lock(&queue_manager->lock);
	queue_manager->queue->tail->next = node;
	queue_manager->queue->tail = node;
	queue_manager->count++;
	pthread_mutex_unlock(&queue_manager->lock);
	return 0;
};

//出队 返回队列中的第一个
int get_queue_size(QUEUE_MANAGER *queue_manager);
char * de_queue(QUEUE_MANAGER *queue_manager)
{
	if(queue_manager ==NULL ||queue_manager->queue->head == NULL ||queue_manager->queue->head->next == NULL)
	{
		//printf("queue is null. \n");
		return NULL;
	}
	pthread_mutex_lock(&queue_manager->lock);
	if(queue_manager->queue->tail == queue_manager->queue->head->next)
	{
		queue_manager->queue->tail = queue_manager->queue->head;
	}
	QUEUE_NODE * node = queue_manager->queue->head->next;
	queue_manager->queue->head->next = node->next;
	queue_manager->count--;
	pthread_mutex_unlock(&queue_manager->lock);

	int len = strlen(node->data);
	printf(" dequeue data:[%d][%s] [%d] \n", len, node->data,get_queue_size(queue_manager));
	char* data = (char*)malloc(len);
	memcpy(data, node->data, len);
	free(node);
	return data;
};
//返回0 代表非空 有元素
int is_queue_empty(QUEUE_MANAGER *queue_manager)
{
	if(queue_manager == NULL)
	{
		return -1;
	}
	return (queue_manager->count == 0) ? 0:1;
};


int get_queue_size(QUEUE_MANAGER *queue_manager)
{
	if(queue_manager == NULL)
	{
		return -1;
	}
	return queue_manager->count;
};

//1：多线程传参传进去
//2：定义全局的变量进行操作
void * enter_call_back(void* arg);
void* out_call_back(void * arg);
int main()
{
	QUEUE_MANAGER * manager= init_queue();
	if(manager == NULL)
	{
		printf("init_queue error \n");
		return -1;
	}

	printf("init queue [0]: %d \n", is_queue_empty(manager));
	printf("init queue size[0] :%d \n",get_queue_size(manager));
	
	//创建多线程，把需要的队列传进去
	pthread_t tids_in[3], tids_out[2];
	int ret = -1;
	for(int i=0; i<3; i++)
	{
		ret = pthread_create(&tids_in[i], NULL, enter_call_back, (void*)manager);
		if(ret != 0)
		{
			printf("pthread_create error. \n");
			return -1;
		}
	}

	for(int i=0; i<2; i++)
	{
		ret = pthread_create(&tids_out[i], NULL, out_call_back, (void*)manager);
		if(ret != 0)
		{
			printf("pthread_create error. \n");
			return -1;
		}
	}

	for(int i=0; i<3; i++)
	{
		pthread_join(tids_in[i], NULL);
	}

	for(int i=0; i<2; i++)
	{
		pthread_join(tids_out[i], NULL);
	}
	printf("Destory_queue start .\n");
	if(Destory_queue(manager) <0)
	{
		printf("Destory_queue error \n");
		return -1;
	}
	return 0;
}
//专门实现入队
void * enter_call_back(void* arg)
{
	QUEUE_MANAGER * manager = (QUEUE_MANAGER *)arg;
	if(manager == NULL)
	{
		printf("thread get manager error. \n");
		return NULL;
	}
	//这里实现三个元素的入队 三个线程应该塞入了9个
	const char * str = "hello world";
	const char * str1 = "hello myworld";
	const char * str2 = "my test of queue";
	printf("thread enter queue [0]: %d,queue_size :%d \n", en_queue(manager, str, strlen(str)), get_queue_size(manager));
	sleep(1);
	printf("thread enter queue [0]: %d,queue_size :%d \n", en_queue(manager, str1, strlen(str1)), get_queue_size(manager));
	sleep(1);
	printf("thread enter queue [0]: %d,queue_size :%d \n", en_queue(manager, str2, strlen(str2)), get_queue_size(manager));
	printf("enter_call_back end \n");
}
//专门实现出队列
void* out_call_back(void * arg)
{
	QUEUE_MANAGER * manager = (QUEUE_MANAGER *)arg;
	if(manager == NULL)
	{
		printf("thread get manager error. \n");
		return NULL;
	}
	while(1)
	{
		//其实不用这个逻辑更符合 最好的方案是把锁加在外部使用的地方
		// if(is_queue_empty(manager) == 0)
		// {
		// 	sleep(1);
		// 	printf("thread queue is null. \n");
		// }
		if(manager == NULL)
		{
			printf("thread manager is null \n");
		}
		char * out = de_queue(manager);
		if(out != NULL)
		{
			printf("thread output queue [%lu][%s]\n", strlen(out), out);
			printf("thread queue size: %d \n", get_queue_size(manager));
			free(out);
			out = NULL;
		}else
		{
			// printf("thread out is null, queue size: %d \n", get_queue_size(manager));
		}
	}
	printf("thread queue size: %d \n", get_queue_size(manager));
	printf("out_call_back end \n");
	return NULL;
}