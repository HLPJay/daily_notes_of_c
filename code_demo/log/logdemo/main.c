/********************************************************************
c语言日志模块的可实现方案：
	1：#和##  属于预处理标记： #后面转换为字符串 ##预处理拼接标记
	2：__VA_ARGS__ C99中新增的特性，支持宏定义中支持可变参数，接收...传递多个参数。
		使用在使用了...的宏定义中：
			#define myprintf(...) fprintf(stderr, __VA_ARGS__)
	3：如何解析不定参数？ ==》应该是__VA_ARGS__ 底层实现，使用va_list
	4：#__VA_ARGS__  仅仅展开列表对应的字符串了。
	5：##__VA_ARGS__ 是GUN的特性，c标准不建议这样用。 解决__VA_ARGS__参数个数为0时，宏定义最后的，会编译报错问题。
		##在如果参数为0时，会把宏定义中前面的逗号删除掉。
	6：c语言中printf输出其实就是上面的逻辑思路：
			#include <stdio.h>
			int printf(const char *format, ...); //输出到标准输出
			int fprintf(FILE *stream, const char *format, ...); //输出到文件
			int sprintf(char *str, const char *format, ...); //输出到字符串str中
			int snprintf(char *str, size_t size, const char *format, ...);
			                              //按size大小输出到字符串str中

		下面系列把一个个变量用va_list调用所替代：
			int vprintf(const char *format, va_list ap);
			int vfprintf(FILE *stream, const char *format, va_list ap);    
			int vsprintf(char *str, const char *format, va_list ap);
			int vsnprintf(char *str, size_t size, const char *format, va_list ap);

	7：redis中分析：
		#include <stdio.h>
		#define D(...)                                                               \
		    do {                                                                     \
		        FILE *fp = fopen("/tmp/log.txt","a");                                \
		        fprintf(fp,"%s:%s:%d:\t", __FILE__, __func__, __LINE__);             \
		        fprintf(fp,__VA_ARGS__);                                             \
		        fprintf(fp,"\n");                                                    \
		        fclose(fp);                                                          \
		    } while (0);
		#define debugf(...)                                                            \
		    if (raxDebugMsg) {                                                         \
		        printf("%s:%s:%d:\t", __FILE__, __FUNCTION__, __LINE__);               \
		        printf(__VA_ARGS__);                                                   \
		        fflush(stdout);                                                        \
		    }
		//epos是一个全局变量，每一次在调用该宏时，获取文件读写指针当前位置 ftello
		static char error[1044];
		static off_t epos;
		#define ERROR(...) { \
			    char __buf[1024]; \
			    snprintf(__buf, sizeof(__buf), __VA_ARGS__); \
			    snprintf(error, sizeof(error), "0x%16llx: %s", (long long)epos, __buf); \
			}
********************************************************************/

//对描述提到的问题进行测试
#include <stdio.h>
#include <stdarg.h>
//1：#的测试，转换为字符  可以用这种方式转换打印字符串
#define P(A) 		printf("%s:%d\n",#A,A)
#define SQUARE(x) 	printf("The square of "#x" is %d.\n", ((x)*(x)))
//其实也是个字符串拼接
#define MY_TOSTR(var) (#var"[test1]\n")

void test_1()
{
	P(88);  //把int 8转为字符直接输出
	//使用场景
	int a=11, b=12;
	P(a+b);
	//求8的平方
	SQUARE(8);
	printf("%s", MY_TOSTR(8888));
}
//2: ##的测试，可以实现字符的拼接，使用场景：自动生成代码，可以用来拼接函数名
// static const char* FUNC = "_func";
//这玩意是直接拼接参数的东东
#define my_math(x, y) (x##e##y)
//linux下直接用空格可以实现拼接
#define XNAME(name, type) #name #type

//linux下的拼接直接用一个空格
#define SOFTWARE_VERSION "Software：V1.00"
#define HARDWARE_VERSION "Hardware：V1.00"
#define SYSTEM_VERSION SOFTWARE_VERSION HARDWARE_VERSION
//TODO: linux下如何实现函数名的拼接？自动生成一些代码  可以参考thritf？ 代码生成器相关
void test_2()
{
	printf("XNAME：%s \n", XNAME(44, 55));

	printf("version: [%s] \n", SYSTEM_VERSION);
	printf("%e \n", my_math(3, 4));
	//函数名拼接
	return;
}

//3: C99新增__VA_ARGS__,改变了可变参数不能用在宏中的定义。 
#define debug(...) printf(__VA_ARGS__)
void test_3()
{
	debug("test of __VA_ARGS__ [%d]:[%s] \n", 0, "test");
	debug("test of __VA_ARGS__ error \n");
}

//但是如果这样定义，会出错，因为当没有参数时，宏定义fmt后面的逗号是多余的
//fmt把输出的格式串提取出来了，上面是直接放在了...
#define debug1(fmt, ...) printf(fmt, __VA_ARGS__)
void test_3_1()
{
	debug1("test of __VA_ARGS__ [%d]:[%s] \n", 0, "test");
	// expected expression before ‘)’ token
	// debug1("test of __VA_ARGS__ error \n"); //这个应该会报错，因为多了一个空格
}
// 4: 用##来优化test_3中把fmt提取出来，如果没有参数就会报错
// 其他思路： 实际的输出用printf系列的其他函数 也可以写入文件
#define debug2(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define debug3(fmt, ...) fprintf (stderr, fmt, ##__VA_ARGS__)

void test_4()
{
	debug2("test of ##__VA_ARGS__ [%d]:[%s] \n", 0, "test");
	debug2("test of ##__VA_ARGS__ success \n");
	debug3("test of ##__VA_ARGS__ [%d]:[%s] \n", 0, "test");
	debug3("test of ##__VA_ARGS__ success \n");
}

// 5： va_list的测试 第一个参数是参数要相加的参数的个数
int sum(int num_args, ...)
{
   int val = 0;
   va_list ap;
   int i;

   va_start(ap, num_args);
   for(i = 0; i < num_args; i++)
   {
      val += va_arg(ap, int);
   }
   va_end(ap);
 
   return val;
}

void stdio_printf(const char* fmt, ...)
{
   int len, i;
   va_list ap;
   char buffer[256];
   
   va_start(ap, fmt);
   len = vsnprintf(buffer, sizeof(buffer), (const char*)fmt, ap);
   va_end(ap);
   printf("buffer: %s \n",buffer);
}

void test_5()
{
	printf("15 和 56 的和 = %d\n",  sum(2, 15, 56) );
	printf("add :%d \n", sum(3,2,3,4));
	stdio_printf("%s %d \n", "test_5", 5);
}
int main()
{
	//#的测试
	test_1();
	test_2();
	//##的测试 TODO linux下##来自动生成代码拼接函数名的实现
	test_3();
	test_3_1();
	test_4();
	test_5();
	return 0;
}