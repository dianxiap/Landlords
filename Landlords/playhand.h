#ifndef PLAYHAND_H
#define PLAYHAND_H

#include <Cards.h>


// 游戏出牌类
class PlayHand
{
public:
    PlayHand();

    // 出牌组合或者方式
    enum HandType
    {
        Hand_Unknown,           // 未知
        Hand_Pass,              // 过

        Hand_Single,            // 单
        Hand_Pair,              // 对

        Hand_Triple,            // 三个
        Hand_Triple_Single,     // 三带一
        Hand_Triple_Pair,       // 三代二

        Hand_Plane,             // 飞机
        Hand_Plane_Two_Single,  // 飞机带两张单排
        Hand_Plane_Two_Pair,    // 飞机带两对

        Hand_Seq_Pair,          // 连队
        Hand_Seq_Single,        // 顺子

        Hand_Bomb,              // 炸弹
        Hand_Bomb_Single,       // 炸弹带单排
        Hand_Bomb_Pair,         // 炸弹带一对
        Hand_Bomb_Two_Single,   // 炸弹带两张单排

        Hand_Bomb_Jokers,       // 王炸
        Hand_Bomb_Jokers_Single,// 王炸带一个
        Hand_Bomb_Jokers_Pair,  // 王炸带一对
        Hand_Bomb_Jokers_Two_Single,   // 王炸带两张单排

    };

    // 传递给类一组牌，通过类分析出牌型，点数，以及相关的附属信息（比如顺子：记录牌的数量）
    PlayHand(Cards& cards);
    PlayHand(HandType type,Card::CardPoint pt,int extra);

    // 得到牌的属性信息
    PlayHand::HandType getHandType();
    Card::CardPoint getCardPoint();
    int getExtra();

    // 比较自己的牌和去他人的牌型大小
    bool canBeeat(const PlayHand& other);

private:
    // 1.对扑克牌进行分类：1张的，2张，3张，4张的有多少种
    void classify(Cards& cards);
    // 2.对牌型进行分类
    void judgeCardType();

    // 判断牌的类型
    bool isPass();              // 空
    bool isSingle();            // 单
    bool isPair();              // 双
    bool isTriple();            // 三
    bool isTripleSingle();      // 三第一
    bool isTriplePair();        // 三代二
    bool isPlane();             // 飞机
    bool isPlaneTwoSingle();    // 飞机带两单
    bool isPlaneTwoPair();      // 飞机带两队
    bool isSeqSingle();         // 顺子
    bool isSeqPair();           // 联队
    bool isBomb();              // 炸弹
    bool isBombSingle();        // 炸弹带一
    bool isBombPair();          // 炸弹带一队
    bool isBombTwoSingle();     // 炸弹带两单
    bool isBombJokers();        // 王炸
    bool isBombJokersSingle();  // 王炸带一单
    bool isBombJokersPair();    // 王炸带一队
    bool isBombJokersTwoSingle();  // 王炸带两单




private:
    HandType m_type;
    Card::CardPoint m_pt;
    int m_extra;
    QVector<Card::CardPoint> m_oneCard;
    QVector<Card::CardPoint> m_twoCard;
    QVector<Card::CardPoint> m_threeCard;
    QVector<Card::CardPoint> m_fourCard;

};

#endif // PLAYHAND_H
