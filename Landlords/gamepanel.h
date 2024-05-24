#ifndef GAMEPANEL_H
#define GAMEPANEL_H

#include <QTimer>
#include <QMainWindow>
#include <QLabel>
#include <QMap>
#include "cardpanel.h"
#include "gamecontrol.h"
#include "animationwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class GamePanel; }
QT_END_NAMESPACE

class GamePanel : public QMainWindow
{
    Q_OBJECT

public:
    GamePanel(QWidget *parent = nullptr);
    ~GamePanel();

    // 要显示的特效动画类型
    enum AnimationType{ShunZi,LianDui,Plane,JokerBomb,Bomb,Bet};

    // 初始化游戏控制类信息
    void gameControlInit();
    // 更新分数面板的分数
    void updatePlayerScore();
    // 切割并存储图片
    void initCardMap();
    // 裁剪图片
    void cropImage(QPixmap& pix,int x,int y,Card& c);
    // 初始化游戏按钮组
    void initButtonsGroup();
    // 初始化玩家在窗口中的上下文环境
    void initPlayerContext();
    // 初始化游戏场景
    void initGameScene();
    // 处理游戏的状态
    void gameStatusPrecess(GameControl::GameStatus status);
    // 发牌
    void startDispatchCard();
    // 移动扑克牌
    void cardMoveStep(Player* player,int curPos);
    // 处理分发得到的扑克牌
    void disposCard(Player* player,Cards& cards);
    // 更新扑克牌在窗口中的显示
    void updatePlayerCards(Player* player);
    // 加载玩家头像的函数
    QPixmap loadRoleImage(Player::Sex sex,Player::Direction direct,Player::Role role);

    // 定时器的处理动作
    void onDispatchCard();
    // 处理玩家状态的变化
    void onPlayerStatusChanged(Player* player,GameControl::PlayerStatus status);
    // 处理玩家抢地主
    void onGrabLordBet(Player* player,int bet,bool flag);
    // 处理玩家的出牌
    void onDisposePlayHand(Player* player,Cards& cards);
    // 处理玩家选牌
    void onCardSelected(Qt::MouseButton button);

    // 显示特效动画
    void showAnimation(AnimationType type,int bet=0);
    // 隐藏玩家打出的牌
    void hidePlayerDropCards(Player* player);

protected:
    void paintEvent(QPaintEvent* ev);
    void mouseMoveEvent(QMouseEvent* ev);


private:
    // 对齐方式
    enum CardAlign{Horizontal,Vertical};
    struct PlayerContext
    {
        // 1.玩家扑克牌显示的区域
        QRect cardRect;
        // 2.出牌的区域
        QRect playHandRect;
        // 3.扑克牌的对齐方式（水平 or 垂直）
        CardAlign align;
        // 4.扑克牌显示正面还是背面
        bool isFrontSide;
        // 5.游戏过程中的提示信息，比如：不出
        QLabel* info;
        // 6.玩家的头像
        QLabel* roleImg;
        // 7.玩家刚打出的牌
        Cards lastCards;
    };

    Ui::GamePanel *ui;
    QPixmap m_bkimage;                  // 主界面背景图片
    GameControl *m_gamectl;             // 游戏控制列
    QVector<Player*> m_playerList;      // 玩家的实例对象
    QMap<Card,CardPanel*> m_cardMap;    // 扑克牌相关数据（数据体，单张扑克牌窗口）
    QSize m_cardSize;                   // 每张扑克牌的宽度高度
    QPixmap m_cardBackImg;              // 每张扑克牌的背景图
    QMap<Player*,PlayerContext> m_contextMap; // 玩家实例与玩家上下文关系的映射
    CardPanel* m_baseCard;              // 发牌区的扑克牌
    CardPanel* m_moveCard;              // 发牌过程中移动的扑克牌
    QVector<CardPanel*> m_last3Card;    // 最后的三张底牌
    QPoint m_baseCardPos;               // 发牌的位置
    GameControl::GameStatus m_gameStatus; // 游戏状态
    QTimer* m_timer;                    // 定时器
    AnimationWindow* m_animation;       // 窗口动画类
    CardPanel* m_curSelCard;            // 保存当前被选中的牌的窗口对象
    QSet<CardPanel*> m_selectCards;     // 保存当前被选中的牌的窗口对象（多张）
    QRect m_cardsRect;                  // 非机器人玩家剩余的扑克牌显示的区域
    QHash<CardPanel*,QRect> m_userCards;// 非机器人手中的牌 与 每张牌在窗口中的位置的映射
};
#endif // GAMEPANEL_H

