// 示例2： 推送模型 主题类中保存所有订阅者  有消息主动推送
#include <iostream>
#include <vector>
#include <string>
using namespace std;
class Secretary;
class PlayserObserver//玩游戏的同事类（观察者）
{
public:
    PlayserObserver(string name)//通过构造函数完成观察者的初始化
    {
        m_name = name;
    }
    void update(string action)
    {
        cout << m_name << "： 收到:" << action << endl;
    }
private:
    string m_name;
};

class Secretary//秘书类（主题对象，通知者）
{
public:
    void addObserver(PlayserObserver *o)
    {
        v.push_back(o);
    }
    void Notify(string action)
    {
        for (vector<PlayserObserver *>::iterator it = v.begin(); it != v.end(); it++)
        {
            (*it)->update(action);//这里是一个推的模型
        }
    }//此处容器拥有观察者对象的指针，然后带着参数去遍历不同观察者的统一接口的接受函数，显示接受到此消息
private:
    vector<PlayserObserver *> v;
};
//注册进去  主动注册 有消息主动推送
int main()
{
    Secretary *s1 = new Secretary; //subject 被观察者
 
    PlayserObserver *po1 = new PlayserObserver("小张");//具体的观察者 被通知对象
    PlayserObserver *po2 = new PlayserObserver("小李");
    s1->addObserver(po1);//将其放进通知队列
    s1->addObserver(po2);
 
    s1->Notify("老板来了");//此处的通知函数是带有参数的
    delete po1;
    delete po2;
    delete s1;
    return 0;
}