早在很久之前第一份工作，就接触使用过协程，对它只有只可理解不可言说的概念，这里通过对ntyco协程代码的理解，梳理一下自己对协程的理解。

# 1：协程的概念

总是把协程俗称为用户态线程，我的理解是，协程依托于线程上执行子例程，由用户态把控协程的调度及上下文。

# 2：协程的性能

协程的性能差不多和线程池的性能相同，差不到哪里去（多线程有线程调度切换开销，协程有管理节点的内存消耗）

总结：协程同步的变成方式，可以达到异步的性能。

# 3：什么时候使用协程

百科上说：适合于用来实现彼此熟悉的程序组件，如合作式多任务，迭代器，无限列表和管道。

我想了想，这个使用场景其实还是没有明确的理解：能使用协程解决的问题，一般都可用多线程线程池的方式去替代。

协程可以处理一些上下文相关的io任务，不用关心同步资源共享的问题，处理IO密集型任务，对计算机cpu消耗不高的任务。

协程可以配合网络IO一起使用，IO的底层控制其实是epoll，所以性能上的提升应该也有限制。

# 4：ntyco中协程的实现

## 1：协程实现关注的技术点：

1：协程的结构如何定义。

2：协程的内存如何存储，初始化寄存器。

3：协程的如何调度切换。

4：如果管理协程状态调度节点，协程的使用逻辑（模拟文件描述符用epoll管理）。

## 2：具体分析协程的实现

### 1：概述：从业务流程思考

1：创建协程

2：任务的执行，调用回调函数和函数参数

3：多个协程，多个回调函数之间的切换。 ==》保存上一个协程的信息，和当前执行的协程，实现切换

4：协程的销毁，资源释放。

### 2：协程的结构

#### 1：协程的节点：

​		寄存器中保存节点正在执行**函数的上下文，执行的回调函数，函数参数**

​		依托于线程，要保存线程**栈空间的大小，栈地址**

​		其他：协程的创建时间戳，协程的id，函数名，网络io处理相关等。

```c
typedef void(* proc_coroutine)(void*);
typedef struct _nty_coroutine{
	nty_cpu_ctx ctx;        //寄存器中的信息
	proc_coroutine * func;  //真正执行的回调函数
	void * arg;				//回调函数对应的参数

	void *stack;			//线程栈的管理
	size_t stack_size;      
	size_t last_stack_size;

	uint64_t birth;     //协程的创建时间，一些标识
	uint64_t id;
	char funcname[64];
	
	uint32_t ops;				 //正在运行标志
	nty_coroutine_status status; //协程当前的状态

	struct _nty_schedule *sched;   	//协程的管理节点
}nty_coroutine;

```

##### 1.1 寄存器中保存上下文关系的结构定义： (由汇编代码实现寄存器的切换，实现协程的切换)

```c
//寄存器中保存的cpu中的设置：
typedef struct _nty_cpu_ctx {
	void *esp; //
	void *ebp;
	void *eip;
	void *edi;
	void *esi;
	void *ebx;
	void *r1;
	void *r2;
	void *r3;
	void *r4;
	void *r5;
} nty_cpu_ctx;
```

#### 2：协程的管理节点：

​	协程的状态： **就绪，等待，睡眠，正在执行**，

​	用两个**红黑树**管理睡眠和等待的协程，用**队列**管理准备就绪的协程，用**链表**管理正在执行的协程（其实一个元素就够了）

​	其他管理：**创建时间，个数，协程切换时间，内存地址，页大小（管理内存）**

​	保存正在执行的**寄存器上下文**，执行中。 ==》其实也就是**当前协程**

```c
//协程的管理节点
typedef struct _nty_schedule{
	nty_cpu_ctx ctx;					//最近一次resume的协程的内容
	nty_coroutine *curr_thread;			//当前执行的协程     ==》配合控制协程的切换

	void *	stack;
	size_t 	stack_size;					 //每个协程分配的内存的大小
	int		page_size;                   //内存分页的大小   与栈内存做对比处理

	uint64_t 	birth;
	int 		spawned_coroutines;
	uint64_t 	default_timeout;

	//epoll处理 用eventfd 文件描述符的形式管理了红黑树节点
	int poller_fd;
	int eventfd;              //好像没有用到该节点   不用保存加入epoll中的fd
	struct epoll_event eventlist[NTY_CO_MAX_EVENTS];	//触发事件
	int num_new_events;        							//触发事件个数

	//用到的相关结构红黑树：
	nty_coroutine_rbtree_sleep sleeping;
	nty_coroutine_rbtree_wait waiting;

	//用到的相关队列
	nty_coroutine_queue ready;
	//管理正在执行的链表
	nty_coroutine_link busy;
}nty_schedule;
```

### 3：协程的创建及初始化

#### 1：初始化管理节点 nty_schedule，一组协程节点管理只需要一个

在协程节点中实际触发。

pthread_key_create，**pthread_setspecific和pthread_getpecific**实现线程中不同的函数共享数据的方式，并设置资源的释放 ==》这**里可以用函数的返回值返回结构形式代替，自己管理释放。**

![image-20210706123336509](C:\Users\XA-146\AppData\Roaming\Typora\typora-user-images\image-20210706123336509.png)

#### 2：协程节点的初始化。  ==》为协程分配内存，处理寄存器节点执向该内存。

创建一个协程节点，申请内存，放入就绪队列中等待触发。

使寄存器节点中信息指向该内存。 ==》这里的内存与寄存器还没有关联，需要做一定的处理

```make
ESP 专门用作堆栈指针，被形象地称为栈顶指针，堆栈的顶部是地址小的区域，压入堆栈的数据越多，ESP也就越来越小。在32位平台上，ESP每次减少4字节。

esp：寄存器存放当前线程的栈顶指针
ebp：寄存器存放当前线程的栈底指针
eip：寄存器存放下一个CPU指令存放的内存地址，当CPU执行完当前的指令后，从EIP寄存器中读取下一条指令的内存地址，然后继续执行。
```

#### 3：触发执行

##### 1：主要的控制调度的两个函数，内部都是同样的线程切换逻辑。

```c
void nty_coroutine_yield(nty_coroutine *co); //该协程睡眠     修改运行标志，切换下一个运行
	//调用switch函数进行切换
int nty_coroutine_resume(nty_coroutine *co); //恢复协程的执行  根据标志，进行初始化寄存器位置，切换执行，资源释放。  根据页管理内存
	//resume对没有执行过的协程的处理 初始化寄存器位置
    //resume对已经执行过的协程的处理 直接调用switch函数进行切换
```

##### 2：核心模块：汇编实现的协程切换（切换寄存器中的信息）

操作的结构：实际上就是每个协程节点中保存的寄存器信息。

与当前寄存器中的信息进行交换处理：

```c
x86_64 的寄存器有16个64位寄存器，分别是：%rax, %rbx, %rcx, %esi, %edi, %rbp, %rsp, %r8, %r9, %r10, %r11, %r12, %r13, %r14, %r15。

%rax 作为函数返回值使用的。
%rsp 栈指针寄存器，指向栈顶
%rdi, %rsi, %rdx, %rcx, %r8, %r9 用作函数参数，依次对应第1参数，第2参数。。。
%rbx, %rbp, %r12, %r13, %r14, %r15 用作数据存储，遵循调用者使用规则，换句话说，就是随便用。调用子函数之前要备份它，以防它被修改
%r10, %r11 用作数据存储，就是使用前要先保存原值。

//实现两部分功能  先保存寄存器中的值，再恢复到寄存器中  
//第一个参数要运行的协程， 第二个为正在运行的协程
int _switch(nty_cpu_ctx *new_ctx, nty_cpu_ctx *cur_ctx);

__asm__ (
"    .text                                  \n"
"       .p2align 4,,15                                   \n"
".globl _switch                                          \n"
".globl __switch                                         \n"
"_switch:                                                \n"
"__switch:                                               \n"
"       movq %rsp, 0(%rsi)      # save stack_pointer     \n"
"       movq %rbp, 8(%rsi)      # save frame_pointer     \n"
"       movq (%rsp), %rax       # save insn_pointer      \n"
"       movq %rax, 16(%rsi)                              \n"
"       movq %rbx, 24(%rsi)     # save rbx,r12-r15       \n"
"       movq %r12, 32(%rsi)                              \n"
"       movq %r13, 40(%rsi)                              \n"
"       movq %r14, 48(%rsi)                              \n"
"       movq %r15, 56(%rsi)                              \n"
"       movq 56(%rdi), %r15                              \n"
"       movq 48(%rdi), %r14                              \n"
"       movq 40(%rdi), %r13     # restore rbx,r12-r15    \n"
"       movq 32(%rdi), %r12                              \n"
"       movq 24(%rdi), %rbx                              \n"
"       movq 8(%rdi), %rbp      # restore frame_pointer  \n"
"       movq 0(%rdi), %rsp      # restore stack_pointer  \n"
"       movq 16(%rdi), %rax     # restore insn_pointer   \n"
"       movq %rax, (%rsp)                                \n"
"       ret                                              \n"
);

按照x86_64的寄存器定义，
    %rdi保存第一个参数的值，即new_ctx的值，
    %rsi保存第二个参数的值，即保存cur_ctx的值。
    X86_64每个寄存器是64bit，8byte。

Movq %rsp, 0(%rsi) 保存在栈指针到cur_ctx实例的rsp项
Movq %rbp, 8(%rsi)
Movq (%rsp), %rax #将栈顶地址里面的值存储到rax寄存器中。Ret后出栈，执行栈顶
Movq %rbp, 8(%rsi) #后续的指令都是用来保存CPU的寄存器到new_ctx的每一项中
Movq 8(%rdi), %rbp #将new_ctx的值
Movq 16(%rdi), %rax #将指令指针rip的值存储到rax中
Movq %rax, (%rsp) # 将存储的rip值的rax寄存器赋值给栈指针的地址的值。

Ret # 出栈，回到栈指针，执行rip指向的指令。
```

##### 3：nty_coroutine_yield 和nty_coroutine_resume函数细节

只有这两个函数调用协程的切换，控制了协程的调度。

nty_coroutine_yield ：保存寄存器中的值，让出cpu，执行到最近的resume

​			==》这里应该获取一个协程，然后进行切换的。

nty_coroutine_resume：第一次的初始化，协程中初始化寄存器信息，切换cpu，执行该协程。

​	整个调度控制由nty_coroutine_resume+协程数据结构控制。

#### 4：调度控制 nty_schedule_run

1：判断管理节点中各数据结构中是否有相关的任务。

2：处理睡眠到时的协程，进行触发执行，红黑树结构

3：处理准备就绪的协程，进行触发执行，队列结构。

4：处理有事件触发的协程，进行触发执行，红黑树结构。  ==》多于网络io配合

**nty_coroutine_resume** 函数触发的执行。

#### 5：资源的销毁：

1：协程节点的资源销毁：根据状态调度销毁函数。

2：管理节点的资源销毁：通过pthread_once函数进行设置，销毁堆内存。

### 4：协程与网络IO适配使用

ntyco中对网络通信相关的api做了相关的处理，实现了一套以协程为机制的套接字处理方案。 

主要函数如下：

​	其实主要业务逻辑就是把本次的fd进行加入epoll中，并管理自己的红黑树节点加入

   处理已经触发的一个业务。

   删除已经触发的业务的epoll监听和红黑树管理

```c
static int nty_poll_inner(struct pollfd *fds, nfds_t nfds, int timeout) {

	if (timeout == 0)
	{
		return poll(fds, nfds, timeout);   //这里没有用到poll的相关处理
	}
	if (timeout < 0)
	{
		timeout = INT_MAX;
	}

	nty_schedule *sched = nty_coroutine_get_sched();
	nty_coroutine *co = sched->curr_thread;
	
	int i = 0;
	for (i = 0;i < nfds;i ++) {
	
		struct epoll_event ev;
		ev.events = nty_pollevent_2epoll(fds[i].events);   //与poll兼容，把事件转为epoll
		ev.data.fd = fds[i].fd;
		epoll_ctl(sched->poller_fd, EPOLL_CTL_ADD, fds[i].fd, &ev);  //fd加入epoll中

		co->events = fds[i].events;
		nty_schedule_sched_wait(co, fds[i].fd, fds[i].events, timeout); //把fd与协程关联，并加入红黑树中
	}
    //特别注意，这个很关键，虽然参数没变，但是其实内部已经切换了下一个协程，最近resume的协程
	nty_coroutine_yield(co); 

	for (i = 0;i < nfds;i ++) {
	
		struct epoll_event ev;
		ev.events = nty_pollevent_2epoll(fds[i].events);
		ev.data.fd = fds[i].fd;
		epoll_ctl(sched->poller_fd, EPOLL_CTL_DEL, fds[i].fd, &ev);  //已经执行了这个协程事件，所以在epoll中移除该节点

		nty_schedule_desched_wait(fds[i].fd);  //从事件触发wait红黑树中删除该协程
	}

	return nfds;
}
```

## 5：扩展及相关demo

扩展：可以使用多线程，多进程（绑核）的方式实现多协程。

ntyco中的demo运行可以适配理解协程的实现逻辑，尤其是网络io相关的demo。