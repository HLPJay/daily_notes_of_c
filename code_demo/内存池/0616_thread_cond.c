//对条件变量锁进行测试 超时等待
/*********************************************
多个线程通信，要实现一种唤醒或者超时唤醒的处理，
pthread_cond_timedwait 进行测试

实现方案：
	申请两个线程，同样的逻辑进行测试
********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/time.h>

pthread_mutex_t g_mutex;
pthread_cond_t g_cond;

//两个线程同样的回调
void* thread_callback(void* arg);
int main()
{
	//初始化全局锁和条件变量
	pthread_mutex_init(&g_mutex, NULL);
	pthread_cond_init(&g_cond, NULL);

	pthread_t t[2];
	//创建两个线程执行回调
	for(int i=0; i<2; i++)
	{
		int ret = pthread_create(&t[i], NULL, thread_callback, (void*)1);
		if(ret != 0)
		{
			printf("pthread_create failed. %d \n", ret);
			return -1;
		}
	}
	sleep(2);
	//主线程实现对线程回调函数中的阻塞等待进行唤醒
	pthread_mutex_lock(&g_mutex);
	pthread_cond_signal(&g_cond); //唤醒一个
	pthread_mutex_unlock(&g_mutex);

	//相关等待及销毁处理
	for(int i = 0; i<2; i++)
	{
		pthread_join(t[i], NULL);
	}

	pthread_cond_destroy(&g_cond);
	pthread_mutex_destroy(&g_mutex);
	return 0;
}
void* thread_callback(void* arg)
{
	//这里两个线程都用pthread_cond_timedwait 进行等待唤醒
	//两种不同的获取时间的方式
	struct timeval now;
	struct timespec outtime;
	gettimeofday(&now, NULL);
	outtime.tv_sec = now.tv_sec + 5; //s
    outtime.tv_nsec = now.tv_usec * 1000; //微妙转为纳秒
    int ret = -1;
    //锁+条件变量实现线程的阻塞一定时间， 或者被唤醒
    pthread_mutex_lock(&g_mutex);
    //等待一定的时间，进行唤醒
	ret = pthread_cond_timedwait(&g_cond, &g_mutex, &outtime);
	pthread_mutex_unlock(&g_mutex);
	printf("pthread_cond_timedwait return %d \n", ret);
	return NULL;
}