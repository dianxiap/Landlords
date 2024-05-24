#include <QRandomGenerator>
#include <QTimer>
#include "gamecontrol.h"
#include "playhand.h"

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

    // 传递出牌玩家对象和玩家打出的牌
    connect(this,&GameControl::pendingInof,m_robotLeft,&Robot::storePendingInfo);
    connect(this,&GameControl::pendingInof,m_robotRight,&Robot::storePendingInfo);
    connect(this,&GameControl::pendingInof,m_user,&Robot::storePendingInfo);

    // 处理玩家出牌
    connect(m_robotLeft,&Robot::notifyPlayHand,this,&GameControl::onPlayHand);
    connect(m_robotRight,&Robot::notifyPlayHand,this,&GameControl::onPlayHand);
    connect(m_user,&Robot::notifyPlayHand,this,&GameControl::onPlayHand);
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

void GameControl::becomeLord(Player *player, int bet)
{
    m_currBet=bet;
    player->setRole(Player::Load);
    player->getPrevPlayer()->setRole(Player::Farmer);
    player->getNextPlayer()->setRole(Player::Farmer);

    // 成为地主的玩家先出牌，因此设置为当前玩家
    m_currPlayer=player;
    player->storeDispatchCard(m_allCards);

    QTimer::singleShot(1000,this,[=]()
    {
        // 游戏状态变化
        gameStatusChanged(PlayingHand);
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

int GameControl::getPlayerMaxBet()
{
    return m_betRecord.bet;
}

void GameControl::onGrabBet(Player *player, int bet)
{
    // 1.通知主界面玩家叫地主了（更新信息提示）
    // bet==0：玩家放弃叫地主，m_betRecord.bet>=bet：无效分数
    if(bet==0||m_betRecord.bet>=bet)
    {
        notifyGrabLordBet(player,0,false);
    }
    else if(bet>0&&m_betRecord.bet==9)
    {
        // 第一个qi抢地主的玩家
        notifyGrabLordBet(player,bet,true);
    }
    else
    {
        // 第2，3个抢地主的玩家
        notifyGrabLordBet(player,bet,false);
    }

    // 2.判断玩家下注是不是3分，如果是抢地主结束
    if(bet==3)
    {
        // 玩家成为地主
        becomeLord(player,bet);
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
        if(m_betRecord.bet==0)
        {
            // 没人抢地主，发射信号告诉主界面重新发牌
            gameStatusChanged(DispatchCard);
        }
        else
        {
            // 让分数高的成为地主
            becomeLord(m_betRecord.player,m_betRecord.bet);
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

void GameControl::onPlayHand(Player *player, Cards &cards)
{
    // 1.将玩家出牌的信号转发给主界面
    notifyPlayHand(player,cards);
    // 2.如果不是空牌，给其他玩家发送信号，保存出牌玩家对象和打出的牌
    if(!cards.isEmpty())
    {
        m_pendPlayer=player;
        m_pendCards=cards;
        pendingInof(player,cards); // 给其他玩家发送出牌信息
    }
    // 如果有炸弹，底分翻倍
    PlayHand::HandType type=PlayHand(cards).getHandType();
    if(type==PlayHand::Hand_Bomb||type==PlayHand::Hand_Bomb_Jokers)
    {
        m_currBet*=2;
    }
    // 3.如果玩家的牌出完了，计算本局游戏的总得分
    if(player->getCards().isEmpty())
    {
        Player* prev=player->getPrevPlayer();
        Player* next=player->getNextPlayer();
        if(player->getRole()==Player::Load)
        {
            player->setScore(player->getScore()+2*m_currBet);
            prev->setScore(prev->getScore()-m_currBet);
            next->setScore(next->getScore()-m_currBet);
            player->setWin(true);
            prev->setWin(false);
            next->setWin(false);
        }
        else
        {
            player->setWin(true);
            player->setScore(player->getScore()+m_currBet);
            if(prev->getRole()==Player::Load)
            {
                prev->setScore(prev->getScore()-2*m_currBet);
                next->setScore(next->getScore()+m_currBet);
                prev->setWin(false);
                next->setWin(true);
            }
            else
            {
                next->setScore(next->getScore()-2*m_currBet);
                prev->setScore(prev->getScore()+m_currBet);
                next->setWin(false);
                prev->setWin(true);
            }
        }
        // 赢了：通知主窗口发送玩家状态改变的信号
        playerStatusChanged(player,GameControl::Winning);
        return ;
    }
    // 4.派没有出完，通知下一个玩家继续出牌
    m_currPlayer=player->getNextPlayer();
    m_currPlayer->preparePlayHand();
    playerStatusChanged(m_currPlayer,GameControl::ThinkingForPlayHand);
}
