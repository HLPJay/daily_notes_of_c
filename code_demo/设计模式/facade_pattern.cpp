#include <iostream>
using namespace std;

/*
	构建一个大的系统时，如果子模块过多，就会需要外观模式。
	用一个总的接口类，管理各个子模块，统筹调用
*/

//参考 https://www.cnblogs.com/ring1992/p/9593112.html
//子系统中的一组接口，就好比语法分析，生成中间代码，生成汇编代码，链接成可执行程序或库.
//外观模式定义的一个高层接口，如提供一个接口Build按钮，通过这样的一个Build按钮，让编译器更加容易使用

// 语法分析子系统
class CSyntaxParser
{
public:
     void SyntaxParser()
     {
          cout<<"Syntax Parser"<<endl;
     }
};

// 生成中间代码子系统
class CGenMidCode
{
public:
     void GenMidCode()
     {
          cout<<"Generate middle code"<<endl;
     }
};

// 生成汇编代码子系统
class CGenAssemblyCode
{
public:
     void GenAssemblyCode()
     {
          cout<<"Generate assembly code"<<endl;
     }
};

// 链接生成可执行应用程序或库子系统
class CLinkSystem
{
public:
     void LinkSystem()
     {
          cout<<"Link System"<<endl;
     }
};

//通过外观模式，实现各个模块的统一调用，管理整个流程
class Facade
{
public:
     void Compile()
     {
          CSyntaxParser syntaxParser;
          CGenMidCode genMidCode;
          CGenAssemblyCode genAssemblyCode;
          CLinkSystem linkSystem;
          syntaxParser.SyntaxParser();
          genMidCode.GenMidCode();
          genAssemblyCode.GenAssemblyCode();
          linkSystem.LinkSystem();
     }
};

// 客户端
int main()
{
     Facade facade;
     facade.Compile();
     return 0;
}