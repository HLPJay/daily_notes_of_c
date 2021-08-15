/*********************************************************
工厂方法：
    与简单工厂的区别，将产品的创建分别委托给单独的类，即一个工厂对应一个产片
    而不是将所有产品的创建交给一个类来管理
    1.首先创建工厂
    2.通过此工厂来获取对应的产品（将对象的创建放到工厂中创建）
***********************************************************/
#include <iostream>
#include <string.h>
using namespace std;

//工厂具体产品基类 
class Fruit
{
public:
    virtual void sayname() = 0;
    virtual ~Fruit(){};
};

//工厂对象基类
class FruitFactory
{
public:
    virtual Fruit* getFruit() = 0;
    virtual void destroyFruit() = 0;
    virtual ~FruitFactory() {};
};

//每个产品对应一个产品属性类和工厂创建类
class Banana : public Fruit
{
public:
    virtual void sayname()
    {
        cout << "Banana " << endl;
    }
};

//每一个对象的实例化其实就是一个产品
class BananaFactory : public FruitFactory
{
public:
    virtual Fruit* getFruit()
    {
        _ba = new Banana;
        return _ba;
    }

    virtual void destroyFruit()
    {
        if(_ba != nullptr)
        {
            delete _ba;
            _ba = nullptr;
        } 
    }
public:
    Banana* _ba;
};

//每个产品对应一个产品属性类和工厂创建类
class Apple : public Fruit
{
public:
    virtual void sayname()
    {
        cout << "Apple " << endl;
    }
};

class AppleFactory : public FruitFactory
{
public:
    virtual Fruit* getFruit()
    {
        _ap = new Apple;
        return _ap;
    }

    virtual void destroyFruit()
    {
        if(_ap != nullptr)
        {
            delete _ap;
            _ap = nullptr;
        } 
    }
public:
    Apple * _ap;
};
int main()
{
    FruitFactory * ff = new BananaFactory();
    Fruit *fruit = ff->getFruit();
    fruit->sayname();
    ff->destroyFruit();
    // delete fruit;
    delete ff;

    ff = new AppleFactory();
    fruit = ff->getFruit();
    fruit->sayname();
    ff->destroyFruit();
    // delete fruit;
    delete ff;
    return 0;
}