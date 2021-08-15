/**************************************************************
简单工厂：
    1.产品的创建交给一个单独的类来管理
    2.此类提供全局的访问接口，根据输入条件的不同来创建不同的对象
使用：
    1.创建工厂，根据输入的条件来创建对应的产品
    2.在创建产品时，返回产品的接口指针
    3.通过此指针来操作相应的对象
***************************************************************/
#include <iostream>
#include <string.h>
using namespace std;
class Fruit
{ 
public:
    virtual void getFruit() = 0;
    virtual ~Fruit() {};
};

class Banana : public Fruit
{
public:
    virtual void getFruit()
    {
        cout << "香蕉" << endl;
    }
};

class Pear : public Fruit
{
public:
    virtual void getFruit()
    {
        cout << "梨子" << endl;
    }
};

class Factory
{
public:
    static Fruit* Create(const char *name)
    {
        Fruit *tmp = NULL;
        if (strcmp(name, "pear") == 0)
        {
            tmp = new Pear();
        }
        else if (strcmp(name, "banana") == 0)
        {
            tmp = new Banana();
        }
        else
        {
            return NULL;
        }
        return tmp;
    }
};
int main()
{
    Fruit *pear = Factory::Create("pear");
    if (pear == NULL)
    {
        cout << "创建pear失败\n";
    }
    pear->getFruit();
    Fruit *banana = Factory::Create("banana");
    if (banana == NULL)
    {
        cout << "创建banana失败\n";
    }
    banana->getFruit();

    if(pear != nullptr)
    {
        delete pear;
        pear = nullptr;
    }

    if(banana != nullptr)
    {
        delete banana;
        banana = nullptr;
    }
    return 0;
}