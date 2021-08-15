
/*******************************************
获取当前时间的相关整理：
	这里了解到有两个结构体：

*******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h> //time()
#include <string.h>

#include <sys/time.h> //gettimeofday
void get_time();
void get_time1();
int main()
{

	// time_t time(time_t * timer)  返回TC1970-1-1 0:0:0开始到现在的秒数
	// 与相关函数：localtime、gmtime、asctime、ctime，结合可以获得当前系统时间或是标准时间。
	//	struct tm* gmtime(const time_t *timep);    转换为没有经过时区转换的UTC时间，是一个struct tm结构指针
	//  stuct tm* localtime(const time_t *timep);  gmtime类似，但是它是经过时区转换的时间。
	//  char *asctime(const struct tm* timeptr);   转换为真实世界的时间，以字符串的形式显示
	//  char *ctime(const time_t *timep);		   转换为真是世界的时间，以字符串显示

	// 	int gettimeofday(struct timeval *tv, struct timezone *tz);
	//			返回当前距离1970年的秒数和微妙数，后面的tz是时区，一般不用
	//	time_t mktime(struct tm* timeptr);		   转换为从1970年至今的秒数

//所以，获取时间一般有两个函数time() 和gettimeofday()

	//time()函数的测试 两种使用方法
	time_t time0, time1;
	time0 = time(NULL);
	time(&time1); //获取当前时间，秒数
	printf("time() test is %ld = %ld \n", time0, time1);
	printf("ctime() :[%s] \n", ctime(&time1)); //直接字符串打印

	//先用gmtime,没有经过时区转换的时间 只有这里才不会有时区的转换  差8小时
	struct tm* g_time = gmtime(&time0);
	printf("asctime() :[%s] \n", asctime(g_time)); //转换为字符串进行打印
	
	//用localtime 转换为经过时区转换的时间
	struct tm* l_time = localtime(&time0);
	printf("asctime() :[%s] \n", asctime(l_time)); //转换为字符串进行打印

	printf("mktime(g_time) to s :[%ld] \n", mktime(g_time)); //这里默认带了时区的转换
	printf("mktime(l_time) to s :[%ld] \n", mktime(l_time));

	char tmpbuf[128];
    struct tm* newtime=localtime(&time1);
   	//测试相关的格式打印
    strftime( tmpbuf, 128, "Today is %A, day %d of %B in the year %Y.\n", newtime);
    printf("%s \n", tmpbuf);

    //其他两个结构体
    get_time();
    get_time1();
	return 0;
}

/******************
秒和纳秒
struct timespec {
	time_t tv_sec; // seconds
	long tv_nsec; // and nanoseconds
};
int clock_gettime(clockid_t, struct timespec *)获取特定时钟的时间：
	CLOCK_REALTIME 				统当前时间，从1970年1.1日算起
	CLOCK_MONOTONIC 			系统的启动时间，不能被设置
	CLOCK_PROCESS_CPUTIME_ID	本进程运行时间
	CLOCK_THREAD_CPUTIME_ID 	本线程运行时间
相关函数
	struct tm *localtime(const time_t *clock);  //线程不安全
	struct tm* localtime_r( const time_t* timer, struct tm* result );//线程安全
	size_t strftime (char* ptr, size_t maxsize, const char* format, const struct tm* timeptr );

秒和微妙
struct timeval {
	time_t tv_sec; // seconds
	long tv_usec; // microseconds
};
int gettimeofday(struct timeval *tv, struct timezone *tz)获取系统的时间
******************/
//1:clock_gettime 操作timespec结构，返回秒+纳秒结构的时间，可以定义时钟类型
void get_time()
{
	struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n",ts.tv_sec, ts.tv_nsec);
    
    struct tm t;
    char date_time[64];
    strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", localtime_r(&ts.tv_sec, &t));
    printf("clock_gettime :date_time=%s, tv_nsec=%ld\n", date_time, ts.tv_nsec);
}
// 2：gettimeofday 获取系统时间 这里微妙和纳秒可以对结构体操作进行转换
//  struct timeval now;
// 	struct timespec outtime;
// 	gettimeofday(&now, NULL);
// 	outtime.tv_sec = now.tv_sec; //s
//  outtime.tv_nsec = now.tv_usec * 1000; //微妙转为纳秒
void get_time1()
{
	struct timeval us;
    gettimeofday(&us,NULL);
    printf("gettimeofday: tv_sec=%ld, tv_usec=%ld\n", us.tv_sec, us.tv_usec);
    
    struct tm t;
    char date_time[64];
    strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", 		
    											localtime_r(&us.tv_sec, &t));
    printf("gettimeofday: date_time=%s, tv_usec=%ld\n", date_time, us.tv_usec);
}

/******************************************
	struct tm {
	    int tm_sec;      // Seconds (0-60) 
	    int tm_min;      // Minutes (0-59) 
	    int tm_hour;     // Hours (0-23) 
	    int tm_mday;     // Day of the month (1-31) 
	    int tm_mon;      // Month (0-11) 
	    int tm_year;     // Year - 1900；从1900年算起，至今的年份 
	    int tm_wday;     // Day of the week (0-6, Sunday = 0) 
	    int tm_yday;     // Day in the year (0-365, 1 Jan = 0) 
	    int tm_isdst;    // Daylight saving time；干啥用的？？？ 
	};

	%a: 英文单词中星期几的缩写版。如：星期三，表示为"Wed"。
​	%A: 英文单词中星期几的完整版。如：星期三，表示为"Wednesday"。
​	%b: 英文单词中月份的缩写版。如：11月，表示为"Nov"。
​	%B: 英文单词中月份的缩写版。如：11月，表示为"November"。
​	%c: 格式化的时间字符串。如：2018-11-28 10:13:40，表示为"Wed Nov 28 10:13:40 2018"。
​	%F: 日期格式为yyyy-mm-dd，与%Y:%m:%d作用相同。如：2018-11-28。
​	%X: 时间格式为hh:mm:ss。如：10:13:40。
​	%j: 一年中的第几天，范围：001-366.
​	%W: 一年中的第几周，范围：00-53.
​	%Y: 日期中的年，如：2018。
​	%m: 日期中的月，范围：00-12。
​	%d: 日期中的天，范围：01-31。
​	%H: 小时，范围：00-24。
​	%M: 分钟，范围：00-59。
​	%S: 秒，范围：00-60。
******************************************/