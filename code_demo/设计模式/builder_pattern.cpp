#include <string.h>
#include <iostream>
using namespace std;

//造房子 可以模块化构造然后组装 这里我们定义房子的基本属性  ==》目标建筑
class House
{
public:
	void setFloor(string floor){
		this->m_floor = floor;
	}
	void setWall(string wall){
		this->m_wall = wall;
	}

	void setDoor(string door){
		this->m_door = door;
	}

	string getFloor(){
		return m_floor;
	}

	string getWall(){
		return  m_wall; 
	}

	string getDoor(){
		return m_door;
	}
private:
	string   m_floor;
	string   m_wall;
	string   m_door;
};

//用构建者模型来进行管理 不同的模型可以用继承的方式管理
//构造一个产品对象，并设定不同的模块构造接口
class Builder
{
public:
	virtual void makeFloor() = 0;
	virtual void makeWall() = 0;
	virtual void makeDoor() = 0;
	virtual House *GetHouse() = 0;
	virtual ~Builder() {}
};

//不同的子类 实现不同的产品模块定制  ==》建筑师
class VillaBuild : public Builder//别墅
{
public:
	VillaBuild(){
		pHouse = new House;
	}

	~VillaBuild()
	{
		if(pHouse != nullptr)
		{
			delete pHouse;
			pHouse = nullptr;
		}
	}

	virtual void makeFloor(){
		pHouse->setFloor("villa floor");
	}

	virtual void makeWall(){
		pHouse->setWall("villa Wall");
	}

	virtual void makeDoor(){
		pHouse->setDoor("villa Door");
	}

	virtual House *GetHouse(){
		return pHouse; 
	}
private:
	House *pHouse;
};

class FlatBuild : public Builder//公寓
{
public:
	FlatBuild(){
		pHouse = new House; 
	}
	~FlatBuild()
	{
		if(pHouse != nullptr)
		{
			delete pHouse;
			pHouse = nullptr;
		}
	}

	virtual void makeFloor(){
		pHouse->setFloor("flat Door");
	}

	virtual void makeWall(){
		pHouse->setWall("flat Wall");
	}

	virtual void makeDoor(){
		pHouse->setDoor("flat Door");
	}

	virtual House *GetHouse(){
		return pHouse;
	}
private:
	House *pHouse;
};

//如果子类过多，产品模型过多，可以用同一个接口对其进行管理  ==》设计者
//通过具体的子类对象  构建不同的建筑类型
class Director
{
public://这里使用的对象注入的功能，将你建造的对象，通过指针传递过来，然后使用指针进行操作
	void Construct(Builder *builder)
	{
		builder->makeFloor();
		builder->makeWall();
		builder->makeDoor();
	}
};


int main()
{
	//设计师提供建筑模板类 可以自己定制不用这个类
	Director * director = new Director();
	//根据需要定制要创建的目标建筑 建筑师创建
	Builder * builder = new VillaBuild();
	//直接通过建筑模板（类似工厂方法进行创建）
	director->Construct(builder);

	//获取建造结果 进行查看
	House * house = builder->GetHouse();
	cout<<"VillaBuild floor:"<<house->getFloor()<<endl;
	cout<<"VillaBuild wall:"<<house->getWall()<<endl;
	cout<<"VillaBuild door:"<<house->getDoor()<<endl;
	if(builder != nullptr)
	{
		delete builder;
		builder = nullptr;
	}

	//创建另一种类型的建筑
	builder = new FlatBuild();
	//通过模板进行构建 ==》设计师
	director->Construct(builder);
	house = builder->GetHouse();
	cout<<"FlatBuild floor:"<<house->getFloor()<<endl;
	cout<<"FlatBuild wall:"<<house->getWall()<<endl;
	cout<<"FlatBuild door:"<<house->getDoor()<<endl;
	if(builder != nullptr)
	{
		delete builder;
		builder = nullptr;
	}

	if(director != nullptr)
	{
		delete director;
		director = nullptr;
	}

	return 0;
}