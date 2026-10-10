#include "mainwindow.h"
#include <QRandomGenerator>
#include <QtMath>

// --- SpikeItem 实现 ---
SpikeItem::SpikeItem(float x, float y, float w, float h) {
    QPolygonF triangle;
    triangle << QPointF(0, h) << QPointF(w / 2, 0) << QPointF(w, h);
    setPolygon(triangle);
    setBrush(Qt::darkGray);
    setPen(QPen(Qt::black, 1));
    setPos(x, y - h);
}
QPainterPath SpikeItem::shape() const {
    QPainterPath path;
    // 稍微缩小一圈的碰撞三角形，给玩家留出擦边的容错空间
    QPolygonF shrinkTriangle;
    shrinkTriangle << QPointF(12, 4) << QPointF(3, 28) << QPointF(21, 28);
    path.addPolygon(shrinkTriangle);
    return path;
}
// --- MissileItem 实现 ---
MissileItem::MissileItem(float x, float y) {
    setRect(0, 0, 35, 16);
    setBrush(Qt::darkRed);
    setPen(QPen(Qt::red, 1));
    setPos(x, y);
}

void MissileItem::updateLogic() {}

// --- ObstacleItem 实现 ---
ObstacleItem::ObstacleItem(ObstacleType t, float x, float y, float w, float h) : type(t) {
    setRect(0, 0, w, h);
    setPos(x, y);

    if (type == ObstacleType::MovingVertical) {
        // 将原本上下移动的障碍物颜色改为绿/橙色，提醒玩家这是可踩踏的
        setBrush(Qt::green);
    } else {
        setBrush(Qt::red); // 普通静态障碍物依然是红色
    }
}

void ObstacleItem::updateLogic() {
    if (type == ObstacleType::MovingVertical) {
        float floatOffset = qSin(floatAngle) * 3.0f;
        floatAngle += 0.1f;
        moveBy(0, floatOffset);
    }
}
// ================= Item 道具实现 =================
Item::Item(ItemType t, float x, float y) : type(t) {
    setRect(0, 0, 20, 20);
    setPos(x, y);

    if (type == ItemType::Heart) {
        setBrush(Qt::red);          // 加血心：红色
        setPen(QPen(Qt::white, 1));
    } else if (type == ItemType::Dash) {
        setBrush(Qt::cyan);         // 冲刺道具：青色
        setPen(QPen(Qt::yellow, 2));
    }
}
// --- Player 实现 ---
Player::Player() {
    run1Pixmap = QPixmap(":/image/paoku1.png").scaled(35, 45, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    run2Pixmap = QPixmap(":/image/paoku2.png").scaled(35, 45, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    crouchPixmap = QPixmap(":/image/paoku3.png").scaled(35, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    jumpPixmap = QPixmap(":/image/paoku4.png").scaled(35, 45, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    reset(); }
void Player::reset() {
    setPixmap(run1Pixmap); // 设置初始图片
    setPos(100, 255);
    velocityY = 0;
    isGrounded = false;
    isCrouching = false;
    jumpCount = 0;
    hp = maxHp;
    isInvincible = false;
    invincibleTimer = 0;
    runAnimFrame =0;
}


void Player::takeDamage() {
    if (!isInvincible && hp > 0) {
        hp--;
        if (hp > 0) {
            isInvincible = true;
            invincibleTimer = 120;
        }
    }
}

void Player::jump() {
    if (isCrouching) crouch(false);
    if (jumpCount < maxJumps) {
        velocityY = jumpStrength;
        jumpCount++;
        isGrounded = false;
    }
}

void Player::crouch(bool start) {
    if (start && !isCrouching) {
        isCrouching = true;
        setPixmap(crouchPixmap); // 切换为下蹲 PNG
        if (isGrounded) setPos(x(), y() + 23);
    } else if (!start && isCrouching) {
        isCrouching = false;
        setPixmap(run1Pixmap);    // 恢复站立奔跑 PNG
        setPos(x(), y() - 23);
    }
}

void Player::fastDrop() {
    if (!isGrounded) velocityY = 18.0f;
}

void Player::updatePhysics() {
    velocityY += gravity;
    setPos(x(), y() + velocityY);

    // 处理无敌时间倒计时
    if (isInvincible) {
        invincibleTimer--;
        if (invincibleTimer <= 0) {
            isInvincible = false;
        }
    }

    animateFrame();
}

void Player::animateFrame() {
    if (isInvincible && (invincibleTimer / 4) % 2 == 0) {
        setOpacity(0.3); // 受击无敌闪烁效果保持不变
    } else {
        setOpacity(1.0);
    }

    // 根据状态自动切换渲染图片
    if (isCrouching) {
        setPixmap(crouchPixmap);
    } else if (!isGrounded) {
        setPixmap(jumpPixmap);
    } else {
        runAnimFrame++;
        if((runAnimFrame/32)%2==0){
            setPixmap(run1Pixmap);
        }
        else{
            setPixmap(run2Pixmap);
        }

    }
}

// --- MainWindow 实现 ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
    // 给 stackedWidget 的第 0 页设置背景图
    ui->stackedWidget->widget(0)->setStyleSheet(
        "QWidget#page { "
        "border-image: url(:/image/yx.jpg) 0 0 0 0 stretch stretch;"
        "}"
        );
    ui->centralwidget->layout()->setContentsMargins(0, 0, 0, 0); // 清除中心部件边距
    ui->graphicsView->setFixedSize(800, 400);
    // 在构造函数或初始化函数中
    QPixmap pixmap(":/images/bg.jpg"); // 确保路径正确
    QPalette palette = this->palette();
    // 将图片缩放至窗口大小并设置为背景画刷
    palette.setBrush(QPalette::Window, QBrush(pixmap.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    this->setPalette(palette);
    this->setAutoFillBackground(true); // 必须开启这一句！

    setFixedSize(800, 400);
    setWindowTitle("天天酷跑 - demo");

    scene = new QGraphicsScene(0, 0, 800, 400, this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    view=ui->graphicsView;
    ui->stackedWidget->setCurrentIndex(0);

    // 3. 点击“开始游戏”按钮，切换到第二页（游戏页，索引为 1）并启动游戏
    connect(ui->btnStart, &QPushButton::clicked, this, [=](){
        ui->stackedWidget->setCurrentIndex(1); // 切换到游戏界面
        resetGame();                       // 启动游戏逻辑
        ui->graphicsView->setFocus();          // 确保 View 获得焦点以响应键盘
        startCountdown();
    });

    // 初始化背景、玩家等游戏组件...
    QPixmap bgPixmap(":/image/bg.png");
    QPixmap scaledBg =bgPixmap.scaled(800,400,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    bgItem1 =new QGraphicsPixmapItem(scaledBg);
    bgItem1->setPos(0, 0);
    bgItem1->setZValue(-100); // 确保放置在场景最底层
    scene->addItem(bgItem1);

    bgItem2 = new QGraphicsPixmapItem(scaledBg);
    bgItem2->setPos(800, 0);  // 拼接在第一张背景图的右侧
    bgItem2->setZValue(-100);
    scene->addItem(bgItem2);

    player = new Player();
    scene->addItem(player);

    uiTextItem = scene->addText("");
    uiTextItem->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    uiTextItem->setDefaultTextColor(Qt::blue);
    uiTextItem->setPos(10, 10);

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameUpdate);

    crouchTimer = new QTimer(this);
    crouchTimer->setSingleShot(true);
    connect(crouchTimer, &QTimer::timeout, this, &MainWindow::onCrouchTimeout);

    // 倒计时提示文本控件
    countdownTextItem = scene->addText("");
    countdownTextItem->setFont(QFont("Microsoft YaHei", 48, QFont::Bold));
    countdownTextItem->setDefaultTextColor(Qt::red);
    countdownTextItem->setZValue(1000); // 放置于最顶层
    countdownTextItem->hide();

    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &MainWindow::updateCountdown);

    dashTimer = new QTimer(this);
    dashTimer->setSingleShot(true);
    connect(dashTimer, &QTimer::timeout, this, [=](){
        isDashing = false;
        player->isInvincible = false;
        player->velocityY = 0; // 结束飞行准备从空中缓缓降落
    });
}
MainWindow::~MainWindow() {
    delete ui;
}
void MainWindow::startCountdown() {
    countdownValue = 3;
    countdownTextItem->setPlainText("3");
    // 调整居中显示
    countdownTextItem->setPos(400 - countdownTextItem->boundingRect().width() / 2,
                              200 - countdownTextItem->boundingRect().height() / 2);
    countdownTextItem->show();

    // 处于倒计时状态，暂停物理更新
    currentState = GameState::StartMenu;
    countdownTimer->start(1000); // 每 1 秒刷新一次
}

void MainWindow::updateCountdown() {
    countdownValue--;
    if (countdownValue > 0) {
        countdownTextItem->setPlainText(QString::number(countdownValue));
        countdownTextItem->setPos(400 - countdownTextItem->boundingRect().width() / 2,
                                  200 - countdownTextItem->boundingRect().height() / 2);
    } else if (countdownValue == 0) {
        countdownTextItem->setPlainText("GO!");
        countdownTextItem->setPos(400 - countdownTextItem->boundingRect().width() / 2,
                                  200 - countdownTextItem->boundingRect().height() / 2);
    } else {
        countdownTimer->stop();
        countdownTextItem->hide();
        startGame(); // 真正的开始游戏，启动 gameTimer
    }
}
float MainWindow::getScrollSpeed() const {
    if (isDashing) return 15.0f;
    if (score > 1000) return 7.0f;
    if (score > 400) return 4.5f;
    return 3.0f;
}

void MainWindow::showStartMenu() {
    currentState = GameState::StartMenu;
    gameTimer->stop();
    resetGame();

}

void MainWindow::startGame() {
    currentState = GameState::Playing;
    gameTimer->start(16);
}

void MainWindow::resetGame() {
    score = 0;
    distanceCounter = 0;
    lastPlatformRightX = 0;
    lastPlatformY = 300;
    crouchTimer->stop();
    isDashing = false;
    if (dashTimer) dashTimer->stop();

    if (bgItem1 && bgItem2) {
        bgItem1->setPos(0, 0);
        bgItem2->setPos(800, 0);
    }
    player->reset();

    for (auto p : platforms) { scene->removeItem(p); delete p; }
    platforms.clear();
    for (auto o : obstacles) { scene->removeItem(o); delete o; }
    obstacles.clear();
    for (auto s : spikes) { scene->removeItem(s); delete s; }
    spikes.clear();
    for (auto m : missiles) { scene->removeItem(m); delete m; }
    missiles.clear();
    for (auto c : coins) { scene->removeItem(c); delete c; }
    coins.clear();
    for (auto it : itemList) { scene->removeItem(it); delete it; }
    itemList.clear();

    QGraphicsRectItem *startPlatform = new QGraphicsRectItem(0, 0, 500, 150);
    QPixmap platformTexture(":/image/kuai.png");
    startPlatform->setBrush(QBrush(platformTexture));
    startPlatform->setPen(Qt::NoPen);
    startPlatform->setPos(0, 300);
    scene->addItem(startPlatform);
    platforms.append(startPlatform);
    lastPlatformRightX = 500;
    lastPlatformY = 300;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (isDashing) return;
    if (event->key() == Qt::Key_Space) {
        if (currentState == GameState::StartMenu) {
            startGame();
        } else if (currentState == GameState::Playing) {
            if (player->isCrouching) {
                crouchTimer->stop();
                player->crouch(false);
            }
            player->jump();
        }
    } else if (event->key() == Qt::Key_S && currentState == GameState::Playing) {
        player->crouch(true);
        crouchTimer->start(500);
        if (!player->isGrounded) player->fastDrop();
    }
}

void MainWindow::onCrouchTimeout() {
    if (currentState == GameState::Playing && player->isCrouching) {
        player->crouch(false);
    }
}

// 地图与连排地刺生成
void MainWindow::spawnMapElements() {
    if (lastPlatformRightX < 950) {
        int gap = QRandomGenerator::global()->bounded(70, 160);
        int pWidth = QRandomGenerator::global()->bounded(220, 380);

        int deltaY = QRandomGenerator::global()->bounded(-80, 80);
        int pY = qBound(150, (int)lastPlatformY + deltaY, 330);

        float spawnX = lastPlatformRightX + gap;

        QGraphicsRectItem *plat = new QGraphicsRectItem(0, 0, pWidth, 200);

        // 使用地块 PNG 进行平铺填充（TexturePattern）
        QPixmap platformTexture(":/image/kuai.png");
        plat->setBrush(QBrush(platformTexture));
        plat->setPen(Qt::NoPen); // 隐藏外边框线，看起来更自然
        plat->setPos(spawnX, pY);
        scene->addItem(plat);
        platforms.append(plat);
        int randVal = QRandomGenerator::global()->bounded(100);
        if (randVal < 35) {
            // 核心功能：随机生成 1 ~ 3 连排地刺！
            int spikeCount = QRandomGenerator::global()->bounded(1, 4); // 随机 1、2 或 3 个
            float startSpikeX = spawnX + pWidth / 3;
            for (int k = 0; k < spikeCount; ++k) {
                SpikeItem *spike = new SpikeItem(startSpikeX + (k * 24), pY); // 紧挨着连排摆放
                scene->addItem(spike);
                spikes.append(spike);
            }
        } else if (randVal < 60) {
            ObstacleItem *obs = new ObstacleItem(ObstacleType::StaticAir, spawnX + pWidth / 2, pY - 55, 45, 30);
            scene->addItem(obs);
            obstacles.append(obs);
        } else if (randVal < 80) {
            ObstacleItem *obs = new ObstacleItem(ObstacleType::MovingVertical, spawnX + pWidth / 2, pY - 70, 25, 25);
            scene->addItem(obs);
            obstacles.append(obs);
        }

        if (QRandomGenerator::global()->bounded(100) < 40) {
            float missileY = pY - QRandomGenerator::global()->bounded(30, 80);
            MissileItem *missile = new MissileItem(900, missileY);
            scene->addItem(missile);
            missiles.append(missile);
        }

        for (int i = 20; i < pWidth - 20; i += 35) {
            if (QRandomGenerator::global()->bounded(100) < 50) {
                QGraphicsEllipseItem *coin = new QGraphicsEllipseItem(0, 0, 15, 15);
                coin->setBrush(Qt::yellow);
                coin->setPos(spawnX + i, pY - 25);
                scene->addItem(coin);
                coins.append(coin);
            }
        }
        if (QRandomGenerator::global()->bounded(100) < 20) {
            ItemType t = (QRandomGenerator::global()->bounded(100) < 60) ? ItemType::Heart : ItemType::Dash;
            float itemY = pY - QRandomGenerator::global()->bounded(30, 70);
            Item *item = new Item(t, spawnX + pWidth / 2, itemY);
            scene->addItem(item);
            itemList.append(item);
        }

        lastPlatformRightX = spawnX + pWidth;
        lastPlatformY = pY;
    }
}

void MainWindow::gameUpdate() {
    if (currentState != GameState::Playing) return;

    if (isDashing) {
        player->setPos(player->x(), 80); // 固定高空飞行 Y 值（无视重力）
        player->velocityY = 0;           // 清空垂直下落速度
        player->isGrounded = false;
        player->animateFrame();          // 维持帧动画显示
    } else {
        player->updatePhysics();         // 正常状态才更新重力物理
    }

    spawnMapElements();
    float scrollSpeed = getScrollSpeed();
    float bgSpeed = scrollSpeed * 0.4f;
    bgItem1->moveBy(-bgSpeed, 0);
    bgItem2->moveBy(-bgSpeed, 0);

    // 当图片完全滑出屏幕左边 (-800) 时，重新拼接到另一张图片的右边
    if (bgItem1->x() <= -800) {
        bgItem1->setPos(bgItem2->x() + 800, 0);
    }
    if (bgItem2->x() <= -800) {
        bgItem2->setPos(bgItem1->x() + 800, 0);
    }

    // 跑动距离自动加分
    distanceCounter++;
    if (distanceCounter >= 6) {
        score += 1;
        distanceCounter = 0;
    }

    lastPlatformRightX -= scrollSpeed;

    // 1. 平台移动与落地
    bool onAnyPlatform = false;
    float playerWidth = player->pixmap().width();
    float playerHeight = player->pixmap().height();

    for (int i = platforms.size() - 1; i >= 0; --i) {
        auto plat = platforms[i];
        plat->moveBy(-scrollSpeed, 0);

        if (player->collidesWithItem(plat)) {
            // 获取平台与玩家的边界坐标
            float playerRight = player->x() + playerWidth;
            float playerBottom = player->y() + playerHeight;
            float platLeft = plat->x();
            float platTop = plat->y();

            // A. 判断是否落在平台顶面：玩家脚底高于平台表面，且处于下落状态
            if (player->velocityY >= 0 && (playerBottom - player->velocityY) <= platTop + 12) {
                player->setPos(player->x(), platTop - playerHeight);
                player->velocityY = 0;
                player->isGrounded = true;
                player->jumpCount = 0;
                onAnyPlatform = true;
            }
            // B. 判断是否撞到平台左侧壁：玩家右侧卡在平台左侧壁内部
            else if (playerRight > platLeft && player->x() < platLeft && playerBottom > platTop + 5) {
                // 强制将玩家退回到平台左侧壁外（抵消平台向左滚动并被推走效果）
                player->setPos(platLeft - playerWidth, player->y());
            }
        }

        if (plat->x() < -plat->rect().width()) {
            scene->removeItem(plat);
            platforms.removeAt(i);
            delete plat;
        }
    }

    // 只要没有踩在任何平台上，恢复空中下落状态
    if (!onAnyPlatform) {
        player->isGrounded = false;
    }

    // 2. 飞弹碰撞（扣 1 血）
    for (int i = missiles.size() - 1; i >= 0; --i) {
        auto m = missiles[i];
        m->moveBy(-scrollSpeed - 3.0f, 0);

        if (player->collidesWithItem(m)) {
            if (!player->isInvincible) {
                player->takeDamage();
            }
            scene->removeItem(m);
            missiles.removeAt(i);
            delete m;
            if (player->hp <= 0) {
                handleGameOver("血量耗尽！被飞弹击毁。");
                return;
            }
            continue;
        }

        if (m->x() < -50) {
            scene->removeItem(m);
            missiles.removeAt(i);
            delete m;
        }
    }

    // 3. 地刺碰撞（扣 1 血，含无敌帧）
    for (int i = spikes.size() - 1; i >= 0; --i) {
        auto s = spikes[i];
        s->moveBy(-scrollSpeed, 0);

        if (player->collidesWithItem(s) && !player->isInvincible) {
            player->takeDamage();
            if (player->hp <= 0) {
                handleGameOver("血量耗尽！踩到地刺。。");
            }
        }

        if (s->x() < -50) {
            scene->removeItem(s);
            spikes.removeAt(i);
            delete s;
        }
    }

    // 4. 方块障碍物碰撞
    for (int i = obstacles.size() - 1; i >= 0; --i) {
        auto obs = obstacles[i];
        obs->moveBy(-scrollSpeed, 0);
        obs->updateLogic();

        if (player->collidesWithItem(obs)) {
            // 获取玩家和障碍物的尺寸与位置信息
            float playerBottom = player->y() + player->pixmap().height();
            float obsTop = obs->y();

            // 判断条件：上下移动障碍物 + 处于下落状态 + 玩家脚底高于障碍物顶面（留出一点判定容错空间，如 15px）
            if (obs->type == ObstacleType::MovingVertical && player->velocityY >= 0 && (playerBottom - player->velocityY) <= obsTop + 15) {
                // 【踩踏成功】
                player->velocityY = -10.0f; // 给予玩家一个向上反弹的冲量（弹跳效果）
                player->jumpCount = 1;      // 重置或刷新跳跃次数，方便空中二次弹跳
                score += 20;               // 增加踩踏得分

                // 移除并销毁该障碍物
                scene->removeItem(obs);
                obstacles.removeAt(i);
                delete obs;
                continue; // 踩踏成功后直接跳过后续碰撞判定
            }
            // 如果不满足踩踏条件（侧面撞击或底撞），且玩家不处于无敌状态，则扣血
            else if (!player->isInvincible) {
                player->takeDamage();
                if (player->hp <= 0) {
                    handleGameOver("血量耗尽！撞到障碍物。");
                    return;
                }
            }
        }

        // 超出左边界清除
        if (obs->x() < -50) {
            scene->removeItem(obs);
            obstacles.removeAt(i);
            delete obs;
        }
    }

    // 5. 金币收集
    for (int i = coins.size() - 1; i >= 0; --i) {
        auto coin = coins[i];
        coin->moveBy(-scrollSpeed, 0);

        if (player->collidesWithItem(coin)) {
            score += 10;
            scene->removeItem(coin);
            coins.removeAt(i);
            delete coin;
            continue;
        }

        if (coin->x() < -20) {
            scene->removeItem(coin);
            coins.removeAt(i);
            delete coin;
        }
    }
    // 6. 道具拾取 (加血心 / 冲刺)
    for (int i = itemList.size() - 1; i >= 0; --i) {
        auto item = itemList[i];
        item->moveBy(-scrollSpeed, 0);

        if (player->collidesWithItem(item)) {
            if (item->type == ItemType::Heart) {
                if (player->hp < player->maxHp) player->hp++;
            } else if (item->type == ItemType::Dash) {
                isDashing = true;
                player->isInvincible = true;
                dashTimer->start(3000); // 开启 3 秒无敌冲刺
            }

            scene->removeItem(item);
            itemList.removeAt(i);
            delete item;
            continue;
        }

        if (item->x() < -20) {
            scene->removeItem(item);
            itemList.removeAt(i);
            delete item;
        }
    }

    // 7. 掉落虚空（强制秒杀，直接归零）
    if (player->y() > 400) {
        player->hp = 0;
        handleGameOver("掉入虚空！");
        return;
    }

    //8. 刷新 UI 界面
        QString hpText = "";
    for (int k = 0; k < player->hp; ++k) hpText += "❤️";
    uiTextItem->setPlainText(QString("得分: %1   血量: %2 %3")
                                 .arg(score)
                                 .arg(hpText)
                                 .arg(isDashing ? " [无敌冲刺中!]" : ""));
}

// 封装一个通用的 GameOver 处理函数（建议在 MainWindow 中定义）
void MainWindow::handleGameOver(const QString &reason) {
    currentState = GameState::GameOver;
    gameTimer->stop();

    // 1. 创建弹窗
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Game Over");
    msgBox.setText(QString("%1\n最终得分: %2").arg(reason).arg(score));

    // 2. 添加两个选项按钮
    QPushButton *retryBtn = msgBox.addButton("再试一次", QMessageBox::AcceptRole);
    QPushButton *menuBtn = msgBox.addButton("返回主界面", QMessageBox::RejectRole);

    msgBox.exec(); // 阻塞等待玩家点击

    // 3. 根据玩家的点击进行处理
    if (msgBox.clickedButton() == retryBtn) {
        // 点击再试一次：重置场景并进入倒计时
        resetGame();
        startCountdown();
    } else if (msgBox.clickedButton() == menuBtn) {
        // 点击返回主界面：重置场景并切回 ui->stackedWidget 第 0 页
        resetGame();
        ui->stackedWidget->setCurrentIndex(0);
    }
}
