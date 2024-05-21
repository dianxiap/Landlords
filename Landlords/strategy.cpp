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
        return getTripleSingleOrPair(beginPoint,PlayHand::Hand_Single);
    case PlayHand::Hand_Triple_Pair:
        return getTripleSingleOrPair(beginPoint,PlayHand::Hand_Pair);
    case PlayHand::Hand_Plane:
        return getPlane(beginPoint);
    case PlayHand::Hand_Plane_Two_Single:
        return getPlane2SingleOr2Pair(beginPoint,PlayHand::Hand_Single);
    case PlayHand::Hand_Plane_Two_Pair:
        return getPlane2SingleOr2Pair(beginPoint,PlayHand::Hand_pair);
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

QVector<Cards> Strategy::getTripleSingleOrPair(Card::CardPoint begin, PlayHand::HandType type)
{
    // 找到点数相同的三张牌
    QVector<Cards> findCardArray=getCards(begin,3);
    if(!findCardArray.isEmpty())
    {
        // 将找到的牌从用户手中删除
        Cards remainCards=m_cards;
        remainCards.remove(findCardArray);
        // 搜索牌型
        Strategy st(m_player,remainCards);
        QVector<Cards> cardsArray=st.findCardType(PlayHand(type,Card::Card_Begin,0),false);
        if(!cardsArray.isEmpty())
        {
            // 将找到的牌和三张点数相同的牌进行组合
            for(int i=0;i<findCardArray.size();++i)
            {
                findCardArray[i].add(cardsArray.at(i));
            }
        }
        else
        {
            findCardArray.clear();
        }
    }

    return findCardArray;
}

QVector<Cards> Strategy::getPlane(Card::CardPoint begin)
{
    QVector<Cards> findCardArray;
    for(Card::CardPoint point=begin;point<=Card::Card_K;point=(Card::CardPoint)(point+1))
    {
        // 根据点数和数量搜索
        Cards prevCards=findSamePointCards(point,3);
        Cards nextCards=findSamePointCards((Card::CardPoint)(point+1),3);
        if(!prevCards.isEmpty()&&!nextCards.isEmpty())
        {
            Cards tmp;
            tmp<<prevCards<<nextCards;
            findCardArray<<tmp;
        }
    }
    return findCardArray;
}

QVector<Cards> Strategy::getPlane2SingleOr2Pair(Card::CardPoint begin, PlanHand::HandType type)
{
    // 找飞机
    QVector<Cards> findCardArray=getPlane(begin);
    if(!findCardArray.isEmpty())
    {
        // 将找到的牌从用户手中删除
        Cards remainCards=m_cards;
        remainCards.remove(findCardArray);
        // 搜索牌型
        Strategy st(m_player,remainCards);
        QVector<Cards> cardsArray=st.findCardType(PlayHand(type,Card::Card_Begin,0),false);
        if(cardsArray.size()>=2)
        {
            // 找到了，将其添加到飞机组合中
            for(int i=0;i<findCardArray.size();++i)
            {
                Cards tmp;
                tmp<<cardsArray[0]<<cardsArray[1];
                findCardArray[i].add(tmp);
            }
        }
        else
        {
            findCardArray.clear();
        }
    }
}
