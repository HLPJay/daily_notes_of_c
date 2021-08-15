#include <iostream>
#include <string.h>
using namespace std;

//适配器的作用，使我们要用的接口满足目标使用接口
//例如：我们的电压通常使220v,但实际产品，有18v,22v等各种不同的电压，我们在用的时候，需要把220v通过适配转成满足的目标产品要求

//定义我们的220v电压接口 
class Current220v{
public:
	//基本的业务接口，如充电，放电等
	void charge(int i) {//相关业务等
		cout<<"use "<<i <<" v !"<<endl; 
	} 
};

//定义一个适配器，把220v的电压适配成不同的电压， 类似变压器的功能
class Adapter : public Current220v
{
	//适配 变成不同的电压接口
public:
	void use18v() {
		Current220v::charge(18);//这里其实使相关的业务，调用基类的业务接口，设置参数，调用基类接口
	}

	void use22v(){
		Current220v::charge(22);
	}

	void use220v(){
		Current220v::charge(220);
	}

	void useOtherV(){
		cout<<"use other v"<<endl;
	}
};

//定义一个基类，供使用的其他产品使用   使用客户提供的统一接口
class baseProduct{
public:
	baseProduct() {
		ad = new Adapter();
	}

	virtual ~baseProduct(){
		if(ad != nullptr)
		{
			delete ad;
			ad = nullptr;
		}
	}
public:
	virtual void use_voltage() = 0;
	Adapter * getAdaptor() { return ad;}
private:
	Adapter * ad;
};


//不同的产品 18v的产品
class useCurrent18v:public baseProduct
{
public:
    void use_voltage() override
    {
        baseProduct::getAdaptor()->use18v();
    }
};

//不同的产品 18v的产品
class useCurrent22v:public baseProduct
{
public:
    void use_voltage() override
    {
        baseProduct::getAdaptor()->use22v();
    }

};

//不同的产品 220v的产品
class useCurrent220v:public baseProduct
{
public:
    void use_voltage() override
    {
        baseProduct::getAdaptor()->use220v();
    }
};

void testuse18v(){
	useCurrent18v * use_18v = new useCurrent18v();
	use_18v->use_voltage();
	delete use_18v;
	use_18v = nullptr;
}

void testuse22v(){
	useCurrent22v * use_22v = new useCurrent22v();
	use_22v->use_voltage();
	delete use_22v;
	use_22v = nullptr;
}

void testuse220v(){
	useCurrent220v * use_220v = new useCurrent220v();
	use_220v->use_voltage();
	delete use_220v;
	use_220v = nullptr;
}

int main()
{
	//当不同的产品使用时 直接定义类，使用适配器统一接口
	testuse18v();
	testuse22v();
	testuse220v();
    return 0;
}