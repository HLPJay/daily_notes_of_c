/************************************************
代理模式：
    1.定义代理接口
    2.代理类和被代理类去实现相同的接口
    3.在代理类中创建被代理的类，在接口中去调用被代理类的接口
使用：
    1.创建代理对象，在代理对象中实现了调用被代理类的接口
      从而看起来好像是调用的同一个接口，但是实际执行的是被代理类的函数
*************************************************/
#include <iostream>
#include <string>
using namespace std;

class Interface
{
public:
    virtual void Request() = 0;
    virtual ~Interface(){}
};
//目标实际类的定义
class RealClass : public Interface
{
public:
    virtual void Request()
    {
        cout << "真实的请求" << endl;
    }
};

//不用这个基类也可以的 这是代理类的实现
//通过代理类对目标类对象进行操作  看起来操作这个类，实际控制目标类对象
class ProxyClass : public Interface
{
private:
    RealClass* m_realClass;
public:
    virtual void Request()
    {
        m_realClass = new RealClass();
        m_realClass->Request();
        delete m_realClass;
    }
};


int main()
{
    ProxyClass* test = new ProxyClass();
    test->Request();
    return 0;
}