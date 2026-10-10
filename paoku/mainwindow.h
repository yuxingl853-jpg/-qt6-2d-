#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QList>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPalette>
#include <QBrush>
#include <QPixmap>
#include "ui_mainwindow.h"

enum class ObstacleType { StaticGround, StaticAir, MovingVertical };
enum class ItemType {
    Heart,
    Dash
};
class Item : public QGraphicsEllipseItem {
public:
    ItemType type;
    Item(ItemType t, float x, float y);
};
// 1. 地刺障碍物
class SpikeItem : public QGraphicsPolygonItem {
public:
    SpikeItem(float x, float y, float w = 24, float h = 30);
    QPainterPath shape() const override;
};

// 2. 高速飞弹
class MissileItem : public QGraphicsRectItem {
public:
    MissileItem(float x, float y);
    void updateLogic();
};

// 3. 障碍物
class ObstacleItem : public QGraphicsRectItem {
public:
    ObstacleType type;
    float floatAngle = 0;

    ObstacleItem(ObstacleType t, float x, float y, float w, float h);
    void updateLogic();
};

// 4. 主角类（新增血量与无敌机制）
class Player : public QGraphicsPixmapItem {
public:
    float velocityY = 0;
    bool isGrounded = false;
    bool isCrouching = false;
    int jumpCount = 0;
    const int maxJumps = 2;
    const float gravity = 0.8f;
    const float jumpStrength = -13.0f;

    // 血量与无敌帧
    int hp = 3;
    const int maxHp = 3;
    bool isInvincible = false;
    int invincibleTimer = 0; // 无敌帧倒计时

    Player();
    void jump();
    void crouch(bool start);
    void fastDrop();
    void updatePhysics();
    void takeDamage();       // 扣血逻辑
    void reset();
    void animateFrame();
    QPixmap run1Pixmap;
    QPixmap run2Pixmap;
    QPixmap crouchPixmap;
    QPixmap jumpPixmap;

private:

    int runAnimFrame = 0;
};


// 5. 主窗口
enum class GameState { StartMenu, Playing, GameOver };

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameUpdate();
    void onCrouchTimeout();
    void updateCountdown();

private:
    Ui::MainWindow *ui;
    void showStartMenu();
    void startGame();
    void resetGame();
    void spawnMapElements();
    float getScrollSpeed() const;
    void handleGameOver(const QString &reason);

    QGraphicsView *view;
    QGraphicsScene *scene;
    Player *player;
    QTimer *gameTimer;
    QTimer *crouchTimer;

    QGraphicsTextItem *countdownTextItem = nullptr;
    int countdownValue = 3;
    QTimer *countdownTimer = nullptr;

    void startCountdown();
    bool isDashing = false;
    QTimer *dashTimer = nullptr;

    QList<Item*> itemList;
    QGraphicsPixmapItem *bgItem1;
    QGraphicsPixmapItem *bgItem2;
    QList<QGraphicsRectItem*> platforms;
    QList<ObstacleItem*> obstacles;
    QList<SpikeItem*> spikes;
    QList<MissileItem*> missiles;
    QList<QGraphicsEllipseItem*> coins;

    QGraphicsTextItem *uiTextItem;
    GameState currentState = GameState::StartMenu;

    int score = 0;
    int distanceCounter = 0;
    float lastPlatformRightX = 0;
    float lastPlatformY = 300;
};

#endif // MAINWINDOW_H