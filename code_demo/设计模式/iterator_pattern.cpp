//1:实现集合中单个元素的类
//2：实现迭代器的类，具体迭代器的类可以操作对应的集合类，用到集合类中的相关遍历方法
//如何实现迭代器中可以用到集合类对象的接口，需要传递
//3：实现集合的类，包含创建迭代器接口。 


//简单的一个集合类，然后创建一个迭代器类，通过迭代器类控制集合的遍历访问集合中元素。
#include <iostream>
#include <vector>
using namespace std;

class TestIterator;
class TestVector{
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
	TestIterator * CreateTestIterator()
	{
		if(m_itr == nullptr)
		{
			m_itr = new TestIterator(this);
		}
		return m_itr;
	}

private:
	vector<int> m_testvector;
	TestIterator * m_itr; //这里为了对应释放
	int count;
};

//通过迭代器类调用对应的方法访问集合元素 这里用下标控制访问
class TestIterator
{
public:
	//要操作的集合的类和集合下标
	TestIterator(TestVector * ts):m_ts(ts), curr(0)
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
	TestVector* m_ts;
	int curr;
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
	TestIterator *itr = test->CreateTestIterator();
	int num = 0;
	//这里的元素是一个int，所以直接打印，可以有其他类型
	while(itr->HasNext())
	{
		cout<<" num "<<num <<"is "<<itr->Next()<<endl;
		num ++;
	}

	for(int i=0;i<test->getCount(); i++)
	{
		cout<<" i "<<i<<" is" <<test->getData(i)<<endl;
	}

	if(test != nullptr)
	{
		delete test;
		test= nullptr;
	}
	return 0;
}
