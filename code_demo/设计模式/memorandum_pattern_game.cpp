#include <iostream>
using namespace std;

//备忘录的类 保存一些必要的中间状态 如游戏人物的临时状态
class RoleStateMemento
{
public:
     RoleStateMemento(unsigned iBlood, unsigned iAttack, unsigned iDefense) : m_iBlood(iBlood), m_iAttack(iAttack), m_iDefense(iDefense){}
 
private:
     friend class GameRole; //定义友元类   可以被这个业务类访问  把相关状态保存下来
 
     unsigned GetBloodValue() { return m_iBlood; }
     unsigned GetAttackValue() { return m_iAttack; }
     unsigned GetDefenseValue() { return m_iDefense; }
 
     unsigned m_iBlood;   // 生命力
     unsigned m_iAttack;  // 攻击力
     unsigned m_iDefense; // 防御力
};

//业务执行类
class GameRole
{
public:
     GameRole() : m_iBlood(100), m_iAttack(100), m_iDefense(100){}
 
     // 存档
     RoleStateMemento *SaveState() { return new RoleStateMemento(m_iBlood, m_iAttack, m_iDefense); }
 
     // 恢复存档
     void RecoveryState(RoleStateMemento *pRoleState)
     {
          m_iBlood = pRoleState->GetBloodValue();
          m_iAttack = pRoleState->GetAttackValue();
          m_iDefense = pRoleState->GetDefenseValue();
          cout<<"Recovery..."<<endl;
     }
 
     void ShowState()
     {
          cout<<"Blood:"<<m_iBlood<<endl;
          cout<<"Attack:"<<m_iAttack<<endl;
          cout<<"Defense:"<<m_iDefense<<endl;
     }
 
     void Fight()
     {
          m_iBlood -= 100;
          m_iAttack -= 10;
          m_iDefense -= 20;
 
          if (m_iBlood == 0)
          {
               cout<<"Game Over"<<endl;
          }
     }
 
private:
     unsigned m_iBlood;   // 生命力
     unsigned m_iAttack;  // 攻击力
     unsigned m_iDefense; // 防御力
};
 
class RoleStateCaretaker
{
public:
     void SetRoleStateMemento(RoleStateMemento *pRoleStateMemento) { m_pRoleStateMemento = pRoleStateMemento; }
     RoleStateMemento *GetRoleStateMemento() { return m_pRoleStateMemento; }
 
private:
     RoleStateMemento *m_pRoleStateMemento;
};
 
int main()
{
     GameRole *pLiXY = new GameRole(); // 创建李逍遥这个角色
     pLiXY->ShowState(); // 显示初始的状态
 
     // 存档
     RoleStateCaretaker *pRoleStateCaretaker = new RoleStateCaretaker();
     pRoleStateCaretaker->SetRoleStateMemento(pLiXY->SaveState()); //保存了状态
 
     // 开始打大BOSS
     pLiXY->Fight();
     pLiXY->ShowState();
 
     // 读档，从新开始
     pLiXY->RecoveryState(pRoleStateCaretaker->GetRoleStateMemento());//恢复状态
     pLiXY->ShowState();
     
     if(pLiXY != nullptr)
     {
          delete pLiXY;
          pLiXY = nullptr;
     }

     if(pRoleStateCaretaker != nullptr)
     {
          delete pRoleStateCaretaker;
          pRoleStateCaretaker = nullptr;
     }

     return 0;
}