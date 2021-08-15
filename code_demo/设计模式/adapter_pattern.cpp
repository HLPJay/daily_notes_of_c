#include <iostream>
#include <string.h>
using namespace std;

//适配器模式
//可以作为升级api，升级版本接口使用
//参考网址：https://blog.csdn.net/yxh_1_/article/details/116085949

//1:可以通过版本类型决定,适配器选择的版本的接口，感觉类似于桥接了
//2:可以通过继承的方式，使用最新的接口

enum VersionType{
    VersionOne,
    VersionTwo,
    VersionThree
};

class old_one_api{
    
    void use_api()
    {
        cout<<" one version api"<<endl;
    }
};

class old_two_api{
    void use_api()
    {
        cout<<" two version api"<<endl;
    }
};

class old_three_api{
    void use_api()
    {
        cout<<" three version api"<<endl;
    }
};


class adapter : public old_one_api, public old_two_api, public old_three_api
{
public:
    void use_version_api_by_type()
    {
        switch(m_type)
        {
            case VersionOne: old_one_api::use_api(); break;
            case VersionTwo: old_two_api::use_api(); break;
            case VersionThree: old_three_api::use_api(); break;
        }
        return;
    }

    void set_version_type(VersionType type)
    {
        m_type = type;
    }
public:
    VersionType m_type;
};

int main()
{
    //通过适配器 分别使用不同的版本
    adapter * ad = new adapter();
    //使用第一个版本：
    ad->set_version_type(VersionOne);
    ad->use_version_api_by_type();
    //使用第儿个版本：
    ad->set_version_type(VersionTwo);
    ad->use_version_api_by_type();
    //使用第三个版本：
    ad->set_version_type(VersionThree);
    ad->use_version_api_by_type();
    delete ad;
    return 0;
}