makefile实现项目的编译，一直是我



## 1：实现编译静态库：

```makefile
CC          =gcc
AR			=ar
CFLAGS		=-Wall
INCLUDES    =-I./../include/
SOURCES	 	=$(wildcard *.c)
OBJ         =$(patsubst %.c, %.o, $(SOURCES))
TARGET      =libfun

#链接生成的.o文件为一个静态库
$(TARGET):$(OBJ)
	@echo "start linked and compiled files......"
	$(AR) rv $(TARGET).a $(OBJ)
	@rm -rf $(OBJ)

#隐式规则  供OBJ使用 编译.c文件对应的.o文件
%.o: %.c
	$(CC) $(INCLUDES) -c -Wall $< -o $@
		
.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ) $(TARGET).a 
```

## 2：实现编译动态库：

```makefile
CC        =gcc
CFLAGS    =-fPIC -shared 
LFLAGS	  =-fPIC -shared 
SOURCES   =$(wildcard *.c)
INCLUDES  =-I./../include/
OBJ       =$(patsubst %.c, %.o, $(SOURCES))
TARGET    =libfun_so

#链接 中间缺少的相关参数  如lib链接 头链接 可以新增变量加入
$(TARGET):$(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o $(TARGET).so
	@rm -rf $(OBJ)
	
#编译 
%.o: %.c
	$(CC) $(INCLUDES) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ) $(TARGET).so
```

