#include <iostream>
#include <string.h>
using namespace std;

class Person
{//抽象类提供接口
public:
	virtual Person *Clone() const = 0;//返回基类指针
	virtual void printT() const = 0;
	virtual ~Person() {}
};

class JavaProgrammer : public Person
{
private:
	string  m_name;
	int  	m_age;
	char  	*m_resume;

public:
	JavaProgrammer()//无参构造函数
	{
		this->m_name = "";
		this->m_age = 0;
		m_resume = NULL;
	}
	
	JavaProgrammer(string name, int age)//有参构造函数
	{
		this->m_name = name;
		this->m_age = age;
		m_resume = NULL;
	}

	~JavaProgrammer()
	{
		if (m_resume != NULL)
		{
			free(m_resume);
			m_resume = NULL;
		}
	}
	//实现自我复制功能 这里应该调用拷贝构造函数  但是涉及深拷贝与浅拷贝问题，这里直接做内存的处理
	Person *Clone() const override
	{
		JavaProgrammer *p = new JavaProgrammer();
		p->m_resume = new char[strlen(this->m_resume) + 1];
		p->m_name = this->m_name;
		strcpy(p->m_resume, this->m_resume);
		// return new JavaProgrammer(*this); 浅拷贝
		return p;
	}
	void setResume(const char *resume)
	{
		m_resume = new char[strlen(resume) + 1]; //因为是字符串末尾多一个'\0'
		strcpy(m_resume, resume);
	}
	void printT() const override
	{
		cout << "m_name:" <<  m_name << "\t" << "m_age:" << m_age << endl;
		if (m_resume != NULL)
		{
			cout << m_resume << endl;
		}
	}
};

int main()
{
	JavaProgrammer javaperson1("李四", 160);
	javaperson1.setResume("我是C++程序员");//字符串操作，使用函数完成初始化操作。注意字符串操作的时候要进行'\0''
	//如果不使用这个函数进行封装，那么就会出现深拷贝和浅拷贝的问题  
	//如果没有这个函数，那么在指针自我拷贝的时候吗，我们就要把内存给创建出来，然后在去进行赋值操作
	javaperson1.printT();//自己打印自己的信息
	Person *p2 = javaperson1.Clone();  //对象具有自我复制功能 注意深拷贝和浅拷贝问题
	p2->printT();
	if(p2 != NULL)
	{
		delete p2;
		p2 = NULL;
	}
	return 0;
}