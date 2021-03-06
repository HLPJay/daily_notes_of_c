#include <iostream>
#include <string.h>

using namespace std;

/***************************
	为了适配不同的接口，做中间封装类

样例分析：
	定义一个基础桥接类，不同类型接口的封装，用不同的子类去构建。
	1：A平台有一套构建形状对象（颜色，形状类型属性）的接口。
	2：B平台有一套构建形状对象（颜色，形状类型属性）的接口。
	3：用一个桥接类，继承的方式实现不同的平台接口的封装。

扩展：
	可以扩展跨平台接口的管理
	可以扩展不同数据库接口的管理
***************************/
//假设有两个平台有两套接口 A平台 和B平台
class AplatfromInterface{
public:
	AplatfromInterface():m_shape(""), m_color("") {}
	~AplatfromInterface() {}
	void setShapeA(string &shape)
	{
		m_shape = shape;
	}
	void setColorA(string &color)
	{
		m_color = color;
	}
	string & getShapeA()
	{
		return m_shape;
	}
	string & getColorA()
	{
		return m_color;
	}
private:
	string m_shape;
	string m_color;
};

class BplatfromInterface{
public:
	BplatfromInterface():m_shape(""), m_color("") {}
	~BplatfromInterface() {}
	void setShapeB(string &shape)
	{
		m_shape = shape;
	}
	void setColorB(string &color)
	{
		m_color = color;
	}
	string & getShapeB()
	{
		return m_shape;
	}
	string & getColorB()
	{
		return m_color;
	}
private:
	string m_shape;
	string m_color;
};

//定义一个桥接基础类，生成目标对象
class Shape{

	//创建一个shape形状的具体对象，子类是真正的实现
public:
	virtual void CreateShape() = 0; //可以传入入参按需构造
	virtual void PrintShape() = 0 ;
	virtual ~Shape() {}
};

//A平台上形状类接口的封装 调用a平台上的类对象的接口
class AplatfromShape: public Shape{
public:
	AplatfromShape() {
		m_a = new AplatfromInterface();
	}
	~AplatfromShape() {
		if(m_a != nullptr)
		{
			delete m_a;
			m_a = nullptr;
		}
	}
	void CreateShape() override
	{
		string shape = "ashape";
		string color = "acolor";
		m_a->setShapeA(shape);
		m_a->setColorA(color);
	}

	void PrintShape() override
	{
		cout<<"platfrom:"<<m_a->getShapeA() <<"   "<<m_a->getColorA()<<endl;
	}
private:
	AplatfromInterface* m_a;
};

//B平台上形状类接口的封装 调用B平台上的类对象的接口
class BplatfromShape: public Shape{
public:
	BplatfromShape() {
		m_b = new BplatfromInterface();
	}
	~BplatfromShape() {
		if(m_b != nullptr)
		{
			delete m_b;
			m_b = nullptr;
		}
	}
	void CreateShape() override
	{
		string shape = "bshape";
		string color = "bcolor";
		m_b->setShapeB(shape);
		m_b->setColorB(color);
	}

	void PrintShape() override
	{
		cout<<"platfrom:"<<m_b->getShapeB() <<"   "<<m_b->getColorB()<<endl;
	}
private:
	BplatfromInterface* m_b;
};

int main()
{
	//使用的时候，不必过多关注接口 根据不同的平台，初始化稍微差异，其他不变
	//如果在A平台 这样用：
	Shape * a_platfrom = new AplatfromShape();
	a_platfrom->CreateShape();
	a_platfrom->PrintShape();
	delete a_platfrom; 
	a_platfrom = nullptr;

	//b 平台可以这样用
	Shape * b_platfrom = new BplatfromShape();
	b_platfrom->CreateShape();
	b_platfrom->PrintShape();
	delete b_platfrom; 
	b_platfrom = nullptr;
	return 0;
}