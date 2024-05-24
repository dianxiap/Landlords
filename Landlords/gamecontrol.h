#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QObject>
#include "userplayer.h"
#include "robot.h"
// #include "card.h"

// 记录玩家下注的相关信息
struct BetRecord
{
    BetRecord()
    {
        reset();
    }
    void reset()
    {
        player=nullptr;
        bet=0;
        times=0;
    }
    // 下注的玩家对象
    Player* player;
    // 玩家下注的分数
    int bet;
    // 第几次叫地主
    int times;
};

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
    void becomeLord(Player* player,int bet);
    // 清空所有玩家的得分
    void clearPlayerScore();
    // 得到玩家下注的最高分数
    int getPlayerMaxBet();

    // 处理叫地主
    void onGrabBet(Player*player, int bet);

    // 处理出牌
    void onPlayHand(Player* player,Cards& cards);

signals:
    // 通知主窗口玩家状态发生变化
    void playerStatusChanged(Player* player,PlayerStatus status);
    // 通知主界面玩家抢地主了
    void notifyGrabLordBet(Player* player,int bet,bool flag);
    // 游戏状态变化
    void gameStatusChanged(GameStatus status);
    // 通知主界面玩家抢地主了
    void notifyPlayHand(Player* player,Cards& cards);
    // 给其他玩家传递出牌数据
    void pendingInof(Player* player,Cards& cards);

private:
    Robot* m_robotLeft;   // 机器人玩家
    Robot* m_robotRight;
    UserPlayer* m_user;   // 用户玩家
    Player* m_currPlayer; // 指向当前玩家
    Player* m_pendPlayer;  // 出牌的玩家
    Cards m_pendCards;     // 出牌的玩家打出的牌
    Cards m_allCards;
    BetRecord m_betRecord;
    int m_currBet;
};

#endif // GAMECONTROL_H
