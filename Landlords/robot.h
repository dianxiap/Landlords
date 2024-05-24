#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include "player.h"

class Robot : public Player
{
    Q_OBJECT
public:
    // 继承构造函数：可以使用父类中的所有构造函数
    using Player::Player;
    explicit Robot(QObject *parent = nullptr);

    void prepareCallLord() override;
    void preparePlayHand() override;

    // 考虑叫地主的函数
    void thinkCallLord() override;
    void thinkPlayHand() override;
};

#endif // ROBOT_H
