/*************************************************************************
观察者模式：
    思想：自己在玩游戏，怕老板发现，自己有很担心，委托秘书来完成观察的任务，当观察到某一刻的变化时，
        执行指定的任务，完成状态的切换
	
	1.创建客户和观察者
	2.客户将自己注册到观察者那里，当观察者得到某一消息时，挨个通知每个客户
	3.观察者者通知携带有消息，调用客户的接收消息的函数，从而完成了所有客户的委托

   这里分为两种模型： 
   		 1 直接将内容发送给各个观察者 --- 推送    
   		 2  将主题直接发送给观察者 ---拉模型
4.观察者使用一个容器来管理每个客户，从而挨个通知
***************************************************************************/
#include <iostream>
#include <list>
using namespace std;
class Subject;
class Observer
{
public:
    Observer(){}
    virtual ~Observer(){}
    virtual void update(Subject *subject) = 0;
    virtual void update(string content) = 0;
};

//注册中心
class  Subject
{
public:
    Subject() {}
    virtual ~ Subject() {}
    virtual string getContent() = 0;
    virtual string getAbstractContent() = 0;
    virtual void setContent(string content) = 0;
    // 订阅主题
    void attach(Observer *observer) {
        observers.push_back(observer);
    }
    // 取消订阅
    void detach(Observer *observer) {
        observers.remove(observer);
    }
 
    // 通知所有的订阅者
    virtual void notifyObservers() {
        for(Observer *reader: observers) {
            // 拉模型  (也是有推送的性质,只是粒度小一些)
            //把主题对象推过去，让观察者自己拉取该主题内容
            reader->update(this);
        }
    }
    
    // 通知所有的订阅者
    virtual void notifyObservers(string content) {
        for(Observer *reader: observers) {
            // 推模型
            reader->update(content);
        }
    }
private:
    list<Observer *> observers;    // 保存注册的观察者
};
 
//观察者  订阅者 主动拉取关注的内容
class Reader : public Observer
{
public:
    Reader() {}
    virtual ~Reader() {}
    virtual void update(Subject *subject) {
        // 调用对应的方法去拉取内容 subject->getContent()
        cout << m_name << "收到报纸和阅读它, 具体内容" << subject->getContent() << endl;
    }
    virtual void update(string content) {
        // 推模型
        cout << m_name << "收到报纸和阅读它, 具体内容" << content << endl;
    }
    string getName() {
        return m_name;
    }
    void setName(string name) {
        m_name = name;
    }
private:
    string m_name;
};

//观察者的目标  发布者
class NewsPaper: public Subject
{
public:
    NewsPaper() {}
    virtual ~NewsPaper() {}
 
    void setContent(string content) {
        m_content = content;
        notifyObservers();
    }
    virtual string getContent() {
        return m_content;
    }
    virtual string getAbstractContent() {
        return  "摘要:";
    }
private:
    string m_content;
};

int main()
{
    // 创建一个报纸主题
    NewsPaper *subject = new NewsPaper();
 
    // 创建观察者，读者
    Reader *reader1 = new Reader();
    reader1->setName("reader1");
 
    Reader *reader2 = new Reader();
    reader2->setName("reader2");
 
    Reader *reader3 = new Reader();
    reader3->setName("reader3");
    subject->attach(reader1);
    subject->setContent("notifyObservers - 1");
    cout << "-----------------------" << endl;
    subject->attach(reader2);
    subject->attach(reader3);
    subject->setContent("notifyObservers -1 -2 -3 ");

    delete reader1;
    delete reader2;
    delete reader3;
    delete subject;
    return 0;
}