#include "animationwindow.h"

#include <QPainter>
#include <QTimer>

AnimationWindow::AnimationWindow(QWidget *parent)
    : QWidget{parent}
{}

void AnimationWindow::showBetScore(int bet)
{
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
    QTimer::singleShot(2000,this,&AnimationWindow::hide());
}

void AnimationWindow::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawPixmap(0,0,m_image.width(),m_image.height(),m_image);
}
