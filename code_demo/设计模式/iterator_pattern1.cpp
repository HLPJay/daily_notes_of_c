//上面的demo发现编译不通过，是类与类之间相关引用的原因，所以这里需要一个中间类来实现

//迭代器测试类初始化要用到 集合类，这里构造一个集合类的基类，实现迭代器的初始化测试
//集合类要用到迭代器类去做相关初始化 同样需要构造一个迭代器基础类，来供集合类接口调用
#include <iostream>
#include <vector>
using namespace std;

class baseIterator{
public:
	virtual ~baseIterator() = default;
	virtual bool HasNext() = 0;
	virtual int Next() = 0;
	virtual void reset() = 0;
};

//包含必要的集合类的方法，通过传入该对象实现迭代器的调用
class baseTestIterator{
public:
	virtual ~baseTestIterator() = default;
	virtual baseIterator * CreateTestIterator()= 0;
	virtual int getCount() = 0;
	virtual int getData(int d) = 0;
};

class TestIterator: public baseIterator
{
public:
	//要操作的集合的类和集合下标
	TestIterator(baseTestIterator * ts):m_ts(ts), curr(0)
	{}
	~TestIterator() {}

	//判断是否有下一个元素
	bool HasNext()
	{
		if(curr >= m_ts->getCount())
		{
			return false;
		}
		return true;
	}

	//返回的其实是集合类中迭代器下一个元素
	int Next()
	{
		int data = m_ts->getData(curr);
		curr++;
		return data;
	}
	//重置节点的信息
	void reset()
	{
		curr=0;
	}
private:
	baseTestIterator* m_ts;
	int curr;
};

class TestVector: public baseTestIterator{
public:
	//构造和析构
	TestVector():m_itr(nullptr), count(0)
	{}
	~TestVector() {
		if(m_itr != nullptr)
		{
			delete m_itr;
			m_itr = nullptr;
		}
	}
	//类本身的方法  插入元素和获取元素
	void push(int d) {
		m_testvector.push_back(d);
		count++;
	}

	void pop()
	{
		m_testvector.pop_back();
		count--;
	}

	int getCount()
	{
		return count;
	}

	int getData(int d)
	{
		if(d < count)
		{
			return m_testvector[d];
		}
		return -1;
	}
	//这里定义创建迭代器的方法，通过迭代器控制数组元素的访问
	baseIterator * CreateTestIterator()
	{
		if(m_itr == nullptr)
		{
			m_itr = new TestIterator(this);
		}
		return m_itr;
	}

private:
	vector<int> m_testvector;
	baseIterator * m_itr; //这里为了对应释放
	int count;
};

int main()
{
	//创建一个集合类 并塞入相关数据
	TestVector * test = new TestVector();
	test->push(5);
	test->push(8);
	test->push(6);
	test->push(7);
	test->push(3);
	test->push(2);
	cout<<"count:"<<test->getCount()<<endl;
	baseIterator *itr = test->CreateTestIterator();
	int num = 0;
	//这里的元素是一个int，所以直接打印，可以有其他类型
	while(itr->HasNext())
	{
		cout<<" num "<<num <<"is "<<itr->Next()<<endl;
		num ++;
	}

	for(int i=0;i<test->getCount(); i++)
	{
		cout<<" i "<<i<<" is " <<test->getData(i)<<endl;
	}

	if(test != nullptr)
	{
		delete test;
		test= nullptr;
	}
	return 0;
}


//C++中相关集合类数据结构的迭代器实现方式类似，只是增加了模板定义，next的处理用指针地址+1代替 模板机制代替了虚基函数处理引用的问题