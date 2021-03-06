/***********************************************************************************************
中介者模式：
    思想：两个类之间不能直接联系，而是通过第三方之间通信，从而将两个类之间的关系解耦
          将具体的两个同事之间的消息进行转发，从而使得两个同事之间避免了直接联系
    1.创建抽象的中介及同事类；中介持有同事类的指针，同事持有中介指针
    2.创建中介，创建同事，在创建具体的同事时，将中介通过基类构造函数传递，使得同事认识中介，然后中介认识具体的同事
    3.同事1去发送消息，在内部封装通过中介去发送消息，将本对象及消息进行传递，将消息发送到中介，然后中介进行判断将消息发送给谁
    具体消息转发逻辑在中介内部完成，调用另一个同事的接收消息的函数，从而得到别的同事发给自己的消息
************************************************************************************************/
#include <iostream>
#include <string>
using namespace std;
class Colleague;
class Mediator   //抽象中介者类
{
public:
    virtual void Send(string message, Colleague* col) = 0;
};


class Colleague
{//同事类
protected:
    Mediator* mediator;
public:
    Colleague(Mediator* temp)
    {
        mediator = temp;
    }
};
class Colleague1 : public Colleague   //同事一
{
public:
    Colleague1(Mediator* media) : Colleague(media){}
    void Send(string strMessage)
    {
        mediator->Send(strMessage, this);
    }
    void Notify(string strMessage)
    {
        cout << "同事一获得了消息 " << strMessage << endl;
    }
};
class Colleague2 : public Colleague   //同事二
{
public:
    Colleague2(Mediator* media) : Colleague(media){}
    void Send(string strMessage)
    {
        mediator->Send(strMessage, this);
    }
    void Notify(string strMessage)
    {
        cout << "同事二获得了消息 " << strMessage << endl;
    }
};
class ConcreteMediator : public Mediator   //具体中介者类
{
public:
    Colleague1 * col1;
    Colleague2 * col2;
    virtual void Send(string message, Colleague* col)
    {
        if (col == col1) //就是说如果传来的第1个同时发送的消息，那么我就调同事2的函数
            col2->Notify(message);
        else             //就是说如果传来的第2个同时发送的消息，那么我就调同事1的函数
            col1->Notify(message);
    }
};
int main()
{
    ConcreteMediator * m = new ConcreteMediator();//创建具体的中介者
    /*中介与同事之间互相认识*/
    Colleague1* col1 = new Colleague1(m);
    Colleague2* col2 = new Colleague2(m);
    m->col1 = col1;
    m->col2 = col2;
    /*同事1发送消息，实际上是将（消息+本对象）传递给中介，让中介去判断发送给谁
    中介者的判断逻辑来决定将消息发送给谁，将消息传到第二个同事那里*/
    col1->Send("吃饭了吗？");

    col2->Send("还没吃，你请吗？");
    // system("pause");
    return 0;
}