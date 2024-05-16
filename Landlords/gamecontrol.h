#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QObject>
#include "userplayer.h"
#include "robot.h"
// #include "card.h"

class GameControl : public QObject
{
    Q_OBJECT
public:
    // 游戏状态
    enum GameStatus
    {
        DispatchCard,
        CallingLord,
        PlayingHand
    };
    // 玩家状态
    enum PlayerStatus
    {
        ThinkingForCallLord,
        ThinkingForPlayHand,
        Winning
    };

    explicit GameControl(QObject *parent = nullptr);

    // 玩家
    // 初始化 和 得到玩家的实例对象
    void playerInit();
    Robot* getLeftRobot();
    Robot* getRightRobot();
    UserPlayer* getUserPlayer();

    // 设置 和 获取当前玩家
    void setCurrentPlayer(Player* player);
    Player* getCurrentPlayer();

    // 得到出牌玩家和打出的牌
    Player* getPendPlayer();
    Cards getPendCards();

    // 扑克牌
    // 初始化一封扑克牌的数据
    void initAllCards();

    // 每次发一张牌
    Card takeOneCard();

    // 得到最后的三张牌
    Cards getSurplusCards();

    // 重置卡牌数据
    void resetCardData();

    // 准备叫地主
    void startLordCard();
    // 成为地主
    void becomeLord(Player* player);
    // 清空所有玩家的得分
    void clearPlayerScore();
    // 处理叫地主

    // 处理出牌

signals:

private:
    Robot* m_robotLeft;   // 机器人玩家
    Robot* m_robotRight;
    UserPlayer* m_user;   // 用户玩家
    Player* m_currPlayer; // 指向当前玩家
    Player* m_pendPlayer;  // 出牌的玩家
    Cards m_pendCards;     // 出牌的玩家打出的牌
    Cards m_allCards;
};

#endif // GAMECONTROL_H
