/***************************************************************************
抽象工厂：
    工厂方法只能生产单一的产品
    但实际场景中，一个工厂其实是可以生产多个产品的。
    如果把简单工厂方法相互组合，就属于一个超级工厂。

    抽象工厂，定义一个适用于所有单一产品接口的超级基础类，由子类决定能够生产的能力。
        感觉场景可以是： 不同产品不同型号的组合的场景
相似性：抽象出接口，然后实现此接口
*****************************************************************************/
#include<iostream>
using namespace std;
//产品基类 这里只定义了这一种
class Fruit
{
public:
    virtual void sayname() = 0;
    virtual ~Fruit() {}
};

//超级工厂类 这里可以扩展不同种类的产品 这里针对fruit中的apple和banana不同地域类型做组合
class FruitFactory
{
public:
    virtual Fruit* getApple() = 0;
    virtual Fruit* getBanana() = 0;
    virtual ~FruitFactory() {}
};

//假设 香蕉有南北方之分
class SouthBanana : public Fruit
{
public:
    virtual void sayname()
    {
        cout << "South Banana " << endl;
    }
};

class NorthBanana : public Fruit
{
public:
    virtual void sayname()
    {
        cout << "North Banana " << endl;
    }
};

//假设  苹果有南北方之分
class SouthApple : public Fruit
{
public:
    virtual void sayname()
    {
        cout << "South Apple " << endl;
    }
};

class NorthApple : public Fruit
{
public:
    virtual void sayname()
    {
        cout << "North Apple " << endl;
    }
};

//继承超级工厂，对可以加工不同种类做细节划分
//这里可以对产品做组合处理等业务扩展
class SourthFruitFactory : public FruitFactory
{
public:
    virtual Fruit* getApple()
    {
        return new SouthApple();
    }
    virtual Fruit* getBanana()
    {
        return new SouthBanana();
    }
};

class NorthFruitFactory : public FruitFactory
{
public:
    virtual Fruit* getApple()
    {
        return new NorthApple();
    }
    virtual Fruit* getBanana()
    {
        return new NorthBanana();
    }
};


int main()
{
    FruitFactory * ff = new SourthFruitFactory();
    Fruit *fruit = ff->getApple();
    fruit->sayname();
    delete fruit;
    fruit = ff->getBanana();
    fruit->sayname();
    delete fruit;

    delete ff;

    ff = new NorthFruitFactory();
    fruit = ff->getApple();
    fruit->sayname();
    delete fruit;
    fruit = ff->getBanana();
    fruit->sayname();

    delete fruit;
    delete ff;
    return 0;
}