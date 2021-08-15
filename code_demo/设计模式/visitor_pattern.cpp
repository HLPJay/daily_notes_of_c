/*************************************************************************************************************
适用于：
	把数据结构 和 作用于数据结构上的操作 进行解耦合;
	适用于数据结构比较稳定的场合
访问者模式总结：
	访问者模式优点是增加新的操作很容易，因为增加新的操作就意味着增加一个新的访问者。
	访问者模式将有关的行为集中到一个访问者对象中。
	那访问者模式的缺点是是增加新的数据结构变得困难了


实现数据结构和操作数据结构的动作==》进行有效的分离
实现方法： 
	分别定义访问者接口（数据结构的基类指针）    
		公园部分访问接口（访问者的指针接口）+  接受访问的内部实现了访问函数的调用
	如果要管理访问者增加超级访问者，使用集合将其封装，然后遍历集合
**************************************************************************************************************/
/*  案例需求：
比如有一个公园，有一到多个不同的组成部分；
	该公园存在多个访问者：清洁工A负责打扫公园的A部分，清洁工B负责打扫公园的B部分，
	公园的管理者负责检点各项事务是否完成，上级领导可以视察公园等等。
也就是说，对于同一个公园，不同的访问者有不同的行为操作，
	而且访问者的种类也可能需要根据时间的推移而变化（行为的扩展性）*/
#include <iostream>
#include <list>
#include <string>
using namespace std;
class  ParkElement;
//定义访问者类  衍生不同各种各样的访问者
class Visitor//不同的访问者 访问公园完成不同的动作 
{
public:
	virtual void visit(ParkElement *park) = 0;
};

//目标访问对象  支持各种不同的业务
class ParkElement//公园不同的部分接受不同的访问者
{
public:
	virtual void accept(Visitor *v) = 0;
};

//办理业务A
class ParkA : public ParkElement  //公园A部分接受访问者
{
public:
	virtual void accept(Visitor *v){  
		v->visit(this);
	}//传来的谁，去回调它是访问函数
};
//办理业务B
class ParkB : public ParkElement//公园B部分接受访问者
{
public:
	virtual void accept(Visitor *v)  {
		v->visit(this);
	}
};

//不同的职能呢个 管理访问多个业务 可以定制
class Park : public ParkElement
{  //公园的部分可以进行集中管理
public:
	Park(){
		m_list.clear();  
	}
	void setPart(ParkElement *e){
		m_list.push_back(e);
	}
public:
	void accept(Visitor *v)
	{
		for (list<ParkElement *>::iterator it = m_list.begin(); it != m_list.end(); it++){
			(*it)->accept(v);
		}
	}
private:
	list<ParkElement *> m_list;
};

//不同的访问者类型   根据不同的访问目的子类，实现不同的业务
class VisitorA : public Visitor//访问者A
{
public:
	virtual void visit(ParkElement *park){  
		cout << "清洁工A 访问 公园A 部分，打扫卫生完毕" << endl;
	}
};

class VisitorB : public Visitor//访问者B
{
public:
	virtual void visit(ParkElement *park){  
		cout << "清洁工B 访问 公园B 部分，打扫卫生完毕" << endl;
	}
};

//领导者作为访问者对所有业务做管理
class VisitorManager : public Visitor//访问者管理员
{
public:
	virtual void visit(ParkElement *park){
		cout << "管理员 检查 此部分卫生情况" << endl;
	}
};
int main()
{
	//不同类型的访问者
	VisitorA *visitorA = new VisitorA;//创建访问者A
	VisitorB *visitorB = new VisitorB;//创建访问者B

	//专门的办理不同的业务
	ParkA *partA = new ParkA;//创建数据结构  A
	ParkB *partB = new ParkB;//创建数据结构  B

	// 接待访问者 执行对应的业务
	partA->accept(visitorA);  //公园接受访问者A访问 +  在这个函数中封装了visitorA去访问公园A部分
	partB->accept(visitorB);  //公园接受访问者B访问 + 在这个函数中封装了visitorA去访问公园B部分

	delete visitorA;
	delete visitorB;

	//作为访问者  可以做复合业务
	VisitorManager *visitorManager = new VisitorManager;
	Park * park = new Park;
	park->setPart(partA);  //将A部分加入容器
	park->setPart(partB);  //将B部分加入容器
	park->accept(visitorManager); //管理员去检查A部分 +  管理员去检查B部分
	//遍历容器，接受visitorManager访问，去调用visitorManager的访问函数
	// system("pause");
	delete partA;
	delete partB;
	delete visitorManager;
	delete park;

	return 0;
}