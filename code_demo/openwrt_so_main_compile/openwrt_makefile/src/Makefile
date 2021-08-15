CFLAGS    +=-fPIC -shared 
LFLAGS	  +=-fPIC -shared 
INCLUDES  +=-I./../include/
SOURCES_test   =$(wildcard *.c)
OBJ_test       =$(patsubst %.c, %.o, $(SOURCES_test))
TARGET_test    =libfun_so

$(TARGET_test):$(OBJ_test)
	$(CC) $(OBJ_test) $(LFLAGS) -o ../$(TARGET_test).so
	@rm -rf $(OBJ_test)

%.o: %.c
	$(CC) $(INCLUDES) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ_test) ../$(TARGET_test).so