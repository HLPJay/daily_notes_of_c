#注意 这里的变量其实依赖上层环境的 调用openwrt的gcc和makefile已经定义的一些规则
DEBUG 			+=-DUSE_DEBUG
CFLAGS			+=-Wall
INCLUDES  		+=
LIB_NAMES 		+=-lfun_so
SOURCES_test   	=$(wildcard *.c)
OBJ_test       	=$(patsubst %.c, %.o, $(SOURCES_test))
TARGET			=main_test

#这里依赖的so，linux只会去/usr/lib目录下去寻找  所以运行时要处理
$(TARGET):$(OBJ_test)
	$(CC) $(OBJ_test) $(LIB_NAMES) -o ../$(TARGET)
	@rm -rf $(OBJ_test)
	
#compile
%.o: %.c
	$(CC) $(INCLUDES) $(DEBUG) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ_test) ../$(TARGET)