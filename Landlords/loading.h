#ifndef LOADING_H
#define LOADING_H

#include <QWidget>

class Loading : public QWidget
{
    Q_OBJECT
public:
    explicit Loading(QWidget *parent = nullptr);

signals:
protected:
    void paintEvent(QPaintEvent* event);

private:
    QPixmap m_bk;
    QPixmap m_progess;
    int m_dist=1;   // 当前进度条的宽度

};

#endif // LOADING_H
