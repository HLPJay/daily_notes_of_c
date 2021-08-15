#ifndef _LOG_H_
#define _LOG_H_
#include <string.h>
 
/*通用字符串存储大小定义*/
#define STR_COMM_SIZE 128
#define STR_MAX_SIZE 1024
 
#define MAX_LOG_FILE_NUM		  (3)
 
#define NUMBER(type) sizeof(type)/sizeof(type[0])
 
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
 
/*日志类型*/
enum
{
    LOG_DEBUG = 0,/*调试日志*/
    LOG_ERROR,/*错误日志*/
    LOG_WARNING,/*告警日志*/
    LOG_ACTION,/*运行日志*/
    LOG_SYSTEM,/*系统日志*/
    BUTTOM
};
 
/*将日志输出到终端*/
#define PRINT_LOG_TO_TERM (0)
/*将日志输出到文件中*/
#define PRINT_LOG_TO_FILE (1)
 
/*调试日志宏定义*/
#define DEBUG_PRINT 0
#define LOG_PRINT(fmt, ...) do{\
	if(DEBUG_PRINT)\
	{\
		printf(fmt"  [line:%d] [%s]\n", ##__VA_ARGS__, __LINE__, __FUNCTION__);\
	}\
}while(0);
 
/*错误日志打印(在日志打印模块还未启动时使用)*/
#define LOG_ERR(fmt, ...) do{\
	printf("[ERROR]  "fmt"  [line:%d] [%s]\n", ##__VA_ARGS__, __LINE__, __FUNCTION__);\
}while(0);
 
 
 
/*存储日志标记. 0-不存储日志, 1-存储日志*/
extern unsigned long g_ulPrintLogPlaceFlag;
 
/*是否打印调试日志标记,0-不打印调试日志,1-打印调试日志*/
extern unsigned long g_ulPrintDebugLogFlag;
 
unsigned long LOG_PrintLog(unsigned char ucType, unsigned char *pucLogInfo);
 
 
/*日志打印宏定义*/
#define LOG_INFO(type, fmt, ...) do{\
	if(PRINT_LOG_TO_TERM == g_ulPrintLogPlaceFlag) \
	{ \
		if((0 == g_ulPrintDebugLogFlag) && (LOG_DEBUG == type)) \
		{\
			break;\
		}\
    	unsigned char ucLogInfo[STR_MAX_SIZE] = {0}; \
		snprintf((char *)ucLogInfo, sizeof(ucLogInfo) - 1, fmt"  [%s] [line:%d] [%s]\n", ##__VA_ARGS__, __FILENAME__, __LINE__, __FUNCTION__); \
		LOG_PrintLog(type, ucLogInfo); \
	} \
	else \
	{ \
		unsigned char ucLogInfo[STR_MAX_SIZE] = {0}; \
		snprintf((char *)ucLogInfo, sizeof(ucLogInfo) - 1, fmt"  [%s] [line:%d] [%s]\n", ##__VA_ARGS__, __FILENAME__, __LINE__, __FUNCTION__); \
		LOG_PrintLog(type, ucLogInfo); \
	} \
}while(0)
 
 
/*是否打印调试日志标记,0-不打印调试日志,1-打印调试日志*/
extern void LOG_SetPrintDebugLogFlag(unsigned long flag);
/*存储日志标记. 0-不存储日志, 1-存储日志*/
extern void LOG_SetPrintLogPlaceFlag(unsigned long flag);
 
extern unsigned long LOG_Init(const char* ucLogFileName, unsigned long ulFileSize);
extern void LOG_Destroy(void);
 
#endif //_LOG_H_
 
 
 
 