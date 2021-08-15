/*
模板模式：
	有没有发现这个是建造者模式的简化版本：将此版本升级就编程了建造者模式
	将一组行为升级成一个模板，从而实现了创建某一类产片的模板，这样就完成了一类产品的创建（这个产品依赖很对创建过程
	将此创建过程进行封装）

实现方法：
	创建一个抽象接口类定义接口;
	不同的子类去继承实现接口，然后在创建子类对象时，返回一个基类指针，然后使用基类指针去调用将此对象的创建过程进行封装的Make函数
*/
#include <iostream>
using namespace std;
class MakeCar
{
public:
	virtual void makeHead() = 0;
	virtual void makeBody() = 0;
	virtual void makeTail() = 0;
public:   
	//把一组行为 变成 一个模板（编程一个函数）；调用一个函数就可以实现一组函数的调用
	void make()
	{
		makeHead();
		makeBody();
		makeTail();
	}
};
class MakeBus : public MakeCar
{
public:
	virtual void makeHead(){
		cout << "bus 组装 车头" << endl;
	}
	virtual void makeBody(){
		cout << "bus 组装 车身" << endl;
	}
	virtual void makeTail(){
		cout << "bus 组装 车尾" << endl;
	}
};
class MakeJeep : public MakeCar
{
public:
	virtual void makeHead(){
		cout << "Jeep 组装 车头" << endl;  
	}
	virtual void makeBody(){
		cout << "Jeep 组装 车身" << endl;
	}
	virtual void makeTail(){
		cout << "Jeep 组装 车尾" << endl;
	}
};
int main()
{
	MakeCar *bus = new MakeBus;
	bus->make();//此函数对创建过程进行了封装

	MakeCar *jeep = new MakeJeep;
	jeep->make();//此函数对创建过程进行了封装
	delete bus;
	delete jeep;
	// system("pause");
	return 0;
}