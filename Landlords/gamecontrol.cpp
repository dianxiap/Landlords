#include <QRandomGenerator>
#include <QTimer>
#include "gamecontrol.h"

GameControl::GameControl(QObject *parent)
    : QObject{parent}
{}

void GameControl::playerInit()
{
    // 对象实例化
    m_robotLeft=new Robot("机器人A",this);
    m_robotRight=new Robot("机器人B",this);
    m_user=new UserPlayer("我自己",this);

    // 头像的显示
    m_robotLeft->setDirection(Player::Left);
    m_robotLeft->setDirection(Player::Right);
    m_user->setDirection(Player::Right);

    // 性别
    Player::Sex sex;
    sex=(Player::Sex)QRandomGenerator::global()->bounded(2);
    m_robotLeft->setSex(sex);
    sex=(Player::Sex)QRandomGenerator::global()->bounded(2);
    m_robotRight->setSex(sex);
    sex=(Player::Sex)QRandomGenerator::global()->bounded(2);
    m_user->setSex(sex);

    // 出牌顺序
    // user
    m_user->setPrevPlayer(m_robotLeft);
    m_user->setNextPlayer(m_robotRight);
    // left robot
    m_robotLeft->setPrevPlayer(m_robotRight);
    m_robotLeft->setNextPlayer(m_user);
    // right robot
    m_robotRight->setPrevPlayer(m_user);
    m_robotRight->setNextPlayer(m_robotLeft);

    // 指定当前玩家
    m_currPlayer=m_user;

    // 处理玩家发出的信号
    connect(m_user,&UserPlayer::notifyGrabLordBet,this,&GameControl::onGrabBet);
    connect(m_robotLeft,&UserPlayer::notifyGrabLordBet,this,&GameControl::onGrabBet);
    connect(m_robotRight,&UserPlayer::notifyGrabLordBet,this,&GameControl::onGrabBet);
}

Robot *GameControl::getLeftRobot()
{
    return m_robotLeft;
}

Robot *GameControl::getRightRobot()
{
    return m_robotRight;
}

UserPlayer *GameControl::getUserPlayer()
{
    return m_user;
}

void GameControl::setCurrentPlayer(Player *player)
{
    m_currPlayer=player;
}

Player *GameControl::getCurrentPlayer()
{
    return m_currPlayer;
}

Player *GameControl::getPendPlayer()
{
    return m_pendPlayer;
}

Cards GameControl::getPendCards()
{
    return m_pendCards;
}

void GameControl::initAllCards()
{
    m_allCards.clear();
    for(int p=Card::Card_Begin+1;p<Card::Card_SJ;++p)
    {
        for(int s=Card::Suit_Begin+1;s<Card::Suit_End;++s)
        {
            Card c((Card::CardPoint)p,(Card::CardSuit)s);
            m_allCards.add(c);
        }
    }
    m_allCards.add(Card(Card::Card_SJ,Card::Suit_Begin));
    m_allCards.add(Card(Card::Card_BJ,Card::Suit_Begin));
}

Card GameControl::takeOneCard()
{
    return m_allCards.takeRandomCard();
}

Cards GameControl::getSurplusCards()
{
    return m_allCards;
}

void GameControl::resetCardData()
{
    // 洗牌
    initAllCards();
    // 清空所有玩家的牌
    m_robotLeft->clearCards();
    m_robotRight->clearCards();
    m_user->clearCards();
    // 初始化出牌玩家和打出的牌
    m_pendPlayer=nullptr;
    m_pendCards.clear();
}

void GameControl::startLordCard()
{
    m_currPlayer->prepareCallLord();
    playerStatusChanged(m_currPlayer,ThinkingForCallLord);
}

void GameControl::becomeLord(Player *player)
{
    player->setRole(Player::Load);
    player->getPrevPlayer()->setRole(Player::Farmer);
    player->getNextPlayer()->setRole(Player::Farmer);

    // 成为地主的玩家先出牌，因此设置为当前玩家
    m_currPlayer=player;
    player->storeDispatchCard(m_allCards);

    QTimer::singleShot(1000,this,[=]()
    {
        // 游戏状态变化
        gameStatusChanged(PlayeringHand);
        // 玩家状态变化
        playerStatusChanged(player,ThinkingForPlayHand);
        // 准备出牌
        m_currPlayer->preparePlayHand();
    });
}

void GameControl::clearPlayerScore()
{
    m_robotLeft->setScore(0);
    m_robotRight->setScore(0);
    m_user->setScore(0);
}

void GameControl::onGrabBet(Player *player, int bet)
{
    // 1.通知主界面玩家叫地主了（更新信息提示）
    notifyGrabLordBet(player,bet);
    // 2.判断玩家下注是不是3分，如果是抢地主结束
    if(bet==3)
    {
        // 玩家成为地主
        becomeLord(player);
        // 清空数据
        m_betRecord.reset();
        return ;
    }
    // 3.下注不够3分，对玩家的分数进行比较，分数高的是地主
    if(m_betRecord.bet<bet)
    {
        m_betRecord.bet=bet;
        m_betRecord.player=player;
    }
    m_betRecord.times++;
    // 如果每个玩家都抢过一次地主，抢地主结束
    if(m_betRecord.times==3)
    {
        if(m_betRecord.bet=0)
        {
            // 没人抢地主，发射信号告诉主界面重新发牌
            gameStatusChanged(DispatchCard);
        }
        else
        {
            // 让分数高的成为地主
            becomeLord(m_betRecord.player);
        }
        m_betRecord.reset();
        return ;
    }
    // 4.切换玩家，通知下一个玩家继续抢地主
    m_currPlayer=player->getNextPlayer();
    // 发送信号给主界面，告知当前状态仍为抢地主
    playerStatusChanged(m_currPlayer,ThinkingForCallLord);
    // 告诉当前玩家继续抢地主
    m_currPlayer->prepareCallLord();
}
