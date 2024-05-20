#ifndef STRATEGY_H
#define STRATEGY_H

#include "player.h"
#include "playhand.h"

// 机器人出牌的策略
class Strategy
{
public:
    Strategy(Player* player,const Cards& cards);

    // 1.制定出牌策略
    // 2.第一个出牌 firstplayer
    // 3.得到比指定牌型大的牌
    // 4.能大过指定的牌时，判断是出牌还是放行，返回true->出牌，返回false->放行

    // 5.找出指定数量（count)的形同点数的牌（point)，找出count张点数为point的牌
    Cards findSamePointCards(Card::CardPoint point,int count);
    // 6.找出所有点数数量为count的牌 ==>得到一个多张扑克牌数组
    QVector<Cards> findCardsByCount(int count);
    // 7.根据点数范围找牌
    Cards getRangeCards(Card::CardPoint begin,Card::CardPoint end);
    // 8.按牌型找牌，并且指定要找的牌是否要大过指定的牌型
    QVector<Cards> findCardType(PlayHand hand,bool beat);

private:
    QVector<Cards> getCards(Card::CardPoint point,int number);

private:
    Player* m_player;
    Cards m_cards;
};

#endif // STRATEGY_H
