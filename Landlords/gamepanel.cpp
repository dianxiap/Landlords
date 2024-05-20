#include <QPainter>
#include <QRandomGenerator>
#include "gamepanel.h"
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
    connect(ui->btnGroup,&ButtonGroup::playHand,this,[=](){});
    connect(ui->btnGroup,&ButtonGroup::pass,this,[=](){});
    connect(ui->btnGroup,&ButtonGroup::betPoint,this,[=](int bet){
        m_gamectl->getUserPlayer()->grabLoadBet(bet);

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
                           (height()-m_cardSize.height())/2-100);
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
        m_contextMap[m_playerList.at(i)].isFrontSide = i==index? true;false;
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
        }
        else
        {
            int leftX=cardsrect.left()+(cardsrect.width()-m_cardSize.width())/2;
            int topY=cardsrect.top()+(cardsrect.height()-(list.size()-1)*cardSpace-panel->height())/2;
            panel->move(leftX,topY+i*cardSpace);
        }
    }
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
        disposCard(curPlayer,cards);
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
            ui->btnGroup->selectPanel(ButtonGroup::CallCard);
        }
        break;
    case GameControl::ThinkingForPlayHand:
        break;
    case GameControl::Winning:
    default:
        break;
    }
}

void GamePanel::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawPixmap(rect(),m_bkimage);
}



