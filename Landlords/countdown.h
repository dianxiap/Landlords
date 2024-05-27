#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QWidget>

class CountDown : public QWidget
{
    Q_OBJECT
public:
    explicit CountDown(QWidget *parent = nullptr);

signals:
};

#endif // COUNTDOWN_H
