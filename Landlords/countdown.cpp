#include "countdown.h"

#include <QPainter>

CountDown::CountDown(QWidget *parent)
    : QWidget{parent}
{
    setFixedSize(70,70);
    m_timer=new QTimer(this);
    // 时间到了就发射信号
    connect(m_timer,&QTimer::timeout,this,[=]()
    {
        m_count--;
        if(m_count<10&&m_count>0)
        {
            m_clock.load(":/images/clock.png");
            // 扣出具体的数字,scaled：缩放
            m_number=QPixmap(":/images/number.png").copy(m_count*(30+10),0,30,42).scaled(20,30);
            // 倒计时5需要告诉主界面提示音
            if(m_count==5)
            {
                notMuchTime();
            }

        }
        else if(m_count<=0)
        {
            m_clock=QPixmap();
            m_number=QPixmap();
            m_timer->stop();
            // 通知主界面时间到了
            timeout();
        }
        // 强制当前窗口进行刷新
        update();
    });
}

void CountDown::showCountDown()
{
    m_count=15;
    m_timer->start(1000);

}

void CountDown::stopCOuntDown()
{
    m_timer->stop();
    m_clock=QPixmap();
    m_number=QPixmap();
    update();
}

void CountDown::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawPixmap(rect(),m_clock);
    p.drawPixmap(24,24,m_number.width(),m_number.height(),m_number);

}
