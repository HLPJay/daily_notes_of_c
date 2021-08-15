//这里有一个关于链表节点管理的技巧。
//可以替换线程池对 线程队列和任务队列的控制
#include <stdio.h>
#include <stdlib.h>

//把item元素插入链表中 list一直指向首节点，在首节点进行插入
//同理可以使用哨兵节点，实现快速插入
#define LIST_ADD(item, list) do{ 	\
	item->prev = NULL;				\
	item->next = list;				\
	list = item;					\
}while(0)

//移除元素的逻辑
//1：元素在list中间  直接移除
//2：元素是首节点和尾节点 
//3：元素就是list节点  重置list  ==》在上面的基础上也可能
#define LIST_REMOVE(item, list) do{		\
	if(item->prev != NULL)				\
		item->prev->next = item->next;	\
	if(item->next != NULL)				\
		item->next->prev = item->prev;	\
	if(list == item)					\
		list = item->next;				\
	item->prev = item->next = NULL;		\
}while(0)

typedef struct _t_list
{
	int data;
	struct _t_list *prev;
	struct _t_list *next;
}LIST;

//加入两个元素  移除两个元素做测试
int main()
{
	//定义list
	LIST *list = NULL; //认定成头节点
	LIST *node = (LIST*)malloc(sizeof(LIST));
	if(node == NULL){
		return -1;
	}
	node->data = 1;
	node->prev = node->next = NULL;
	//每次插入到头节点前
	LIST_ADD(node, list);

	LIST *node1 = (LIST*)malloc(sizeof(LIST));
	if(node1 == NULL){
		return -1;
	}
	node1->data = 2;
	node1->prev = node1->next = NULL;
	//每次插入到头节点前
	LIST_ADD(node1, list);

	//取节点，如果list节点不为NULL
	while(list!= NULL)
	{
		LIST* node_out = list;
		LIST_REMOVE(node_out, list);
		if(node_out == NULL)
		{
			printf("error node_list is null \n");
			return -1;
		}else
		{
			printf("get list data: [%d] \n", node_out->data);
			free(node_out);
		}
	}
	printf("")
	return 0;
}

/*************************************
ubuntu@ubuntu:~/c10k$ ./test
get list data: [2] 
get list data: [1]
*************************************/