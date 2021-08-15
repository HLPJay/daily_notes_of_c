/*************************************
实现内存池的逻辑：
	1：定义大块内存的申请结构
	2：定义小块内存的申请结构
	3：定义管理节点

相关函数的实现：
	1：大块内存直接申请，保存指针
	2：小块内存使用结构进行申请
	3：大块内存和小块内存的释放逻辑
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <fcntl.h>
#define MP_ALIGNMENT       		32
#define MP_PAGE_SIZE			4096
#define MP_MAX_ALLOC_FROM_POOL	(MP_PAGE_SIZE-1)

//为了字节对齐
#define mp_align(n, alignment) (((n)+(alignment-1)) & ~(alignment-1))
#define mp_align_ptr(p, alignment) (void *)((((size_t)p)+(alignment-1)) & ~(alignment-1))

//存储大块内存的指针
typedef struct mp_large_t{
	struct mp_large_t *next;
	void * alloc;
}MP_LARGE_T;

//存储小节点的结构
typedef struct mp_node_t
{
	unsigned char * last;
	unsigned char * end;
	struct mp_node_t *next;
	size_t failed;
}MP_NODE_T;

//定义管理节点的结构 柔性数组，实际的内存是head[0]
typedef struct mp_pool_t
{
	size_t max;

	struct mp_node_t * current;
	struct mp_large_t * large;

	struct mp_node_t head[0];
}MP_POOL_T;



//重新申请内存块，申请大内存
static void* mp_alloc_block(MP_POOL_T * pool, size_t size);
static void* mp_alloc_large(MP_POOL_T * pool, size_t size);

//按照字节对齐申请大块内存
void* mp_memalign(MP_POOL_T * pool, size_t size, size_t alignment);

//总接口，申请大内存或者小内存，默认字节对齐方式
void* mp_alloc(MP_POOL_T * pool, size_t size);

//申请内存，但是不做字节对齐处理
void * mp_nalloc(MP_POOL_T * pool, size_t size);
//申请内存并重置为0
void * mp_calloc(MP_POOL_T * pool, size_t size);
int mp_free(MP_POOL_T *pool, void* p);

MP_POOL_T * mp_create_pool(size_t size);
void mp_destory_pool(MP_POOL_T* pool);
void mp_reset_pool(MP_POOL_T * pool);

int main()
{
	int size = 1 << 12; //左移12位，申请内存块的大小，一般是2的倍数 这里是2的12次方
	printf("pool size is : %d \n", size);
	MP_POOL_T *pool = mp_create_pool(size);
	printf("pool->nax = %ld \n", pool->max);
	//申请小块内存测试
	for(int i=0; i<10; i++)
	{
		void * mp = mp_alloc(pool, 512);
		// mp_free(pool, mp); //这是小块内存  不用释放
	}
	void * large_t = mp_alloc(pool, 8192);
	printf("large test %p \n", large_t);
	printf("pool large test %p \n", pool->large->alloc);

	printf("mp_align(123, 32): %d, mp_align(17, 32): %d\n", mp_align(24, 32), mp_align(17, 32));
	// printf("mp_align_ptr(p->current, 32): %lx, p->current: %lx, mp_align(p->large, 32): %lx, p->large: %lx\n", mp_align_ptr(p->current, 32), p->current, mp_align_ptr(p->large, 32), p->large);

	int i=0;
	int j = 0;
	for (i = 0;i < 5;i ++) {

		char *pp = mp_calloc(pool, 32);
		for (j = 0;j < 32;j ++) {
			if (pp[j]) {
				printf("calloc wrong\n");
			}
			// printf("calloc success\n");
		}
	}

	mp_reset_pool(pool);

	for (i = 0;i < 58;i ++) {
		mp_alloc(pool, 256);
	}

	mp_destory_pool(pool);
	return 0;
}




//申请线程池 保存管理节点+实际的内存
//管理节点和哨兵节点
MP_POOL_T * mp_create_pool(size_t size)
{
	MP_POOL_T * pool;
	//申请大内存 目标申请大小+管理节点结构体
	//并指定节点对齐方式
	int ret = posix_memalign((void**)&pool, MP_ALIGNMENT, size+sizeof(MP_POOL_T)+sizeof(MP_NODE_T));
	if(ret)
	{
		printf("malloc mm pool error. \n");
		return NULL;
	}
	//区分小块内存还是大块内存的标志
	pool->max = (size < MP_MAX_ALLOC_FROM_POOL) ? size : MP_MAX_ALLOC_FROM_POOL;
	pool->current = pool->head;
	pool->large = NULL;

	pool->head->last = (unsigned char*)pool +sizeof(MP_POOL_T) +sizeof(MP_NODE_T);
	pool->head->end = pool->head->last+size;
	pool->head->next = NULL;
	pool->head->failed = 0;
	return pool;
}

//线程池的销毁处理
void mp_destory_pool(MP_POOL_T* pool)
{
	if(pool == NULL) return;
	//先销毁大内存 保存的大块内存的起始链表节点
	MP_LARGE_T * large;
	large = pool->large;
	while(large)
	{
		if(large->alloc!=NULL)
		{
			free(large->alloc);
		}
		large = large->next;
	}
	//销毁在线程池内维护的小内存节点 可能重新分配小内存的池子吧
	MP_NODE_T * node, *next_node;
	node = pool->head->next;
	while(node)
	{
		next_node = node->next;
		free(node);
		node = next_node;
	}
	free(pool);
}

//销毁大内存的申请 小块内存的重新指向
void mp_reset_pool(MP_POOL_T * pool)
{
	MP_LARGE_T *large;
	for(large = pool->large; large; large=large->next)
	{
		if(large->alloc)
		{
			free(large->alloc);
		}
	}
	pool->large = NULL;

	MP_NODE_T* node;
	for(node = pool->head; node; node=node->next)
	{
		node->last = (unsigned char*)node+sizeof(MP_NODE_T);
	}
}
//按块分配内存  内存池不够用了，再分配内存
//返回当前内存
static void* mp_alloc_block(MP_POOL_T * pool, size_t size)
{
	//计算第一块内存池的大小 不需要管理结构体
	unsigned char* temp;
	MP_NODE_T * node = pool->head;
	size_t psize = (size_t)(node->end-(unsigned char*)node);

	int ret = posix_memalign((void**)&temp, MP_ALIGNMENT, psize);
	if(ret) return NULL;

	//对申请到的内存连接
	MP_NODE_T* new_node;
	new_node = (MP_NODE_T*)temp;
	new_node->end = temp+psize;
	new_node->next = NULL;
	new_node->failed = 0;

	//做一下字节对齐处理
	temp=temp+sizeof(MP_NODE_T);
	temp = mp_align_ptr(temp, MP_ALIGNMENT);
	new_node->last = temp+size; //指向已经用过后的最新空闲节点

	//遍历当前的池子   池子申请内存大于4次就看下一个池子
	MP_NODE_T* current, *p;
	current = pool->current;
	//遍历所有的池子 有池子可用 取出来
	//failed 统一加1，并取失败小于4次的
	for(p=current; p->next; p=p->next)
	{
		if(p->failed++>4)
		{
			current = p->next;
		}
	}
	//新内存加入到池子队列中
	p->next = new_node;
	//重新定义当前的池子
	pool->current = current ? current:new_node;
	return temp;
}

//申请大块内存 然后用头插法插入管理节点  或者查找已经释放的大内存直接使用
static void* mp_alloc_large(MP_POOL_T * pool, size_t size)
{
	void * temp = malloc(size);
	if( temp == NULL) return NULL;

	size_t n = 0;
	MP_LARGE_T * large;
	for(large = pool->large; large; large=large->next)
	{
		if(large->alloc == NULL)
		{
			large->alloc = temp;
			return temp;
		}
		if(n++>3) break;
	}
	//在小块的内存池中 申请结构体
	large = mp_alloc(pool, sizeof(MP_LARGE_T));
	if(large == NULL)
	{
		free(temp);
		return NULL;
	}
	//头插法
	large->alloc = temp;
	large->next = pool->large;
	pool->large = large;
	return temp;
}
//按照字节对齐分配大块内存
void* mp_memalign(MP_POOL_T * pool, size_t size, size_t alignment)
{
	void * temp;
	int ret = posix_memalign(&temp, alignment, size);
	if(ret) return NULL;

	//在内存池中取内存存结构
	MP_LARGE_T * large = mp_alloc(pool, sizeof(MP_LARGE_T));
	if(large == NULL)
	{
		free(temp); 
		return NULL;
	}
	large->alloc = temp;
	large->next = pool->large;
	pool->large = large;
	return temp;
}

//在内存池中申请内存 
void* mp_alloc(MP_POOL_T * pool, size_t size)
{
	unsigned char* temp;
	//从当前节点开始  计算可用池子，申请内存
	MP_NODE_T * p;
	if(size <= pool->max)
	{
		p=pool->current;
		do{
			temp= mp_align_ptr(p->last, MP_ALIGNMENT);
			if((size_t)(p->end - temp) >= size)
			{
				p->last = temp +size;
				return temp;
			}
			p=p->next;
		}while(p);
		//内存不够的情况下  需要重新申请内存块
		return mp_alloc_block(pool, size);
	}
	//申请大块内存
	return mp_alloc_large(pool, size);
}
//这个函数用于不用字节对齐的处理
void * mp_nalloc(MP_POOL_T * pool, size_t size)
{
	unsigned char* temp;
	MP_NODE_T * p;
	if(size <= pool->max)
	{
		p=pool->current;
		do{
			temp=p->last;
			if((size_t)(p->end - temp) >= size)
			{
				p->last = temp +size;
				return temp;
			}
			p = p->next;
		}while(p);
		return mp_alloc_block(pool, size);
	}
	return mp_alloc_large(pool, size);
}
//申请内存并重置为0
void * mp_calloc(MP_POOL_T * pool, size_t size)
{
	void * p = mp_alloc(pool, size);
	if(p)
	{
		memset(p, 0, size);
	}
	return p;
}
//释放一块大内存
int mp_free(MP_POOL_T *pool, void* p)
{
	MP_LARGE_T * large;
	for(large = pool->large; large; large=large->next)
	{
		if(p == large->alloc)
		{
			free(large->alloc);
			large->alloc = NULL;
			return 0;
		}
	}
	return -1;
}


