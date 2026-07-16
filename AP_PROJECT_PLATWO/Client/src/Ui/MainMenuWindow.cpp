#include "mainmenuwindow.h"
#include "ui_mainmenuwindow.h"
#include "client_manager.h"
#include <QMessageBox>
#include <QAction>
#include <QIcon>
#include <QDebug>
#include <QFile>
#include <QException>

MainMenuWindow::MainMenuWindow(user* loggedInUser, ClientManager* client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainMenuWindow)
    , currentUser(loggedInUser)
    , clientManager(client)
    , mainMenuWidget(nullptr)
    , gameDetailPage(nullptr)
    , editProfilePage(nullptr) {

    qDebug() << "[MainMenuWindow] Constructor started";
    qDebug() << "[MainMenuWindow] loggedInUser:" << (loggedInUser ? loggedInUser->username : "null");
    qDebug() << "[MainMenuWindow] clientManager:" << (client ? "valid" : "null");

    // ===== مرحله 1: راه‌اندازی UI =====
    qDebug() << "[MainMenuWindow] Setting up UI...";
    ui->setupUi(this);
    qDebug() << "[MainMenuWindow] UI setup complete";

    // ===== مرحله 2: تنظیم ویژگی‌های پنجره =====
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setAttribute(Qt::WA_StyledBackground, true);
    qDebug() << "[MainMenuWindow] Attributes set";

    // ===== مرحله 3: ایجاد MainMenu =====
    qDebug() << "[MainMenuWindow] Creating MainMenu widget...";
    mainMenuWidget = new MainMenu(this);
    if (!mainMenuWidget) {
        qDebug() << "[MainMenuWindow] ERROR: mainMenuWidget is null!";
        QMessageBox::critical(this, "Error", "Failed to create MainMenu widget.");
        return;
    }
    qDebug() << "[MainMenuWindow] MainMenu created";

    // ===== مرحله 4: ایجاد GameDetailPage =====
    qDebug() << "[MainMenuWindow] Creating GameDetailPage...";
    gameDetailPage = new GameDetailPage(currentUser, clientManager, this);
    if (!gameDetailPage) {
        qDebug() << "[MainMenuWindow] ERROR: gameDetailPage is null!";
        QMessageBox::critical(this, "Error", "Failed to create GameDetailPage.");
        return;
    }
    qDebug() << "[MainMenuWindow] GameDetailPage created";

    // ===== مرحله 5: ایجاد EditProfilePage =====
    qDebug() << "[MainMenuWindow] Creating EditProfilePage...";
    editProfilePage = new EditProfilePage(currentUser, clientManager, this);
    if (!editProfilePage) {
        qDebug() << "[MainMenuWindow] ERROR: editProfilePage is null!";
        QMessageBox::critical(this, "Error", "Failed to create EditProfilePage.");
        return;
    }
    qDebug() << "[MainMenuWindow] EditProfilePage created";

    // ===== مرحله 6: اضافه کردن به StackedWidget =====
    qDebug() << "[MainMenuWindow] Adding widgets to stacked widget...";
    ui->stackedWidget->addWidget(mainMenuWidget);
    ui->stackedWidget->addWidget(gameDetailPage);
    ui->stackedWidget->addWidget(editProfilePage);
    ui->stackedWidget->setCurrentIndex(0);
    qDebug() << "[MainMenuWindow] Widgets added to stacked widget";

    // ===== مرحله 7: اتصال سیگنال‌ها =====
    qDebug() << "[MainMenuWindow] Connecting signals...";
    connect(mainMenuWidget, &MainMenu::gameSelected, this, &MainMenuWindow::onGameSelected);
    connect(mainMenuWidget, &MainMenu::editProfileRequested, this, &MainMenuWindow::onEditProfileRequested);
    connect(mainMenuWidget, &MainMenu::exitRequested, this, &MainMenuWindow::exitRequested);

    connect(gameDetailPage, &GameDetailPage::backToMenu, this, &MainMenuWindow::onBackToMenu);
    connect(gameDetailPage, &GameDetailPage::startNewGame, this, &MainMenuWindow::onStartNewGame);

    connect(editProfilePage, &EditProfilePage::backToMenu, this, &MainMenuWindow::onBackToMenu);
    connect(editProfilePage, &EditProfilePage::profileUpdated, this, [this]() {
        qDebug() << "[MainMenuWindow] Profile updated, refreshing user data...";
        if (currentUser && clientManager) {
            clientManager->sendGetUser(currentUser->username);
        }
    });

    connect(clientManager, &ClientManager::getUserResponse, this,
            [this](bool success, user* userData, const QString& /*message*/) {
                qDebug() << "[MainMenuWindow] getUserResponse received, success:" << success;
                if (success && userData && currentUser) {
                    *currentUser = *userData;
                    delete userData;
                    mainMenuWidget->setUserInfo(currentUser);
                    gameDetailPage->setUser(currentUser);
                }
            });
    qDebug() << "[MainMenuWindow] Signals connected";

    // ===== مرحله 8: تنظیم آیکون =====
    qDebug() << "[MainMenuWindow] Setting window icon...";
    QIcon icon(":/icons/logo.png");
    if (!icon.isNull()) {
        setWindowIcon(icon);
        qDebug() << "[MainMenuWindow] Window icon set";
    } else {
        qDebug() << "[MainMenuWindow] WARNING: Logo icon not found!";
    }

    // ===== مرحله 9: تنظیم اطلاعات کاربر =====
    qDebug() << "[MainMenuWindow] Setting user info...";
    setUserInfo(currentUser);
    qDebug() << "[MainMenuWindow] User info set";

    qDebug() << "[MainMenuWindow] Constructor finished successfully";
}

MainMenuWindow::~MainMenuWindow() {
    qDebug() << "[MainMenuWindow] Destroyed";
    delete ui;
}

void MainMenuWindow::setUserInfo(user* loggedInUser) {
    currentUser = loggedInUser;
    if (mainMenuWidget) {
        mainMenuWidget->setUserInfo(currentUser);
    }
    if (gameDetailPage) {
        gameDetailPage->setUser(currentUser);
    }
    if (editProfilePage) {
        editProfilePage->setUser(currentUser);
    }
}

void MainMenuWindow::showEvent(QShowEvent* event) {
    qDebug() << "[MainMenuWindow] showEvent called";
    QWidget::showEvent(event);
    qDebug() << "[MainMenuWindow] showEvent finished";
}

void MainMenuWindow::paintEvent(QPaintEvent* event) {
    qDebug() << "[MainMenuWindow] paintEvent called";
    QWidget::paintEvent(event);
    qDebug() << "[MainMenuWindow] paintEvent finished";
}

// بقیه توابع بدون تغییر
void MainMenuWindow::onGameSelected(int gameId) {
    qDebug() << "[MainMenuWindow] onGameSelected:" << gameId;
    GameType type;
    switch(gameId) {
    case 0: type = GameType::DotsAndBoxes; break;
    case 1: type = GameType::NineMensMorris; break;
    case 2: type = GameType::Fanorona; break;
    default: return;
    }
    if (gameDetailPage) {
        gameDetailPage->setGameType(type);
        gameDetailPage->loadData();
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainMenuWindow::onBackToMenu() {
    qDebug() << "[MainMenuWindow] onBackToMenu";
    ui->stackedWidget->setCurrentIndex(0);
}

void MainMenuWindow::onStartNewGame(GameType gameType, bool isHost) {
    qDebug() << "[MainMenuWindow] onStartNewGame";
    QMessageBox::information(this, "New Game",
                             QString("Starting game %1 as %2")
                                 .arg(static_cast<int>(gameType))
                                 .arg(isHost ? "Host" : "Guest"));
}

void MainMenuWindow::onEditProfileRequested() {
    qDebug() << "[MainMenuWindow] onEditProfileRequested";
    if (editProfilePage) {
        editProfilePage->loadUserData();
        ui->stackedWidget->setCurrentIndex(2);
    }
}