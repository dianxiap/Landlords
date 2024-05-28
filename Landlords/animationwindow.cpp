#include "animationwindow.h"

#include <QPainter>
#include <QTimer>

AnimationWindow::AnimationWindow(QWidget *parent)
    : QWidget{parent}
{
    m_x=0;
}

void AnimationWindow::showBetScore(int bet)
{
    m_x=0;
    if(bet==1)
    {
        m_image.load(":/images/score1.png");
    }
    else if(bet==2)
    {
        m_image.load(":/images/score2.png");
    }
    else if(bet==3)
    {
        m_image.load(":/images/score3.png");
    }
    update(); // 由框架调用paintevent函数

    // 发送一次性的定时信号
    QTimer::singleShot(2000,this,&AnimationWindow::hide);
}

void AnimationWindow::showSequence(Type type)
{
    m_x=0;
    QString name=type==Pair?":/images/liandui.png":":/images/shunzi.png";
    m_image.load(name);
    update();
    // 2s后窗口自动隐藏
    QTimer::singleShot(2000,this,&AnimationWindow::hide);
}

void AnimationWindow::showJokerBomb()
{
    m_index=0;
    m_x=0;
    QTimer* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        m_index++;
        if(m_index>8)
        {
            timer->stop();
            timer->deleteLater();
            m_index=8;
            hide();
        }
        QString name=QString(":/images/joker_bomb_%1.png").arg(m_index);
        m_image.load(name);
        update();
    });
    timer->start(60);

}

void AnimationWindow::showBomb()
{
    m_index=0;
    m_x=0;
    QTimer* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        m_index++;
        if(m_index>12)
        {
            timer->stop();
            timer->deleteLater();
            m_index=12;
            hide();
        }
        QString name=QString(":/images/bomb_%1.png").arg(m_index);
        m_image.load(name);
        update();
    });
    timer->start(60);
}

void AnimationWindow::showPlane()
{
    m_x=width();
    m_image.load(":/images/plane_1.png");
    setFixedHeight(m_image.height());
    update();

    // 把飞机飞过的区域分成5份，每一个区域显示固定的图片
    int step=width()/5;
    QTimer* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        static int dist=0;  // 飞机移动的距离
        static int timers=0;
        dist+=5;
        if(dist>=step)
        {
            // 飞机移动到下一个区域
            dist=0;
            timers++;
            QString name=QString(":/images/plane_%1.png").arg(timers%5+1);
            m_image.load(name);
        }
        if(m_x<=-110)
        {
            // 飞机飞出窗口
            timer->stop();
            timer->deleteLater();
            dist=timers=0;
            hide();
        }
        m_x-=5;
        update();
    });
    timer->start(15);
}

void AnimationWindow::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawPixmap(m_x,0,m_image.width(),m_image.height(),m_image);
}
