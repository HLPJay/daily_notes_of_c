//解释器模式
#include <iostream>
#include <string>
using namespace std;

//只是提供一个解释器的中间类，为解释语句做辅助功能，输入要时特定的格式
class Context
{
	public: //通过此类进行中间变量的存储和运算，得到数据通过此类，修改也是通过此类
		Context(int num){ m_num = num;}
	public:
		void setNum(int num){ m_num = num;}
		int getNum(){ return m_num;}
		void setRes(int res){ m_res = res;}
		int getRes(){ return m_res;}
	private:
		int m_num;
		int m_res;
};

//解释器基类   调用接口的入口而已，子类真正调用入参，根据不同的解释做处理 
//可以定义一个基类 实现多个子类不同的解释功能，如运算符+，-，*，/
class Expression
{
public:
	virtual void interpreter(Context *context) = 0;
};

//可以对输入做不同的解析  这里只做了输出
class PlusExpression : public Expression
{
public:
	virtual void interpreter(Context *context)
	{
		int num = context->getNum();
		num++;
		context->setNum(num);
		context->setRes(num);
	}
};

//可以对输入做不同的解析  这里只做了输出
class MinusExpression : public Expression
{
public:
	virtual void interpreter(Context *context)
	{
		int num = context->getNum();
		num--;
		context->setNum(num);
		context->setRes(num);
	}
};

int main()
{  
	//通过一个中间类进行数据的存储转化和设置
	Context *explain = new Context(10);//设置要解释的内容
	Expression *e1 = new PlusExpression;//设置解释器
	e1->interpreter(explain);//调用解释器内的解释函数
	cout << "PlusExpression:" << explain->getRes() << endl;//得到数据结果

	Expression *e2 = new MinusExpression;
	e2->interpreter(explain);
	cout << "MinusExpression:" << explain->getRes() << endl;

	delete explain;
	delete e2;
	delete e1;
	// system("pause");
	return 0;
}