/*************************************************************************
组合模式：
	是把单个对象和组合对象放在一起使用，把操作单个对象，和操作整个对象搞的没有太大差别
	在遍历的时候，如果存储的是叶子结点，那么调用叶子结点的显示函数
	如果存储的是组合对象，那么递归调用自己的显示函数
实现方式：
	定义单个对象和一组对象的接口，提供统一的操作函数
	在定义结点的时候，都将其创建出的对象，使用抽象类指针，指向这个创建的对象，
	然后将此指针，添加到相对应的结点。
***************************************************************************/
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Component
{
public:
	Component(string name){this->name = name;}
	virtual void add(Component *) = 0;
	virtual void remove(Component *) = 0;
	virtual void display(int) = 0;
public:
	string name;
};

class Leaf :public Component
{
public://子类继承父类，父类初始化要使用初始化列表
	Leaf(string name) :Component(name){}
	void add(Component *c)
	{   cout << "leaf cannot add" << endl;}
	void remove(Component *c)
	{  cout << "leaf cannot remove" << endl;  }
	void display(int depth)
	{
		string str(depth, '-');
		str += name;
		cout << str << endl;
	}
};


class Composite :public Component
{
private:
	vector<Component*> component;
public:
	Composite(string name) :Component(name){}
	void add(Component *c)
	{
		component.push_back(c);//容器操作，将其放入容器
	}
	void remove(Component *c)
	{
		for (vector<Component*>::iterator iter = component.begin(); iter != component.end(); iter++)
		{
			if (*iter == c)
			{
				component.erase(iter);
			}
			iter++;
		}
	}
	void display(int depth)
	{
		string str(depth, '-');
		str += name;//在调用打印的时候先将组合结点的名字打印出来
		cout << str << endl;//如果是组合结点直接打印
		for (vector<Component*>::iterator iter = component.begin(); iter != component.end(); iter++)
		{
			(*iter)->display(depth + 1);//如果是叶子结点，那么调用叶子结点的打印函数，将此-符号数量加1传递给叶子结点进行打印
		}         //每次多向下递归一层，就多向下遍历加1
	}
};

int main()
{
	Component *p = new Composite("小李");//组合结点Node中有一个vector向量
	p->add(new Leaf("小王"));//这里面存储的是叶子结点，所以调用叶子结点的Display函数
	p->add(new Leaf("小强"));


	Component *sub = new Composite("小虎");//在这里是把组合对象当成单个对象使用
	sub->add(new Leaf("小王"));  
	sub->add(new Leaf("小明"));   
	sub->add(new Leaf("小柳"));  
	
	p->add(sub);
	p->display(0);//在调用显示函数的时候，如果存储的是叶子结点，那么调用叶子结点的显示函数
	 //如果存储的是组合结点，那么递归调用组合结点的显示函数

	sub->display(2);
	// system("pause");
	return 0;
}