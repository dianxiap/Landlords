#include "strategy.h"

Strategy::Strategy(Player *player, const Cards &cards)
{
    m_player=player;
    m_cards=cards;
}

Cards Strategy::makeStrategy()
{
    // 得到出票玩家对象以及打出的牌
    Player* pendPlayer=m_player->getPendPlayer();
    Cards pendCards=m_player->getPendCards();

    // 判读上次出牌的玩家是不是我自己
    if(pendPlayer== m_player||pendPlayer==nullptr)
    {
        // 直接出牌
        // 如果是我自己，出牌没有限制
        return firstPlay();
    }
    else
    {
        // 如果不是我自己，需要找出比出牌玩家点数大的牌
        PlayHand type(pendCards);
        Cards beatCards=getGreaterCards(type);
        // 找到了点数大的牌，考虑是否打出
        bool shouldBeat=whetherToBeat(beatCards);
        if(shouldBeat)
        {
            return beatCards;
        }
        else
        {
            return Cards();
        }
    }
    return Cards();

}

Cards Strategy::firstPlay()
{

}

Cards Strategy::getGreaterCards(PlayHand type)
{

}

bool Strategy::whetherToBeat(Cards &cs)
{
    // 没有找到能够击败对方的牌
    if(cs.isEmpty())
    {
        return false;
    }
    // 得到出牌玩家的对象
    Player* pendPlayer=m_player->getPendPlayer();
    if(m_player->getRole()==pendPlayer->getRole())
    {
        // 当前玩家是农民
        // 手里的牌所剩无几并且是一个完整的牌型
        Cards left=m_cards;
        left.remove(cs);
        if(PlayHand(left).getHandType()!=PlayHand::Hand_Unknown)
        {
            return true;
        }
        // 如果cs对象中的牌的最小点数是2，大小王 ---> 不出牌
        Card::CardPoint basePoint=PlayHand(cs).getCardPoint(); // 得到牌型里的最小点数
        if(basePoint==Card::Card_2||basePoint==Card::Card_SJ||basePoint==Card::Card_BJ)
        {
            return false;
        }

    }
    else
    {
        // 当前玩家是地主
        PlayHand myHand(cs);
        // 如果是三个2带一个或一对，则不出牌（保存实力）
        if((myHand.getHandType()==PlayHand::Hand_Triple_Single||myHand.getHandType()==PlayHand::Hand_Triple_Pair)
                && myHand.getCardPoint()==Card::Card_2)
        {
            return false;
        }
        // 如果cs是对2，并且出牌玩家手中的牌的数量大于等于10&&自己的牌数量大于等于5，则不出牌（保存实力）
        if(myHand.getHandType()==PlayHand::Hand_Pair&& myHand.getCardPoint()==Card::Card_2
            && pendPlayer->getCards().cardCount()>=10&&m_player->getCards().cardCount()>=5)
        {
            return false;
        }
    }
    return true;
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
        return getPlane2SingleOr2Pair(beginPoint,PlayHand::Hand_Pair);
    case PlayHand::Hand_Seq_Pair:
    {
        CardInfo info;
        info.begin=beginPoint;
        info.end=Card::Card_Q;
        info.number=2;
        info.base=3;
        info.extra=extra;
        info.beat=beat;
        info.getSeq=&Strategy::getBaseSeqPair;
        return getSeqPairOrSeqSingle(info);
    }
    case PlayHand::Hand_Seq_Single:
    {
        CardInfo info;
        info.begin=beginPoint;
        info.end=Card::Card_10;
        info.number=1;
        info.base=5;
        info.extra=extra;
        info.beat=beat;
        info.getSeq=&Strategy::getBaseSeqSingle;
        return getSeqPairOrSeqSingle(info);
    }
    case PlayHand::Hand_Bomb:
        return getBomb(beginPoint);
    default:
        return QVector<Cards>();
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

QVector<Cards> Strategy::getPlane2SingleOr2Pair(Card::CardPoint begin, PlayHand::HandType type)
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


QVector<Cards> Strategy::getSeqPairOrSeqSingle(CardInfo &info)
{
    QVector<Cards> findCardArray;
    if(info.beat)
    {
        // 最少3个，最大为A
        for(Card::CardPoint point=info.begin;point<=info.end;point=(Card::CardPoint)(point+1))
        {
            bool found=true;    // 标记是否找到连对
            Cards seqCards;
            // extra表示要找的点数和数量
            for(int i=0;i<info.extra;++i)
            {
                // 基于点数和数量进行牌的搜索
                Cards cards =findSamePointCards((Card::CardPoint)(point+i),info.number);
                if(cards.isEmpty()||(point+info.extra)>=Card::Card_2)
                {
                    found=false;
                    seqCards.clear();
                    break;
                }
                else
                {
                    seqCards<<cards;
                }
            }
            if(found)
            {
                findCardArray<<seqCards;
                return findCardArray;
            }
        }
    }
    else // 表示要搜索的连对对点数和数量无要求
    {
        for(Card::CardPoint point=info.begin;point<=info.end;point=(Card::CardPoint)(point+1))
        {
            // 将找到的基础3连对存储起来
            // 通过成员函数指针 getSeq 调用当前类的成员函数，并传递参数 point,
            // 当于调用 this->某个函数(point)
            Cards baseSeq=(this->*info.getSeq)(point);
            if(baseSeq.isEmpty())
            {
                continue;
            }
            // 再存储到容器中
            findCardArray<<baseSeq;

            int followed=info.base; // 连对的数量
            Cards alreadyFollowedCards; //存储后续找到的满足条件的连对

            while(true)
            {
                // 新的起始点数
                Card::CardPoint followedPoint=Card::CardPoint(point+followed);
                // 判断是否超出上限
                if(followedPoint>=Card::Card_2)
                {
                    break;
                }
                Cards followedCards=findSamePointCards(followedPoint,info.number);
                if(followedCards.isEmpty())
                {
                    break;
                }
                else
                {
                    alreadyFollowedCards<<followedCards;
                    Cards newSeq=baseSeq;
                    newSeq<<alreadyFollowedCards;
                    findCardArray<<newSeq;
                    followed++;
                }
            }
        }
    }
    return findCardArray;
}

QVector<Cards> Strategy::getBomb(Card::CardPoint begin)
{
    QVector<Cards> findCardsArray;
    for(Card::CardPoint point=begin;point<Card::Card_End;point=(Card::CardPoint)(point+1))
    {
        Cards cs=findSamePointCards(point,4);
        if(!cs.isEmpty())
        {
            findCardsArray<<cs;
        }
    }
    return findCardsArray;
}

Cards Strategy::getBaseSeqPair(Card::CardPoint point)
{
    Cards cards0=findSamePointCards(point,2);
    Cards cards1=findSamePointCards((Card::CardPoint)(point+1),2);
    Cards cards2=findSamePointCards((Card::CardPoint)(point+2),2);
    Cards baseSeq;
    if(!cards0.isEmpty()&&!cards1.isEmpty()&&!cards2.isEmpty())
    {
        baseSeq<<cards0<<cards1<<cards2;
    }
    return baseSeq;
}

Cards Strategy::getBaseSeqSingle(Card::CardPoint point)
{
    Cards cards0=findSamePointCards(point,1);
    Cards cards1=findSamePointCards((Card::CardPoint)(point+1),1);
    Cards cards2=findSamePointCards((Card::CardPoint)(point+2),1);
    Cards cards3=findSamePointCards((Card::CardPoint)(point+3),1);
    Cards cards4=findSamePointCards((Card::CardPoint)(point+4),1);
    Cards baseSeq;
    if(!cards0.isEmpty()&&!cards1.isEmpty()&&!cards2.isEmpty()&&!cards3.isEmpty()&&!cards4.isEmpty())
    {
        baseSeq<<cards0<<cards1<<cards2<<cards3<<cards4;
    }
    return baseSeq;
}
