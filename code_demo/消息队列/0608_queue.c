//实现一个队列的接口   多线程安全的队列
/*************************************
所谓的线程，进程间通信：
	管道，信号量，信号，消息队列，共享内存，原生socket，全局变量加锁

两个点：
	1: 保证同一时间的某个变量的单一访问
	2：定义可以共同访问的变量。

1：使用c++实现是比较好的方案
2：使用c实现一般是用#define 宏进行操作  可以参考nginx
*************************************/

//实现线程安全的消息队列
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	// pthread_mutex_t lock;
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
	QUEUE_MANAGER* queue_manger = (QUEUE_MANAGER*)malloc(sizeof(QUEUE_MANAGER));
	if(queue_manger == NULL)
	{
		printf("create queue manager error. \n");
		return NULL;
	}
	memset(queue_manger, 0, sizeof(QUEUE_MANAGER));
	queue_manger->queue = queue;
	// pthread_mutex_init(&queue_manger->lock , NULL);
	queue_manger->count=0;
	return queue_manger;
};

int Destory_queue(QUEUE_MANAGER *queue_manger)
{
	if(queue_manger == NULL || queue_manger->queue == NULL)
	{
		printf("Destory_queue error. \n");
		return -1;
	}
	QUEUE *queue = queue_manger->queue;
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
	// pthread_mutex_destroy(&queue_manger->lock);
	free(queue);
	free(queue_manger);
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

	queue_manager->queue->tail->next = node;
	queue_manager->queue->tail = node;
	queue_manager->count++;
	return 0;
};

//出队 返回队列中的第一个
char * de_queue(QUEUE_MANAGER *queue_manager)
{
	if(queue_manager ==NULL ||queue_manager->queue->head == NULL ||queue_manager->queue->head->next == NULL)
	{
		printf("queue is null. \n");
		return NULL;
	}
	QUEUE_NODE * node = queue_manager->queue->head->next;
	queue_manager->queue->head->next = node->next;

	int len = strlen(node->data);
	printf(" dequeue data:[%d][%s] \n", len, node->data);
	char* data = (char*)malloc(len);
	memcpy(data, node->data, len);
	free(node);

	queue_manager->count--;
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
	//入队  出队 判空  取大小
	const char * str = "hello world";
	const char * str1 = "hello myworld";
	const char * str2 = "my test of queue";
	printf("en_queue [0]: %d \n", en_queue(manager, str, strlen(str)));
	printf("en_queue [0]: %d \n", en_queue(manager, str1, strlen(str1)));
	printf("en_queue [0]: %d \n", en_queue(manager, str2, strlen(str2)));

	printf("en queue [1]: %d \n", is_queue_empty(manager));
	printf("en queue size[3] :%d \n",get_queue_size(manager));
	char * out = de_queue(manager);
	if(out == NULL)
	{
		printf("de_queue1 error is null \n");
	}else
	{
		printf("de_queue 1 [%lu][%s]\n", strlen(out), out);
		free(out);
		out = NULL;
		printf("de_queue size[2] :%d \n",get_queue_size(manager));
	}
	
	out = de_queue(manager);
	if(out == NULL)
	{
		printf("de_queue2 error is null \n");
	}else
	{
		printf("de_queue 2 [%lu][%s]\n", strlen(out), out);
		free(out);
		out = NULL;
		printf("de_queue size[1] :%d \n",get_queue_size(manager));
	}

	out = de_queue(manager);
	if(out == NULL)
	{
		printf("de_queue3 error is null \n");
	}else
	{
		printf("de_queue 3 [%lu][%s]\n", strlen(out), out);
		free(out);
		out = NULL;
		printf("de_queue size[0] :%d \n",get_queue_size(manager));
	}

	out = de_queue(manager);
	if(out!= NULL)
	{
		printf("null queue de_queue error \n");
	}else
	{
		printf("null queue de_queue success \n");
	}

	if(Destory_queue(manager) <0)
	{
		printf("Destory_queue error \n");
		return -1;
	}
	return 0;
}