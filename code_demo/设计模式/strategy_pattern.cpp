/********************************************************************************************
策略模式：
    将一系列策略进行封装，客户根据需求去调用相应的策略，实现很方便，后续添加策略时，只需继承接口并实现该策略
    1.创建策略类，不同的策略去继承同一个接口函数
    2.将策略组成一个算法本
    
    1.持有接口函数的指针，根据条件选择具体的算法，完成策略接口的赋值
    2.调用策略执行函数，在执行函数中完成具体的算法，在外界看来调用同一个函数接口可以执行不同的策略
***********************************************************************************************/
#include<iostream>
using namespace std;

class Strategy
{
public:
    virtual void discount() = 0;
};

class five : public Strategy
{
public:
    virtual void discount()
    {
        cout << "打5折" << endl;
    }
};

class six : public Strategy
{
public:
    virtual void discount()
    {
        cout << "打6折" << endl;
    }
};

class Context
{
public:
    Context(Strategy *strategy)
    {
        this->p = strategy;
    }

    void Operator()
    {
        p->discount();
    }
    
private:
    Strategy *p;
};

int main()
{
    Strategy *strategy = new five();//创建具体的算法对象
    Context *p = new Context(strategy);//扔向算法目录
    p->Operator();//指针指向哪个算法，那么此指针调用的就是谁的算法
    delete strategy;
    delete p;

    Strategy *strategy1 = new six();//创建具体的算法对象
    Context *p1 = new Context(strategy1);//扔向算法目录
    p1->Operator();//指针指向哪个算法，那么此指针调用的就是谁的算法

    delete strategy1;
    delete p1;
    // system("pause");
    return 0;
}