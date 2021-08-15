/****************************************************************************************************************
命令模式：
	说白了就是通过中间人专门去管理这个命令，将这个中间人称为管理命令类，由这个中间人完成指令的发送动作

适用于：
	是将一个请求封装为一个对象，从而使你可用不同的请求对客户端进行参数化；对请求排队或记录请求日志，以及支持可撤销的操作。

实现方法：
	创建命令：
		内部构造使用对象注入技术，完成对象调用的绑架  +  executecommand接口定义
		不同的命令中完成命令的执行者函数
			（executecommand重写）+ executecommand（去调用具体执行者的函数） 
*****************************************************************************************************************/
#include<iostream>
#include<string>
#include<list>
using namespace std;
class barbecuer //执行指令的类
{
public:
	void bakemutton(){
	  cout << "烤羊肉串" << endl;
	}
	void bakechickenwing(){
	  cout << "烤鸡翅" << endl;
	}
};

class Command   //命令基类，既包含了命令基类的赋值，也定义了相关操作的接口
{
protected://命令类很关键的一步是，告诉大家谁去接受这个指令
	barbecuer *receiver;//类的包含
public:
	Command(barbecuer*receiver) //命令类很关键的一步是，告诉大家谁去接受这个指令
	{  
		this->receiver = receiver;
	}
	virtual void executecommand() = 0;
};

//不同的命令执行1
class bakermuttoncommand :public Command //命令传送者
{
public:
	bakermuttoncommand(barbecuer*receiver) :Command(receiver){}
	void executecommand(){
		receiver->bakemutton();
	}//执行烤羊肉串
};

//不同的命令执行2
class bakechikenwingcommand :public Command //命令传送者
{
public:
	bakechikenwingcommand(barbecuer*receiver) :Command(receiver){}
	void executecommand(){
		receiver->bakechickenwing();
	}//执行烤鸡翅
};

class waiter //服务员单一，一次只能给一个对象下达指令
{
private:
	Command *command;
public:
	void setorder(Command*command) //对象注入技术，对象的赋值
	{
		this->command = command;
	}
	void notify(){
		command->executecommand();
	}
};

//这个类对单一服务员的时候，可有可无。只是为了说明下面的超级服务员做铺垫
class waiter2  //超级服务员，通过list可以给多个对象下达命令进行操作
{
private:
	list<Command*>orders;
public:
	void setorder(Command*command){
		orders.push_back(command);
	}
	void cancelorder(Command *command){}
	void notify()
	{
		list<Command*>::iterator iter = orders.begin();
		while (iter != orders.end())
		{
			(*iter)->executecommand();
			iter++;
		}
	}
};

int main()
{
	barbecuer *boy = new barbecuer;//创建一个执行动作的对象
	Command *bm1 = new bakermuttoncommand(boy);//来一个命令创建一次，传送一次命令
	//在bakermuttoncommand里面完成对executecommand的操作
	//命令类，在基类中完成命令的执行者赋值（使用初始化列表）
	bm1->executecommand();
	Command *bm2 = new bakermuttoncommand(boy);
	Command *bc1 = new bakechikenwingcommand(boy);

	waiter2 *girl = new waiter2();//使用超级服务员，去管理这些个命令
	girl->setorder(bm1);
	girl->setorder(bm2);
	girl->setorder(bc1);

	girl->notify();
	// system("pause");

	delete boy;
	delete bm2;
	delete bm1;
	delete bc1;
	delete girl;
	return 0;
}