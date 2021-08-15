/************************************************
实现c++的单例：
	1：单例的特性：唯一的对象，不允许私自构造，不允许直接修改，提供特定的接口，线程安全
	2：实现方案：	
		static特性，栈内存和堆内存
		class类的构造函数的特性（private，public，protected），delete的特性，限制允许构造
		多线程安全与正常的释放资源
************************************************/

//其实  不用堆内存，用栈内存是最方便的


#include <iostream>
#include <mutex>
using namespace std;

class Singleton{
private:
	Singleton() = default;
	Singleton(const Singleton & s) = delete;
	Singleton &operator = (const Singleton &s) = delete;
	~Singleton(){};

	class FreeSingleton
	{
		public:
			~FreeSingleton()
			{
				cout<<"start free singletion \n";
				if(m_sig_instance != NULL)
				{
					cout<<"free singletion \n";
					delete m_sig_instance;
					m_sig_instance = NULL;
				}
			}
	};

public:
	//对静态成员操作，所以控制成static
	static Singleton * GetInstence()
	{
		if(Singleton::m_sig_instance == NULL)
		{
			m_mutex.lock();
			if(Singleton::m_sig_instance == NULL)
			{
				m_sig_instance = new Singleton();
			}
			m_mutex.unlock();
		}
	}

	void Print()
	{
		cout<<"Singleton Instence.\n ";
	}

private:
	static Singleton * m_sig_instance;
	static std::mutex m_mutex;
	static FreeSingleton m_freed;

};

Singleton * Singleton::m_sig_instance = NULL;
std::mutex Singleton::m_mutex;
//借助static 栈的特性，通过析构释放对应的内存 注意这里的类型和static变量的作用域取值
Singleton::FreeSingleton Singleton::m_freed; 

int main()
{
	Singleton::GetInstence()->Print();
	return 0;
}