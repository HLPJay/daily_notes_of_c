/*********************************************************
享元模型： 
	目的时避免大量非常类似的开销。
	如果需要大量相似对象表示数据，只有很少的差异，可以经过一些处理，
把很少差异的参数移动到外部，用传参的方式调用，减少实例化的对象数。

实现demo:
	参考网上实例，如下五子棋，红黑节点的双方一直操作的对象，需要一直操作，可以用一个对象管理。（一个房间管理两个棋手，重复利用对象）
	更加拓展，如棋牌游戏中，如果多个房间，多人下棋，
*********************************************************/

#include <iostream>
#include <map>
#include <vector>
using namespace std;

//管理坐标
typedef struct pointTag
{
    int x;
    int y;

    pointTag(){}
    pointTag(int a, int b)
    {
        x = a;
    	y = b;
    }

    bool operator <(const pointTag& other) const
    {
        if (x < other.x)
        {
            return true;
        }
        else if (x == other.x)
        {
            return y < other.y;
        }

        return false;
    }
}POINT;

//管理红黑子
typedef enum PieceColorTag
{
    BLACK,
    WHITE
}PIECECOLOR;

//把棋子的颜色和坐标作为一个对象进行构造
class CPiece
{
public:
    CPiece(PIECECOLOR color) : m_color(color){}
    PIECECOLOR GetColor() { return m_color; }

    void SetPoint(POINT point) { m_point = point; } //拷贝构造
    POINT GetPoint() { return m_point; }

protected:
    PIECECOLOR m_color;
    POINT m_point;
};
//管理棋子
class CGomoku : public CPiece
{
public:
    CGomoku(PIECECOLOR color) : CPiece(color){}
};

//根据黑白棋 定义一个对象 重复使用 ==》根据入参，可以决定不同的选手 
class CPieceFactory
{
public:
    CPiece *GetPiece(PIECECOLOR color)
    {
        CPiece *pPiece = NULL;
	    if (m_vecPiece.empty())
	    {
			pPiece = new CGomoku(color);
			m_vecPiece.push_back(pPiece);
	    }
	    else
	    {
	        for (vector<CPiece *>::iterator it = m_vecPiece.begin(); it != m_vecPiece.end(); ++it)
	        {
	            if ((*it)->GetColor() == color)
		        {
		            pPiece = *it;
		            break;
		        }
	        }
	        if (pPiece == NULL)
	        {
		        pPiece = new CGomoku(color);
		        m_vecPiece.push_back(pPiece);
	        }
	    }
        return pPiece;
    }

    ~CPieceFactory()
    {
        for (vector<CPiece *>::iterator it = m_vecPiece.begin(); it != m_vecPiece.end(); ++it)
        {
            if (*it != NULL)
	        {
		        delete *it;
		        *it = NULL;
	        }
  		}
    }

private:
    vector<CPiece *> m_vecPiece;
};

//棋盘管理
class CChessboard
{
public:
	//落子 节点坐标对应 共享的一个节点对象
    void Draw(CPiece *piece)
    {
	    if (piece->GetColor())
	    {
	        cout<<"Draw a White"<<" at ("<<piece->GetPoint().x<<","<<piece->GetPoint().y<<")"<<endl;
	    }
	    else
	    {
	    	cout<<"Draw a Black"<<" at ("<<piece->GetPoint().x<<","<<piece->GetPoint().y<<")"<<endl;
	    }
	    m_mapPieces.insert(pair<POINT, CPiece *>(piece->GetPoint(), piece));
    }

    //显示棋盘上所有的节点
    void ShowAllPieces()
    {
        for (map<POINT, CPiece *>::iterator it = m_mapPieces.begin(); it != m_mapPieces.end(); ++it)
    	{
            if (it->second->GetColor())
	        {
	        	cout<<"("<<it->first.x<<","<<it->first.y<<") has a White chese."<<endl;
	        }
	        else
	        {
	       		cout<<"("<<it->first.x<<","<<it->first.y<<") has a Black chese."<<endl;
	        }
    	}
    }

private:
    map<POINT, CPiece *> m_mapPieces;
};

int main()
{
	//创建一个棋盘管理房间对象  调用方法根据传参选择对手，重复利用两个对象
    CPieceFactory *pPieceFactory = new CPieceFactory();
    //创建一个棋盘对象 
    CChessboard *pCheseboard = new CChessboard();

 	//先手第一个选手 选择白棋 定义坐标并落子
    CPiece *pPiece = pPieceFactory->GetPiece(WHITE); //选定对象
    pPiece->SetPoint(POINT(2, 3));
    pCheseboard->Draw(pPiece);

    //第二个选手  根据颜色找到自己的棋子进行落子
    pPiece = pPieceFactory->GetPiece(BLACK);
    pPiece->SetPoint(POINT(4, 5));
    pCheseboard->Draw(pPiece);

    //第一个选手落子
    pPiece = pPieceFactory->GetPiece(WHITE);
    pPiece->SetPoint(POINT(2, 4));
    pCheseboard->Draw(pPiece);

    //第二个选手落子
    pPiece = pPieceFactory->GetPiece(BLACK);
    pPiece->SetPoint(POINT(3, 5));
    pCheseboard->Draw(pPiece);

    //展示棋盘上所有的落子
    cout<<"Show all cheses"<<endl;
    pCheseboard->ShowAllPieces();

    //删除棋盘
    if (pCheseboard != NULL)
    {
    	delete pCheseboard;
    	pCheseboard = NULL;
    }
    //删除房间
    if (pPieceFactory != NULL)
    {
    	delete pPieceFactory;
    	pPieceFactory = NULL;
    }
    return 0;
}