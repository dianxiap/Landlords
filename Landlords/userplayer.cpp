#include "userplayer.h"

UserPlayer::UserPlayer(QObject *parent)
    : Player{parent}
{
    m_type=Player::User;
}

void UserPlayer::prepareCallLord()
{

}

void UserPlayer::preparePlayHand()
{
    // 需要告诉主窗口玩家开始出牌了，然后主窗口再开始倒计时
    startCountDown();
}
