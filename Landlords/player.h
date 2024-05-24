#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "cards.h"

class Player : public QObject
{
    Q_OBJECT
public:
    enum Role{Load,Farmer};         // 角色
    enum Sex{Man,Woman};            // 性别
    enum Direction{Left,Right};     // 头像的显示方向
    enum Type{Robot,User,UnKnow};   // 玩家的类型
    explicit Player(QObject *parent = nullptr);
    explicit Player(QString name,QObject *parent = nullptr);

    // 名字
    void setName(QString name);
    QString getName();

    // 角色
    void setRole(Role role);
    Role getRole();

    // 性别
    void setSex(Sex sex);
    Sex getSex();

    // 头像的显示方位
    void setDirection(Direction direction);
    Direction getDirection();

    // 玩家的类型
    void setType(Type type);
    Type getType();

    // 玩家的分数
    void setScore(int score);
    int getScore();

    // 玩家的输赢
    void setWin(bool flag);
    bool isWin();

    // 提供当前玩家的上家/下家对象
    void setPrevPlayer(Player* player);
    void setNextPlayer(Player* player);
    Player* getPrevPlayer();
    Player* getNextPlayer();

    // 叫地主/抢地主
    void grabLoadBet(int point);

    // 存储扑克牌（发牌的时候得到的）
    void storeDispatchCard(Card& card);
    void storeDispatchCard(Cards& cards);

    // 得到所有的牌
    Cards getCards();

    // 清空玩家手中的牌
    void clearCards();

    // 出牌
    void playHand(Cards& cards);

    // 设置出牌的玩家以及打出的扑克牌
    void setPendingInfo(Player* player,Cards& cards);
    Player* getPendPlayer();
    Cards getPendCards();

    // 存储出牌玩家对象和打出的牌
    void storePendingInfo(Player* player,Cards& cards);

    // 虚函数
    virtual void prepareCallLord();
    virtual void preparePlayHand();
    virtual void thinkCallLord();
    virtual void thinkPlayHand();

signals:
    // 通知已经掉地主下注
    void notifyGrabLordBet(Player* player,int bet);
    // 通知已经出牌
    void notifyPlayHand(Player* player,Cards& cards);
    // 通知已经发牌了
    void notifyPickCards(Player* player,Cards& cards);

protected:
    int m_score;
    QString m_name;
    Role m_role;
    Sex m_sex;
    Direction m_direction;
    Type m_type;
    bool m_isWin;
    Player* m_prev;
    Player* m_next;
    Cards m_cards;         // 存储多张扑克牌（玩家手中的牌）
    Cards m_pendCards;     // 打出的待处理的扑克牌
    Player* m_pendPlayer;  // 打出的扑克牌的所有者
};

#endif // PLAYER_H
