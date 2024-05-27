#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include "endingpanel.h"
#include "gamepanel.h"
#include "playhand.h"
#include "ui_gamepanel.h"

GamePanel::GamePanel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GamePanel)
{
    ui->setupUi(this);

    // 1.背景图
    int num=QRandomGenerator::global()->bounded(10);
    QString path=QString(":/images/background-%1.png").arg(num+1);
    m_bkimage.load(path);

    // 2.窗口的标题和大小
    setWindowTitle("欢乐斗地主");
    setFixedSize(1000,650);

    // 3.实例化游戏控制类对象
    gameControlInit();

    // 4.玩家得分（更新）
    updatePlayerScore();

    // 5.切割游戏图片
    initCardMap();

    // 6.初始化游戏中的按钮组
    initButtonsGroup();

    // 7.初始化玩家在窗口中的上下文环境
    initPlayerContext();

    // 8.扑克牌场景的初始化
    initGameScene();

    // 定时器实例化
    m_timer=new QTimer(this);
    connect(m_timer,&QTimer::timeout,this,&GamePanel::onDispatchCard);

    m_animation=new AnimationWindow(this);
}

GamePanel::~GamePanel()
{
    delete ui;
}

void GamePanel::gameControlInit()
{
    m_gamectl=new GameControl(this);
    m_gamectl->playerInit();
    // 得到玩家的实例对象
    Robot* leftRobot=m_gamectl->getLeftRobot();
    Robot* rightRobot=m_gamectl->getRightRobot();
    UserPlayer* user=m_gamectl->getUserPlayer();
    // 存储的顺序：左侧机器人 右侧机器人 中间玩家
    m_playerList<<leftRobot<<rightRobot<<user;

    // 在主窗口中处理游戏控制类发射的信号
    connect(m_gamectl,&GameControl::playerStatusChanged,this,&GamePanel::onPlayerStatusChanged);
    connect(m_gamectl,&GameControl::notifyGrabLordBet,this,&GamePanel::onGrabLordBet);
    connect(m_gamectl,&GameControl::gameStatusChanged,this,&GamePanel::gameStatusPrecess);
    connect(m_gamectl,&GameControl::notifyPlayHand,this,&GamePanel::onDisposePlayHand);

    connect(leftRobot,&Player::notifyPickCards,this,&GamePanel::disposCard);
    connect(rightRobot,&Player::notifyPickCards,this,&GamePanel::disposCard);
    connect(user,&Player::notifyPickCards,this,&GamePanel::disposCard);
}

void GamePanel::updatePlayerScore()
{
    ui->scorePanel->setScores(
            m_playerList[0]->getScore(),
            m_playerList[1]->getScore(),
            m_playerList[2]->getScore());
}

void GamePanel::initCardMap()
{
    // 1.加载大图
    QPixmap pixmap(":/images/card.png");
    // 2.计算每张图片大小
    m_cardSize.setWidth(pixmap.width()/13);
    m_cardSize.setHeight(pixmap.height()/5);

    // 3.切割
    // 背景图
    m_cardBackImg=pixmap.copy(2*m_cardSize.width(),4*m_cardSize.height(),
                                m_cardSize.width(),m_cardSize.height());
    // 正常图片
    for(int i=0,suit=Card::Suit_Begin+1;suit<Card::Suit_End;++suit,++i)
    {
        for(int j=0,pt=Card::Card_Begin+1;pt<Card::Card_End;++pt,++j)
        {
            Card card((Card::CardPoint)pt,(Card::CardSuit)suit);
            // 裁剪图片
            cropImage(pixmap,j*m_cardSize.width(),i*m_cardSize.height(),card);
        }
    }
    // 大小王
    Card c;
    c.setPoint(Card::Card_SJ);
    c.setSuit(Card::Suit_Begin);
    cropImage(pixmap,0,4*m_cardSize.height(),c);

    c.setPoint(Card::Card_BJ);
    cropImage(pixmap,m_cardSize.width(),4*m_cardSize.height(),c);
}

void GamePanel::cropImage(QPixmap &pix, int x, int y,Card& c)
{
    QPixmap sub=pix.copy(x,y,m_cardSize.width(),m_cardSize.height());
    CardPanel* panel=new CardPanel(this);
    // 将图片设置给窗口
    panel->setImage(sub,m_cardBackImg);
    // 将卡牌数据设置给窗口
    panel->setCard(c);
    panel->hide();
    // 将卡牌数据和卡牌对应的窗口映射保存
    m_cardMap.insert(c,panel);
    connect(panel,&CardPanel::cardSelected,this,&GamePanel::onCardSelected);
}

void GamePanel::initButtonsGroup()
{
    ui->btnGroup->initButtons();
    ui->btnGroup->selectPanel(ButtonGroup::Start);

    connect(ui->btnGroup,&ButtonGroup::startGame,this,[=](){
        // 界面初始化
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        m_gamectl->clearPlayerScore();
        updatePlayerScore();
        // 切换游戏状态 -> 发牌
        gameStatusPrecess(GameControl::DispatchCard);
    });
    connect(ui->btnGroup,&ButtonGroup::playHand,this,&GamePanel::onUserPlayHand);
    connect(ui->btnGroup,&ButtonGroup::pass,this,&GamePanel::onUserPass);
    connect(ui->btnGroup,&ButtonGroup::betPoint,this,[=](int bet){
        m_gamectl->getUserPlayer()->grabLoadBet(bet);
        ui->btnGroup->selectPanel(ButtonGroup::Empty);

    });
}

void GamePanel::initPlayerContext()
{
    // 1.放置玩家扑克牌的区域
    QRect cardsRect[]=
    {
        // x,y, width,height
        QRect(90,130,100,height()-200),                 // 左侧机器人
        QRect(rect().right()-190,130,100,height()-200), // 右侧机器人
        QRect(250,rect().bottom()-120,width()-500,100)  // 中间玩家
    };
    // 2.玩家出牌的区域
    QRect playHandRect[]=
    {
        QRect(260,150,100,100),                         // 左侧机器人
        QRect(rect().right()-360,150,100,100),          // 右侧机器人
        QRect(150,rect().bottom()-290,width()-300,100)  // 中间玩家
    };
    // 3.玩家头像显示的位置
    QPoint roleImgPos[]=
    {
        QPoint(cardsRect[0].left()-80,cardsRect[0].height()/2+20),
        QPoint(cardsRect[1].right()+10,cardsRect[1].height()/2+20),
        QPoint(cardsRect[2].right()-10,cardsRect[2].top()-10)
    };

    int index=m_playerList.indexOf(m_gamectl->getUserPlayer());
    for(int i=0;i<m_playerList.size();i++)
    {
        PlayerContext context;
        context.align= i==index? Horizontal:Vertical;
        context.isFrontSide= i==index?true:false;
        context.cardRect=cardsRect[i];
        context.playHandRect=playHandRect[i];
        // 提示信息
        context.info=new QLabel(this);
        context.info->resize(160,98);
        context.info->hide();
        // 显示到出牌区域的中心位置
        QRect rect=playHandRect[i];
        QPoint pt(rect.left()+(rect.width()-context.info->width())/2,
                  rect.top()+(rect.height()-context.info->height())/2);
        context.info->move(pt);
        // 玩家的头像
        context.roleImg=new QLabel(this);
        context.roleImg->resize(84,120);
        context.roleImg->hide();
        context.roleImg->move(roleImgPos[i]);
        m_contextMap.insert(m_playerList.at(i),context);
    }
}

void GamePanel::initGameScene()
{
    // 1.发牌区的扑克牌
    m_baseCard=new CardPanel(this);
    m_baseCard->setImage(m_cardBackImg,m_cardBackImg);
    // 2.发牌过程中移动的扑克牌
    m_moveCard=new CardPanel(this);
    m_moveCard->setImage(m_cardBackImg,m_cardBackImg);
    // 3.最后的三张底牌（用于窗口显示）
    for(int i=0;i<3;++i)
    {
        CardPanel* panel=new CardPanel(this);
        panel->setImage(m_cardBackImg,m_cardBackImg);
        m_last3Card.push_back(panel);
        panel->hide();
    }
    // 扑克牌的位置
    m_baseCardPos=QPoint((width()-m_cardSize.width())/2,
                           height()/2-100);
    m_baseCard->move(m_baseCardPos);
    m_moveCard->move(m_baseCardPos);

    int base=(width()-3*m_cardSize.width()-2*10)/2;
    for(int i=0;i<3;++i)
    {
        m_last3Card[i]->move(base+(m_cardSize.width()+10)*i,20);
    }
}

void GamePanel::gameStatusPrecess(GameControl::GameStatus status)
{
    // 1.记录游戏状态
    m_gameStatus=status;
    // 2.处理游戏状态
    switch(status)
    {
    case GameControl::DispatchCard:
        startDispatchCard();
        break;
    case GameControl::CallingLord:
        {
            // 在case中定义变量需要放到大括号里
            // 取出底牌数据
            CardList last3Card= m_gamectl->getSurplusCards().toCardList();
            // 给扑克牌窗口设置图片
            for(int i=0;i<last3Card.size();i++)
            {
                // 取出最后的三张底牌
                QPixmap front=m_cardMap[last3Card.at(i)]->getImage();
                m_last3Card[i]->setImage(front,m_cardBackImg);
                // 最后的三张牌抢完地主在显示，因此先隐藏起来
                m_last3Card[i]->hide();
            }
            // 开始叫地主
            m_gamectl->startLordCard();
            break;
        }
    case GameControl::PlayingHand:
        // 隐藏发牌区的底牌和移动的牌
        m_baseCard->hide();
        m_moveCard->hide();
        // 显示留给地主的三张牌
        for(int i=0;i<m_last3Card.size();++i)
        {
            m_last3Card[i]->show();
        }
        // 隐藏各个玩家抢地主过程中的提示信息
        for(int i=0;i<m_playerList.size();++i)
        {
            PlayerContext &context=m_contextMap[m_playerList.at(i)];
            context.info->hide();
            // 显示各个玩家的头像
            Player* player=m_playerList.at(i);
            QPixmap pixmap=loadRoleImage(player->getSex(),player->getDirection(),player->getRole());
            context.roleImg->setPixmap(pixmap);
            context.roleImg->show();
        }
        break;
    default:
        break;
    }
}

void GamePanel::startDispatchCard()
{
    // 重置每张卡牌的属性
    for(auto it=m_cardMap.begin();it!=m_cardMap.end();++it)
    {
        it.value()->setSeclected(false);
        it.value()->setFrontSide(true);
        it.value()->hide();
    }
    // 隐藏三张底牌
    for(int i=0;i<m_last3Card.size();++i)
    {
        m_last3Card.at(i)->hide();
    }
    // 重置玩家的窗口上下文信息
    int index=m_playerList.indexOf(m_gamectl->getUserPlayer());
    for(int i=0;i<m_playerList.size();++i)
    {
        m_contextMap[m_playerList.at(i)].lastCards.clear();
        m_contextMap[m_playerList.at(i)].info->hide();
        m_contextMap[m_playerList.at(i)].roleImg->hide();
        m_contextMap[m_playerList.at(i)].isFrontSide = i==index? true:false;
    }
    // 重置所有玩家的卡牌数据
    m_gamectl->resetCardData();
    // 显示底牌
    m_baseCard->show();
    // 隐藏按钮面板
    ui->btnGroup->selectPanel(ButtonGroup::Empty);
    // 启动定时器
    m_timer->start(10);
    // 播放背景音乐
}

void GamePanel::cardMoveStep(Player *player, int curPos)
{
    // 得到每个玩家的手牌展示区域
    QRect cardRect=m_contextMap[player].cardRect;
    // 每个玩家的单元步长
    int unit[]=
    {
        (m_baseCardPos.x()-cardRect.right())/100,
        (cardRect.left()-m_baseCardPos.x())/100,
        (cardRect.top()-m_baseCardPos.y())/100
    };
    // 每次窗口移动时候每个玩家对应的牌的实时坐标位置
    QPoint pos[]=
    {
        QPoint(m_baseCardPos.x()-curPos*unit[0],m_baseCardPos.y()),
        QPoint(m_baseCardPos.x()+curPos*unit[1],m_baseCardPos.y()),
        QPoint(m_baseCardPos.x(),m_baseCardPos.y()+curPos*unit[2])
    };
    // 移动扑克牌窗口
    int index=m_playerList.indexOf(player);
    m_moveCard->move(pos[index]);

    // 临界状态的处理
    if(curPos==0)
    {
        m_moveCard->show();
    }
    if(curPos==100)
    {
        m_moveCard->hide();
    }
}

void GamePanel::disposCard(Player *player, Cards &cards)
{
    CardList list=cards.toCardList();
    for(int i=0;i<list.size();++i)
    {
        CardPanel* panel=m_cardMap[list.at(i)];
        panel->setOwner(player);
    }
    // 更新扑克牌在窗口中的显示
    updatePlayerCards(player);
}

void GamePanel::updatePlayerCards(Player *player)
{
    Cards cards=player->getCards();
    CardList list=cards.toCardList();

    m_cardsRect=QRect();
    m_userCards.clear();
    // 取出显示扑克牌的区域
    int cardSpace=20;
    QRect cardsrect=m_contextMap[player].cardRect;
    for(int i=0;i<list.size();++i)
    {
        CardPanel* panel=m_cardMap[list.at(i)];
        panel->show();
        panel->raise(); // 让当前窗口显示在子窗口的最上面
        panel->setFrontSide(m_contextMap[player].isFrontSide);

        // 水平 或 垂直展示
        if(m_contextMap[player].align==Horizontal)
        {
            int leftX=cardsrect.left()+(cardsrect.width()-(list.size()-1)*cardSpace-panel->width())/2;
            int topY=cardsrect.top()+(cardsrect.height()-m_cardSize.height())/2;
            if(panel->isSelected())
            {
                // 当前扑克牌被选中了
                topY-=10;
            }
            panel->move(leftX+cardSpace*i,topY);
            m_cardsRect=QRect(leftX,topY,cardSpace*i*m_cardSize.width(),m_cardSize.height());
            int curWidth=0;
            if(list.size()-1==i)
            {
                // 当前牌是最后一张，记录实际宽度
                curWidth=m_cardSize.width();
            }
            else
            {
                curWidth=cardSpace;
            }
            QRect cardRect(leftX+cardSpace*i,topY,curWidth,m_cardSize.height());
            m_userCards.insert(panel,cardRect);
        }
        else
        {
            int leftX=cardsrect.left()+(cardsrect.width()-m_cardSize.width())/2;
            int topY=cardsrect.top()+(cardsrect.height()-(list.size()-1)*cardSpace-panel->height())/2;
            panel->move(leftX,topY+i*cardSpace);
        }
    }

    // 显示玩家打出的牌到出牌区域
    // 得到玩家的出牌区域以及打出去的牌
    QRect playCardRect=m_contextMap[player].playHandRect;
    Cards lastCards=m_contextMap[player].lastCards;
    if(!lastCards.isEmpty())
    {
        int playSpacing=24; // 打出的牌的间隙（堆叠展示）
        CardList lastCardList=lastCards.toCardList();
        auto itplayed=lastCardList.constBegin();
        for(int i=0;itplayed!=lastCardList.constEnd();++itplayed,++i)
        {
            CardPanel* panel=m_cardMap[*itplayed];
            panel->setFrontSide(true);
            panel->raise();  // raise方法，让当前窗口显示到同级别窗口的上方
            // 将打出的牌显示到出牌区域
            if(m_contextMap[player].align==Horizontal)
            {
                int leftBase=playCardRect.left()*
                       (playCardRect.width()-(lastCardList.size()-1)*playSpacing-panel->width())/2;
                int top=playCardRect.top()+(playCardRect.height()-panel->height())/2;
                panel->move(leftBase*i*playSpacing,top);
            }
            else
            {
                int left=playCardRect.left()+(playCardRect.width()-panel->width())/2;
                int top=playCardRect.top();
                panel->move(left,top*i*playSpacing);
            }
            panel->show();
        }
    }
}

QPixmap GamePanel::loadRoleImage(Player::Sex sex, Player::Direction direct, Player::Role role)
{
    // 找图片
    QVector<QString> lordMan;
    QVector<QString> lordWoman;
    QVector<QString> famerMan;
    QVector<QString> famerWoman;

    lordMan<<":/images/lord_man_1.png"<<":/images/lord_man_2.png";
    lordWoman<<":/images/lord_woman_1.png"<<":/images/lord_woman_2.png";
    famerMan<<":/images/farmer_man_1.png"<<":/images/farmer_man_2.png";
    famerWoman<<":/images/farmer_woman_1.png"<<":/images/farmer_woman_2.png";

    // 加载图片 QImage
    QImage image;
    int random=QRandomGenerator::global()->bounded(2); // 使用随机数选择图片
    if(sex==Player::Man&&role==Player::Load)
    {
        image.load(lordMan.at(random));
    }
    else if(sex==Player::Man&&role==Player::Farmer)
    {
        image.load(famerMan.at(random));
    }
    else if(sex==Player::Woman&&role==Player::Load)
    {
        image.load(lordWoman.at(random));
    }
    else if(sex==Player::Woman&&role==Player::Farmer)
    {
        image.load(famerWoman.at(random));
    }

    // Qimage转成 Qpixmap
    QPixmap pixmap;
    if(direct==Player::Left)
    {
        pixmap=QPixmap::fromImage(image);
    }
    else
    {
        // 头像在右边，需要镜像一下
        pixmap=QPixmap::fromImage(image.mirrored(true,false));
    }
    return pixmap;
}

void GamePanel::onDispatchCard()
{
    // 记录扑克牌的位置
    static int curMovePos=0;
    // 当前玩家
    Player* curPlayer=m_gamectl->getCurrentPlayer();
    if(curMovePos>=100)
    {
        // 给玩家发一张牌
        Card card=m_gamectl->takeOneCard();
        curPlayer->storeDispatchCard(card);
        Cards cards(card);
        // disposCard(curPlayer,cards);
        // 切换玩家
        m_gamectl->setCurrentPlayer(curPlayer->getNextPlayer());
        curMovePos=0;
        // 发牌动画
        cardMoveStep(curPlayer,curMovePos);
        // 判断牌是否发完了
        if(m_gamectl->getSurplusCards().cardCount()==3)
        {
            // 种植定时器
            m_timer->stop();
            // 切换游戏状态 发牌 -> 叫地主
            gameStatusPrecess(GameControl::CallingLord);
            return ;
        }

    }
    // 移动扑克牌
    cardMoveStep(curPlayer,curMovePos);
    curMovePos+=15;

}

void GamePanel::onPlayerStatusChanged(Player *player, GameControl::PlayerStatus status)
{
    switch(status)
    {
    case GameControl::ThinkingForCallLord:
        // 切换按钮组的按钮（只有用户玩家才显示按钮组）
        if(player==m_gamectl->getUserPlayer())
        {
            ui->btnGroup->selectPanel(ButtonGroup::CallCard,m_gamectl->getPlayerMaxBet());
        }
        break;
    case GameControl::ThinkingForPlayHand:
        // 1.隐藏上一轮打出的牌
        hidePlayerDropCards(player);
        if(player==m_gamectl->getUserPlayer())
        {
            // 取出出牌玩家的对象
            Player* pendPlayer=m_gamectl->getPendPlayer();
            if(pendPlayer==m_gamectl->getUserPlayer()||pendPlayer==nullptr)
            {
                // 当前玩家是出牌玩家
                ui->btnGroup->selectPanel(ButtonGroup::PlayCard);
            }
            else
            {
                ui->btnGroup->selectPanel(ButtonGroup::PassOrPlay);
            }
        }
        else
        {
            ui->btnGroup->selectPanel(ButtonGroup::Empty);
        }
        break;
    case GameControl::Winning:
        m_contextMap[m_gamectl->getLeftRobot()].isFrontSide=true;
        m_contextMap[m_gamectl->getRightRobot()].isFrontSide=true;
        // 刷新
        updatePlayerCards(m_gamectl->getLeftRobot());
        updatePlayerCards(m_gamectl->getRightRobot());
        // 更新玩家得分
        updatePlayerScore();
        m_gamectl->setCurrentPlayer(player);
        showEndingScorePanel();
        break;
    default:
        break;
    }
}

void GamePanel::onGrabLordBet(Player *player, int bet, bool flag)
{
    // 更新抢地主的信息提示
    PlayerContext context=m_contextMap[player];
    if(bet==0)
    {
        context.info->setPixmap(QPixmap(":/images/buqinag.png"));
    }
    else
    {

        if(flag)
        {
            // 第一次抢地主
            context.info->setPixmap(QPixmap(":/images/jiaodizhu.png"));
        }
        else
        {
            // 第2，3次抢地主
            context.info->setPixmap(QPixmap(":/images/qiangdizhu.png"));
        }
    }
    context.info->show();

    // 显示叫地主的分数
    showAnimation(Bet,bet);

    // 播放分数的背景音乐
}

void GamePanel::onDisposePlayHand(Player *player, Cards &cards)
{
    // 存储玩家打出的牌
    auto it=m_contextMap.find(player);
    it->lastCards=cards;
    // 2.根据出牌的牌型播放游戏特效
    PlayHand hand(cards);
    PlayHand::HandType type=hand.getHandType();
    if(type==PlayHand::Hand_Plane||
        type==PlayHand::Hand_Plane_Two_Pair||
        type==PlayHand::Hand_Plane_Two_Single)
    {
        showAnimation(Plane);
    }
    else if(type==PlayHand::Hand_Seq_Single)
    {
        showAnimation(LianDui);
    }
    else if(type==PlayHand::Hand_Seq_Pair)
    {
        showAnimation(ShunZi);
    }
    else if(type==PlayHand::Hand_Bomb)
    {
        showAnimation(Bomb);
    }
    else if(type==PlayHand::Hand_Bomb_Jokers)
    {
        showAnimation(JokerBomb);
    }

    // 如果玩家打出的是空牌（没出牌），显示提示信息
    if(cards.isEmpty())
    {
        it->info->setPixmap(QPixmap(":/images/pass.png"));
        it->info->show();
    }
    // 3.更新玩家剩余的牌
    updatePlayerCards(player);
    // 4.在出牌的时候播放提示音效

}

void GamePanel::onCardSelected(Qt::MouseButton button)
{
    // 1.判断是不是出牌状态
    if(m_gameStatus==GameControl::DispatchCard||
        m_gameStatus==GameControl::CallingLord)
    {
        return ;
    }
    // 2.判断发出信号的牌的所有者是不是当前用户玩家
    CardPanel* panel=(CardPanel*)sender();  // sender用来获取发射信号的对象
    if(panel->getOwner()!=m_gamectl->getUserPlayer())
    {
        return ;
    }
    // 3.保存当前被选中的牌的窗口对象
    m_curSelCard=panel;
    // 4.判断参数是鼠标左键还是鼠标右键
    if(button==Qt::LeftButton)
    {
        // 设置扑克牌的选中状态
        panel->setSeclected(!panel->isSelected());
        // 更新扑克牌在窗口中的显示
        updatePlayerCards(panel->getOwner());
        // 保存或删除扑克牌窗口对象
        QSet<CardPanel*>::const_iterator it=m_selectCards.find(panel);
        if(it==m_selectCards.constEnd())
        {
            m_selectCards.insert(panel);
        }
        else
        {
            m_selectCards.erase(it);
        }
    }
    else if(Qt::RightButton)
    {
        // 调用出牌按钮的槽函数
        onUserPlayHand();
    }
}

void GamePanel::onUserPlayHand()
{
    // 判断游戏状态
    if(m_gameStatus!=GameControl::PlayingHand)
    {
        return ;
    }
    // 判断玩家是不是用户玩家
    if(m_gamectl->getCurrentPlayer()!=m_gamectl->getUserPlayer())
    {
        return ;
    }
    // 判断要出的牌是否为空
    if(m_selectCards.isEmpty())
    {
        return ;
    }
    // 得到要打出的牌型
    Cards cs;
    for(auto it=m_selectCards.begin();it!=m_selectCards.end();++it)
    {
        Card card=(*it)->getCard();
        cs.add(card);
    }
    PlayHand hand(cs);
    PlayHand::HandType type=hand.getHandType();
    if(type==PlayHand::Hand_Unknown)
    {
        return ;
    }
    // 判断当前玩家的牌能不能压住上家的牌
    if(m_gamectl->getPendPlayer()!=m_gamectl->getUserPlayer())
    {
        Cards cards=m_gamectl->getPendCards();
        if(!hand.canBeeat(PlayHand(cards)))
        {
            return ;
        }
    }
    // 通过玩家对象出牌
    m_gamectl->getUserPlayer()->playHand(cs);
    // 清空容器
    m_selectCards.clear();
}

void GamePanel::onUserPass()
{
    // 判断是不是用户玩家
    Player* curPlayer=m_gamectl->getCurrentPlayer();
    Player* userPlayer=m_gamectl->getUserPlayer();
    if(curPlayer!=userPlayer)
    {
        return ;
    }
    // 判断当前用户玩家是不是上一轮出牌的玩家（可以不处理）
    Player* pendPlayer=m_gamectl->getPendPlayer();
    if(pendPlayer==userPlayer||pendPlayer==nullptr)
    {
        return ;
    }
    // 打出一个空的Cards对象
    Cards empty;
    userPlayer->playHand(empty);
    // 清空用户选择的牌（玩家可能选择了一些牌，但是没有打出去）
    for(auto it=m_selectCards.begin();it!=m_selectCards.end();++it)
    {
        // 设置为未显示状态
        (*it)->setSeclected(false);
    }
    m_selectCards.clear();
    // 更新玩家待出牌区域的牌
    updatePlayerCards(userPlayer);
}

void GamePanel::showAnimation(AnimationType type, int bet)
{
    // 对特效动画类型做判定
    switch(type)
    {
    case AnimationType::LianDui:
        break;
    case AnimationType::ShunZi:
        break;
    case AnimationType::Plane:
        break;
    case AnimationType::Bomb:
        break;
    case AnimationType::JokerBomb:
        break;
    case AnimationType::Bet:
        m_animation->setFixedSize(160,98);
        m_animation->move((width()-m_animation->width())/2,(height()-m_animation->height())/2-140);
        m_animation->showBetScore(bet);
        break;
    }
    // 当显示完分数后，该子窗口就被隐藏了，所以要再show一下以显示其他特效动画
    m_animation->show();
}

void GamePanel::hidePlayerDropCards(Player *player)
{
    // 根据m_contextMap找到player玩家对应的上下文信息
    auto it=m_contextMap.find(player);
    if(it!=m_contextMap.end())
    {
        // lastcards为空表示玩家没有出牌
        if(it->lastCards.isEmpty())
        {
            it->info->hide();
        }
        else
        {
            // Cards --> card
            CardList list=it->lastCards.toCardList();
            for(auto last=list.begin();last!=list.end();++it)
            {
                // 取出每张卡牌的数据
                // 根据单张卡牌数据找到对应的卡牌窗口
                m_cardMap[*last]->hide();
            }
        }
        it->lastCards.clear();
    }
}

void GamePanel::showEndingScorePanel()
{

    bool islord=m_gamectl->getUserPlayer()->getRole()==Player::Load?true:false;
    bool iswin=m_gamectl->getUserPlayer()->isWin();
    EndingPanel* panel=new EndingPanel(islord,iswin,this);
    panel->show();
    panel->move((width()-panel->width())/2,-panel->height());
    panel->setPlayerScore(m_gamectl->getLeftRobot()->getScore(),
                          m_gamectl->getRightRobot()->getScore(),
                          m_gamectl->getUserPlayer()->getScore());


    QPropertyAnimation* animation=new QPropertyAnimation(panel,"geometry",this);
    // 动画持续的时间
    animation->setDuration(1500);
    // 设置窗口的起始位置和终止位置
    animation->setStartValue(QRect(panel->x(),panel->y(),panel->width(),panel->height()));
    animation->setEndValue(QRect((width()-panel->width())/2,(height()-panel->height())/2,
                                 panel->width(),panel->height()));
    // 设置窗口的运动曲线
    animation->setEasingCurve(QEasingCurve(QEasingCurve::OutBounce));
    // 播放动画效果
    animation->start();

    // 处理窗口信号
    connect(panel,&EndingPanel::continueGame,this,[=]()
    {
        panel->close();
        panel->deleteLater();
        animation->deleteLater();
        // 把出牌按钮隐藏起来
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        // 继续发牌
        gameStatusPrecess(GameControl::DispatchCard);
    });

}

void GamePanel::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawPixmap(rect(),m_bkimage);
}

void GamePanel::mouseMoveEvent(QMouseEvent *ev)
{
    // 左键参与到鼠标移动
    if(ev->button()& Qt::LeftButton)
    {
        // 找到鼠标的位置
        QPoint pt=ev->pos();
        // 判断鼠标有没有在扑克牌区域里
        if(!m_cardsRect.contains(pt))
        {
            m_curSelCard=nullptr;
        }
        else
        {
            QList<CardPanel*> list=m_userCards.keys();
            for(int i=0;i<list.size();++i)
            {
                CardPanel* panel=list.at(i);
                // 如果鼠标位置在某张扑克牌上面
                if(m_userCards[panel].contains(pt)&&m_curSelCard!=panel)
                {
                    // 点击这张扑克牌
                    panel->clicked();
                    m_curSelCard=panel;
                }
            }
        }
    }
    else
    {

    }
}



