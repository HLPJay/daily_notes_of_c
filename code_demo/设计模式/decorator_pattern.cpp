/*********************************************************************************************************************
装饰模式又叫做包装模式。
	通过一种对客户端透明的方式来扩展对象的功能，是继承关系的一个替换方案。
装饰模式:
	就是把要(添加的附加功能)分别放在单独的类中，并让这个类包含它要装饰的对象，
	当需要执行时，客户端就可以有选择地、按顺序地使用装饰功能包装对象。

适用于：
	装饰者模式:  动态的给一个对象添加一些额外的职责。就增加功能来说，此模式比生成子类更为灵活。

通过定义一个接口，然后将不同的子类的功能挨个添加到此接口当中,
	完成对对象的功能的添加比通过继承来扩充类的功能强大很多。

通过定义一个抽象类接口函数，然后子类去实现它。
	然后添加一项功能就派生出一个类，然后将要装饰的对象扔进去

使用基类指针指向其传递来的对象，去调用它原来的功能。
然后定一个装饰类指针指向装饰对象（被装饰的对象） 调用接口函数（接口函数去调用它原来的自己的接口）
**********************************************************************************************************************/
#include <iostream>
using namespace std;

class Car
{
public://定义一个抽象类，完成接口的定义
	virtual void show() = 0;
};

class RunCar : public Car
{
public:
	void run(){ cout << "可以跑" << endl;}
	virtual void show()
	{  run();  }
};

class SwimCarDirector : public Car
{
public:
	SwimCarDirector(Car *p){  m_p = p;}
	void swim(){  cout << "可以游" << endl;}
	virtual void show()
	{
		m_p->show();
		swim();
	}
	private:
		Car *m_p;//通过基类指针进行操作
};
class FlyCarDirector : public Car
{
public:
	FlyCarDirector(Car *p){m_p = p;}

	void fly(){  
		cout << "可以飞" << endl;
	}
	virtual void show()
	{
		m_p->show();
		fly();
	}
private:
	Car *m_p;//通过基类指针进行操作
};

int main()
{
	Car *runcar = new RunCar;
	runcar->show();

	cout << "车开始装饰swim" << endl;
	SwimCarDirector *swimCar = new SwimCarDirector(runcar);
	swimCar->show();

	cout << "车开始装饰fly" << endl;
	FlyCarDirector *flyCar = new FlyCarDirector(swimCar);
	flyCar->show();

	delete flyCar;
	delete swimCar;
	delete runcar;
	return 0;
}