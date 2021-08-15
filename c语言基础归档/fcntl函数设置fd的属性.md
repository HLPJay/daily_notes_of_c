# fcntl可以实现记录锁的功能

## fcntl 改变已打开文件的属性

```c
	int fcntl(int fd, int cmd);
	int fcntl(int fd, int cmd, long arg);
	int fcntl(int fd, int cmd, struct flock * lock);
```

## fcntl可以实现的功能：

	可以复制已有描述符
	获取或设置文件描述符
	获取或设置文件状态标志
	获取或设置异步I/O所有权
	获取或设置记录锁  ==》重要
	
		实例：使用fcntl函数可以实现记录锁的功能
			 可以使用fcntl打印相关的文件描述标志

## fcntl设置记录锁时，涉及的数据结构如下

```
struct flcok
{
    short int l_type; //锁定的状态
    short int l_whence; //决定l_start 位置
    off_t l_start; //锁定区域的开头位置
    off_t l_len; //锁定区域的大小
    pid_t l_pid; //锁定动作的进程
};
```

## fcntl的demo：

```c
/*******************************************************
在了解fcntl函数过程中，阅读unix环境高级编程章节：
	lseek，显示为一个打开的文件设置偏移量，允许超过文件长度，会补0
		od命令可以查看文件 -c 二进制查看

磁盘写之前有缓冲区，保证磁盘与缓冲区的一致性：
	fsync   fdatasync   sync

fcntl 改变已打开文件的属性
	int fcntl(int fd, int cmd);
	int fcntl(int fd, int cmd, long arg);
	int fcntl(int fd, int cmd, struct flock * lock);
	
	可以复制已有描述符
	获取或设置文件描述符
	获取或设置文件状态标志
	获取或设置异步I/O所有权
	获取或设置记录锁  ==》重要

	实例：使用fcntl函数可以实现记录锁的功能
		 可以使用fcntl打印相关的文件描述标志
*********************************************************/

// fseek 可以定位超过文件长度，中间的空洞不需要占内存，被读为\0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)  
#define FILE_PATH   "./file.hole"

	int my_fseek();

	void lock_init(struct flock *lock, short type, short whence, off_t start, off_t len);
	int readw_lock(int fd);
	int writew_lock(int fd);
	int unlock(int fd);
	pid_t lock_test(int fd, short type, short whence, off_t start, off_t len);


	int fcntllock_test();
	int readlock_test();
	int writelock_test();
	int writelock_test1();
//参考https://blog.csdn.net/anonymalias/article/details/9197641
int main(int agrc, char ** argv)
{
	printf("fseek tets: \n");
	my_fseek();

	printf("记录锁的测试： \n");
	fcntllock_test();
	readlock_test();
	writelock_test();
	writelock_test1();
	return 0;
}



// 打开一个文件 用fseek偏移  观察文件内容
/*
hlp@ubuntu:~/delay_test$ od -c file.hole 
0000000   a   b   c   d   e   f   g   h   i   j  \0  \0  \0  \0  \0  \0
0000020  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
*
0040000   A   B   C   D   E   F   G   H   I   J
0040012
*/
//使用 ls -ls 可以查看文件占用的磁盘块数
int my_fseek()
{
	char buf1[] = "abcdefghij";
	char buf2[] = "ABCDEFGHIJ";
	int fd;
	fd = creat("file.hole", FILE_MODE);
		 //相当与调用固定模式的open
		//以只写的方式打开创建的文件
	if(fd < 0)
	{
		printf("create file error \n");
	}

	if(write(fd, buf1, 10) != 10)
	{
		printf("buf1 write error \n");
	}

	if(lseek(fd, 16384, SEEK_SET) == -1)
	{
		printf("lseek error \n");
	}

	if(write(fd, buf2, 10) != 10)
	{
		printf("buf2 write error \n");
	}

	if(fd >0)
	{
		close(fd);
	}
	return 0;
}

//调用的函数，在文章末尾贴出
int fcntllock_test()
{
    int fd = open(FILE_PATH, O_RDWR | O_CREAT, FILE_MODE);
    writew_lock(fd);
 
    if (fork() == 0)
    {
 		printf(" %d \n", lock_test(fd, F_WRLCK, SEEK_SET, 0, 0));
 		printf(" %d \n", lock_test(fd, F_RDLCK, SEEK_SET, 0, 0));
        exit(0);
    }
 
    sleep(3);
    unlock(fd);
 
    return 0;
}
//调用的函数，在文章末尾贴出
int readlock_test()
{
    int fd = open(FILE_PATH, O_RDWR | O_CREAT, FILE_MODE);
    readw_lock(fd);
 
    //child  1
    if (fork() == 0)
    {
        printf("child 1 try to get write lock... \n");
        writew_lock(fd);
        printf("child 1 get write lock.. \n");
 
        unlock(fd);
        printf("child 1 release write lock... \n");
 
        exit(0);
    }
 
    //child 2
    if (fork() == 0)
    {
        sleep(3);
 
        printf("child 2 try to get read lock... \n");
        readw_lock(fd);
        printf("child 2 get read lock... \n");
 
        unlock(fd);
        printf("child 2 release read lock... \n");
        exit(0);
    }
 
    sleep(10);
    unlock(fd);
 
    return 0;
}

//调用的函数，在文章末尾贴出
int writelock_test()
{ 
    int fd = open(FILE_PATH, O_RDWR | O_CREAT, FILE_MODE);
    writew_lock(fd);
 
    //child  1
    if (fork() == 0)
    {
        sleep(3);
 
        printf("child 1 try to get write lock... \n");
        writew_lock(fd);
        printf("child 1 get write lock... \n");
 
        unlock(fd);
        printf("child 1 release write lock... \n");
 
        exit(0);
    }
 
    //child 2
    if (fork() == 0)
    {
        printf(" child 2 try to get read lock...\n");
        readw_lock(fd);
        printf(" child 2 get read lock...\n");
 
        unlock(fd);
        printf(" child 2 release read lock... \n");
 
        exit(0);
    }
 
    sleep(10);
    unlock(fd);
 
    return 0;
}

//调用的函数，在文章末尾贴出
int writelock_test1()
{ 
    int fd = open(FILE_PATH, O_RDWR | O_CREAT, FILE_MODE);
    writew_lock(fd);
 
    //child  1
    if (fork() == 0)
    {
        printf(" child 1 try to get write lock...\n");
        writew_lock(fd);
        printf(" child 1 get write lock...\n");
 
        unlock(fd);
        printf(" child 1 release write lock...\n");
 
        exit(0);
    }
 
    //child 2
    if (fork() == 0)
    {
        sleep(3);
 
        printf("child 2 try to get read lock... \n");
        readw_lock(fd);
        printf(" child 2 get read lock...\n");
 
        unlock(fd);
        printf(" child 2 release read lock...\n");
 
        exit(0);
    }
 
    sleep(10);
    unlock(fd);
 
    return 0;
}


void lock_init(struct flock *lock, short type, short whence, off_t start, off_t len)
{
    if (lock == NULL)
        return;
 
    lock->l_type = type;
    lock->l_whence = whence;
    lock->l_start = start;
    lock->l_len = len;
}
 
int readw_lock(int fd)
{
    if (fd < 0)
    {
        return -1;
    }
 
    struct flock lock;
    lock_init(&lock, F_RDLCK, SEEK_SET, 0, 0);
 
    if (fcntl(fd, F_SETLKW, &lock) != 0)
    {
        return -1;
    }
    
    return 0;
}
 
int writew_lock(int fd)
{
    if (fd < 0)
    {
        return -1;
    }
 
    struct flock lock;
    lock_init(&lock, F_WRLCK, SEEK_SET, 0, 0);
 
    if (fcntl(fd, F_SETLKW, &lock) != 0)
    {
        return -1;
    }
 
    return 0;
}
 
int unlock(int fd)
{
    if (fd < 0)
    {
        return -1;
    }
 
    struct flock lock;
    lock_init(&lock, F_UNLCK, SEEK_SET, 0, 0);
 
    if (fcntl(fd, F_SETLKW, &lock) != 0)
    {
        return -1;
    }
 
    return 0;
}
 
pid_t lock_test(int fd, short type, short whence, off_t start, off_t len)
{
    struct flock lock;
    lock_init(&lock, type, whence, start, len);
 
    if (fcntl(fd, F_GETLK, &lock) == -1)
    {
        return -1;
    }
 
    if(lock.l_type == F_UNLCK)
        return 0;
    return lock.l_pid;
}
```

