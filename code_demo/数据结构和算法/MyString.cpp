//有次面试让裸写实现一个string的封装，这里做相关的笔记：




#include <stdio.h>
#include <stdlib.h>
#include <string.h>


class MyString{
public:
	//构造函数 析构函数  拷贝构造函数  赋值构造函数
	MyString() { mystr = NULL; }
	~MyString()
	{
		if(mystr != NULL)
		{
			free(mystr);
			mystr = NULL;
		}
	}
	//= NULL是没有参数和上面冲突了
	MyString(const char* cstr );
	MyString(const MyString &s);
	MyString &operator=( const MyString &str);

	void Print() 
	{
		printf("%s \n", mystr);
	}
private:
	char* mystr = NULL; //成员变量一定要注意初始化，在这里或者构造函数后面
};

MyString::MyString(const char* cstr)
{
	if(cstr == NULL)
	{
		mystr= NULL;
		// mystr = (char*)malloc(1);//申请内存过小，可能导致踩内存
		// mystr[0] = '\0';
		return ;
	}

	mystr = (char*)malloc((int)strlen(cstr)+1);
	memset(mystr, 0 , (int)strlen(cstr)+1);
	memcpy(mystr, cstr, (int)strlen(cstr)+1);
}

MyString::MyString(const MyString &s)
{
	if(this == &s)
	{
		return;
	}
	if(mystr != NULL)
	{
		free(mystr);
	}
	mystr = (char*)malloc((int)strlen(s.mystr)+1);
	memset(mystr, 0 , (int)strlen(s.mystr)+1);
	memcpy(mystr, s.mystr, (int)strlen(s.mystr)+1);
}

MyString & MyString::operator=( const MyString &str)
{
	if(this == &str)
	{
		return *this;
	}
	if(mystr != NULL)
	{
		free(mystr);
	}
	mystr = (char*)malloc((int)strlen(str.mystr)+1);
	memset(mystr, 0 , (int)strlen(str.mystr)+1);
	memcpy(mystr, str.mystr, (int)strlen(str.mystr)+1);
	return *this;
}
int main()
{
	MyString str("123");
	str.Print();
	MyString str1 = "1233";
	str1.Print();
//有问题的
	MyString str2(str1);
	str2.Print();
//有问题的  CMyString str3 = str;

	MyString str3 ;
	str3 = str;
	str3.Print();

	MyString str4;
	MyString str5;
	str4 = str5 = str2;
	str4.Print();
	str5.Print();

	return 0;
}

//malloc free和new delete的底层实现
//智能指针的实现