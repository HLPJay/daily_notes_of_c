/*************************************************************************************************
状态模式：
	大白话就是，状态的改变依赖于内部变量的改变；  
在设计的时候要特别注意，
	要将状态设计成闭环，就是当所有条件不满足的时候，返回初始状态并给出提示“没有此状态”

State模式:
	允许通过改变对象的内部状态而改变对象的行为，这个对象表现得就好像修改了它的类一样。
	状态模式主要解决的是当控制一个对象状态转换的条件表达式过于复杂时的情况。
	把状态的判断逻辑转译到表现不同状态的一系列类当中，可以把复杂的判断逻辑简化。

适用于：
	对象的行为，依赖于它所处的当前状态。行为随状态改变而改变的场景。
	适用于：通过用户的状态来改变对象的行为。  
状态接口：
状态模式：比较复杂，多种状态一定形成循环对其进行判断，状态的改变依赖于成员变量值的改变
比如：
	要有成员变量设置和得到此变量的值的功能
 		要有得到当前状态状态的指针变量指向
 		要有设置状态指针的操作
 		要有执行的操作（根据指针指向的状态，使用状态指针去调用状态指向的重写函数）并携带当前对象，
 			根据当前对象的成员变量的值进行不同的操作
    	如果条件不满足就对多个状态进行遍历，先释放当前对象指针，然后设置下一个状态的状态，去调用下一个状态的成员函数，
    	如果到最后一个状态还是不成立然后设置其状态为初始状态，在最后打印出没有此状态的信息。
实现方法;
	设定此类的某一状态值，根据此状态值去调用得到状态的函数，如果不满足就返回没有此状态，然后设置指针指向下一个状态。
**************************************************************************************************/
#include <iostream>
using namespace std;
class Worker;
//状态基类  子类实现不同状态的具体业务，提供对外接口
class State
{
public:
	virtual void doSomeThing(Worker *w) = 0;
	virtual ~State() {}
};

//状态管理者 根据传入的状态进行不同的函数调用
class Worker
{
public:
	Worker(){}
	~Worker(){
		if(m_currstate!= NULL)
		{
			delete m_currstate;
			m_currstate = NULL;
		}
	}

	int getHour(){  return m_hour;  }
	void setHour(int hour){  m_hour = hour;}

	State* getCurrentState(){  return m_currstate;  }
	void setCurrentState(State* state){  m_currstate = state;  }

	void doSomeThing()  //这里可以保存状态类型修改状态变更
	{  
		m_currstate->doSomeThing(this);  
	} //调用者指针管理状态类
private:
	int  m_hour;
	State  *m_currstate; //对象的当前状态
};

class State2;
class State1 : public State
{
public:
	void doSomeThing(Worker *w);
};

class State2 : public State
{
public:
	void doSomeThing(Worker *w);
};


void State1::doSomeThing(Worker *w)
{
	if (w->getHour() == 7 || w->getHour() == 8){
		cout << "吃早饭" << endl;
	}else{
		delete w->getCurrentState();  //状态1 不满足 要转到状态2
		w->setCurrentState(new State2);
		w->getCurrentState()->doSomeThing(w); //
	}
}

void State2::doSomeThing(Worker *w)
{
	if (w->getHour() == 9 || w->getHour() == 10){
		cout << "工作" << endl;
	}
	else
	{
		delete w->getCurrentState(); //状态2 不满足 要转到状态3 后者恢复到初始化状态
		w->setCurrentState(new State1); //恢复到当初状态
		cout << "当前时间点：" << w->getHour() << "未知状态" << endl;
	}
}
int main()
{
	Worker *w1 = new Worker;
	State1 * s1 = new State1(); //内部保证释放了
	w1->setCurrentState(s1);
	w1->setHour(7);
	w1->doSomeThing();

	w1->setHour(9);//内部根据业务判断  改变了状态
	w1->doSomeThing();

	delete w1;
	// system("pause");
	return 0;
}