#include "strategy.h"

Strategy::Strategy(Player *player, const Cards &cards)
{
    m_player=player;
    m_cards=cards;
}

Cards Strategy::findSamePointCards(Card::CardPoint point, int count)
{
    if(count<1||count>4)
    {
        return Cards();
    }
    // 大小王
    if(point==Card::Card_SJ||point==Card::Card_BJ)
    {
        if(count>1) return Cards();

        Card card;
        card.setPoint(point);
        card.setSuit(Card::Suit_Begin);
        // 搜索是否存在大小王
        if(m_cards.contains(card))
        {
            Cards cards;
            cards.add(card);
        }
        return Cards();
    }

    // 不是大小王(遍历花色)
    int findCount=0;
    Cards findCards;
    for(int suit=Card::Suit_Begin+1;suit<Card::Suit_End;++suit)
    {
        Card card;
        card.setPoint(point);
        card.setSuit((Card::CardSuit)suit);

        if(m_cards.contains(card))
        {
            findCount++;
            findCards.add(card);
            if(findCount==count)
            {
                return findCards;
            }
        }
    }
    return Cards();
}

QVector<Cards> Strategy::findCardsByCount(int count)
{
    if(count<1||count>4)
    {
        return QVector<Cards>();
    }

    QVector<Cards> cardsArray;
    for(Card::CardPoint point=Card::Card_3;point<Card::Card_End;point=(Card::CardPoint)(point+1))
    {
        // 如果该点数的牌的数量符合指定数量，则加入结果数组
        if(m_cards.pointCount(point)==count)
        {
            Cards cs;
            cs<<findSamePointCards(point,count);
            cardsArray<<cs;
        }
    }
    return cardsArray;
}

Cards Strategy::getRangeCards(Card::CardPoint begin, Card::CardPoint end)
{
    Cards rangeCards;
    for(Card::CardPoint point=begin;point<end;point=(Card::CardPoint)(point+1))
    {
        int count=m_cards.pointCount(point);
        Cards cs=findSamePointCards(point,count);
        rangeCards<<cs;
    }
    return rangeCards;
}

QVector<Cards> Strategy::findCardType(PlayHand hand, bool beat)
{
    PlayHand::HandType type=hand.getHandType();
    Card::CardPoint point=hand.getCardPoint();
    int extra=hand.getExtra();

    // 确定起始点数
    Card::CardPoint beginPoint=beat?Card::CardPoint(point+1):Card::Card_3;

    switch(type)
    {
    case PlayHand::Hand_Single:
        return getCards(beginPoint,1);
    case PlayHand::Hand_Pair:
        return getCards(beginPoint,2);
    case PlayHand::Hand_Triple:
        return getCards(beginPoint,3);
    case PlayHand::Hand_Triple_Single:
        break;
    case PlayHand::Hand_Triple_Pair:
        break;
    case PlayHand::Hand_Plane:
        break;
    case PlayHand::Hand_Plane_Two_Single:
        break;
    case PlayHand::Hand_Plane_Two_Pair:
        break;
    case PlayHand::Hand_Seq_Pair:
        break;
    case PlayHand::Hand_Seq_Single:
        break;
    case PlayHand::Hand_Bomb:
        break;
    default:
        break;
    }
}

QVector<Cards> Strategy::getCards(Card::CardPoint point, int number)
{
    QVector<Cards> findCardsArray;
    for(Card::CardPoint pt=point;pt<Card::Card_End;pt=(Card::CardPoint)(pt+1))
    {
        Cards cs=findSamePointCards(pt,number);
        if(!cs.isEmpty())
        {
            findCardsArray<<cs;
        }
    }
    return findCardsArray;
}
