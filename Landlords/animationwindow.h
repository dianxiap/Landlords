#ifndef ANIMATIONWINDOW_H
#define ANIMATIONWINDOW_H

#include <QWidget>

class AnimationWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationWindow(QWidget *parent = nullptr);

    // 显示下注分数
    void showBetScore(int bet);

protected:
    void paintEvent(QPaintEvent* ev);
signals:

private:
    QPixmap m_image;
};

#endif // ANIMATIONWINDOW_H
