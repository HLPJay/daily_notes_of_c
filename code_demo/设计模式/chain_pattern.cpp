/****************************************************************************************************
链式
	概念(使用一个单链表，尾结点指向空，然后挨个遍历此结点指向的函数（这些函数都来自与抽象类中函数的重写）
	将则责任链上的对象统统创建出来，然后执行到哪个对象，哪个对象就去调用这个重写的函数方法)
适用于：
	链条式处理事情。工作流程化、消息处理流程化、事物流程化；
****************************************************************************************************/
#include <iostream>
using namespace std;
class CarHandle//责任链，设计一个类将其创建成链
{
public:
	virtual void HandleCar() = 0;
public:
	CarHandle *setNextHandle(CarHandle *carhandle)
	{
		this->carhandle = carhandle;
		return this->carhandle;
	}
protected:
	CarHandle *carhandle;
};

class CarHandleHead : public CarHandle
{
public:
	virtual void HandleCar()
	{
		cout << "车头处理" << endl;
		if (this->carhandle != NULL){  
			carhandle->HandleCar();  
		}
	}
};
class CarHandleBody : public CarHandle
{
public:
	virtual void HandleCar()
	{
		cout << "车身处理" << endl;
		if (this->carhandle != NULL){  
			carhandle->HandleCar();  
		}
	}
};
class CarHandleTail : public CarHandle
{
public:
	virtual void HandleCar()
	{
		cout << "车尾处理" << endl;
		if (this->carhandle != NULL){  
			carhandle->HandleCar(); 
		}
	}
};
class CarHandleyou : public CarHandle
{
public:
	virtual void HandleCar()
	{
		cout << "handle you success" << endl;
		if (this->carhandle != NULL)
		{
			carhandle->HandleCar();
		}
	}
};
int main()
{
	CarHandleHead *head = new CarHandleHead;
	CarHandleBody *body = new CarHandleBody;
	CarHandleTail *tail = new CarHandleTail; //这里是创建一些列的对象，对象包含下一个结点的指针
	CarHandleyou  *you = new CarHandleyou;
	head->setNextHandle(body);//在这里将这些函数使用链表串起来
	body->setNextHandle(tail);
	tail->setNextHandle(you);
	you->setNextHandle(NULL);

	//处理
	head->HandleCar();
	delete head;
	delete body;
	delete tail;
	delete you;
	// system("pause");
	return 0;
}