#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QTimer>
#include <QWidget>

class CountDown : public QWidget
{
    Q_OBJECT
public:
    explicit CountDown(QWidget *parent = nullptr);

    void showCountDown();
    // 玩家在倒计时的时候把牌打出去了，则种植倒计时
    void stopCOuntDown();

signals:
    void notMuchTime();
    void timeout();
protected:
    void paintEvent(QPaintEvent *ev);


private:
    QPixmap m_clock;
    QPixmap m_number;
    QTimer* m_timer;
    int m_count;
};

#endif // COUNTDOWN_H
