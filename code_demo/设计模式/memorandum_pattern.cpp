/**********************************************************
备忘录模式：
	就是用一个类来保存一个类的中间状态；；；；；
	如果想大规模的保存和管理状态那么就要是用一个管理类-------
	-------就好比命令模式的时候，命令太多的话，我们在把他给管理起来------------
发起类中要有：
	设置状态的成员函数，创建包含当前状态信息的对象
	通过管理类中的，发起类的指针传递，设置当前类中的状态。
*********************************************************/
#include <iostream>
#include <string>
using namespace std;
class Memo;
class Originator//初始状态类，我们想保存其中一个状态
{
public:
	string state;
	Memo* CreateMemo();
	void SetMemo(Memo* memo);
	void Show(){
		cout << "状态：" << state << endl;
	}
};

class Memo//备忘录类，保存状态所使用到的结构
{
public:
	string state;
	Memo(string strState){
		state = strState;
	}
};


Memo* Originator::CreateMemo(){
	return new Memo(state);
}//通过返回一个包含中间状态的对象

void Originator::SetMemo(Memo* memo){  
	state = memo->state;
}

class Caretaker//管理者类
{
public:
	Memo* memo;//管理者类中包含了，保存状态的结点对象的指针
};  //这样就可以通过这个指针，操作结点的状态


int main()
{
	Originator* on = new Originator();
	on->state = "on";
	on->Show();
	Caretaker* c = new Caretaker();//使用此指针指向这个，包含中间状态的对象
	c->memo = on->CreateMemo();//保存了这个状态，可以恢复

	on->state = "off";
	on->Show();

	on->SetMemo(c->memo); //恢复
	on->Show();
	// system("pause");
	return 0;
}