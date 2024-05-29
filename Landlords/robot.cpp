#include "robot.h"
#include "strategy.h"
#include "robotgraplord.h"
#include "robotplayhand.h"

Robot::Robot(QObject *parent)
    : Player{parent}
{
    m_type=Player::Robot;
}

void Robot::prepareCallLord()
{
    RobotGrapLord* subThread=new RobotGrapLord(this);
    connect(subThread,&RobotGrapLord::finished,this,[=](){
        subThread->deleteLater();
    });
    subThread->start(); //启动子线程
}

void Robot::preparePlayHand()
{
    RobotPlayHand* subThread=new RobotPlayHand(this);
    connect(subThread,&RobotPlayHand::finished,this,[=](){
        subThread->deleteLater();
    });
    subThread->start();
}

void Robot::thinkCallLord()
{
    /**
     * 基于手中的牌计算权重
     * 大小王：6
     * 顺子/炸弹：5
     * 三张点数相同的牌：4
     * 2的权重：3
     * 对牌：1
    */
    int weigth=0;
    Strategy st(this,m_cards);
    weigth+=st.getRangeCards(Card::Card_SJ,Card::Card_BJ).cardCount()*6;

    QVector<Cards> optSeq=st.pickOptimalSeqSingles();
    weigth+=optSeq.size()*5;

    QVector<Cards> bombs=st.findCardsByCount(4);
    weigth+=bombs.size()*5;

    Cards tmp=m_cards;
    tmp.remove(optSeq);
    tmp.remove(bombs);
    Cards cards2=st.getRangeCards(Card::Card_2,Card::Card_2);
    tmp.remove(cards2);
    QVector<Cards> triples=Strategy(this,tmp).findCardsByCount(3);
    weigth+=triples.size()*4;


    weigth+=m_cards.pointCount(Card::Card_2)*3;

    tmp.remove(triples);
    QVector<Cards> pairs=Strategy(this,tmp).findCardsByCount(2);
    weigth+=pairs.size()*1;

    if(weigth>=22)
    {
        grabLoadBet(3);
    }
    else if(weigth<22&&weigth>=18)
    {
        grabLoadBet(2);
    }
    else if(weigth<18&&weigth>=10)
    {
        grabLoadBet(1);
    }
    else grabLoadBet(0);
}

void Robot::thinkPlayHand()
{
    Strategy st(this,m_cards);
    Cards cs=st.makeStrategy();
    playHand(cs);
}
