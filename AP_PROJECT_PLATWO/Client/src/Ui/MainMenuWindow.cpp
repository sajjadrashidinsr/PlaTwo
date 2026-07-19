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

    qDebug() << "[MainMenuWindow] Setting up UI...";
    ui->setupUi(this);
    qDebug() << "[MainMenuWindow] UI setup complete";

    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setAttribute(Qt::WA_StyledBackground, true);
    qDebug() << "[MainMenuWindow] Attributes set";

    qDebug() << "[MainMenuWindow] Creating MainMenu widget...";
    mainMenuWidget = new MainMenu(this);
    if (!mainMenuWidget) {
        qDebug() << "[MainMenuWindow] ERROR: mainMenuWidget is null!";
        QMessageBox::critical(this, "Error", "Failed to create MainMenu widget.");
        return;
    }
    qDebug() << "[MainMenuWindow] MainMenu created";

    qDebug() << "[MainMenuWindow] Creating GameDetailPage...";
    gameDetailPage = new GameDetailPage(currentUser, clientManager, this);
    if (!gameDetailPage) {
        qDebug() << "[MainMenuWindow] ERROR: gameDetailPage is null!";
        QMessageBox::critical(this, "Error", "Failed to create GameDetailPage.");
        return;
    }
    qDebug() << "[MainMenuWindow] GameDetailPage created";

    qDebug() << "[MainMenuWindow] Creating EditProfilePage...";
    editProfilePage = new EditProfilePage(currentUser, clientManager, this);
    if (!editProfilePage) {
        qDebug() << "[MainMenuWindow] ERROR: editProfilePage is null!";
        QMessageBox::critical(this, "Error", "Failed to create EditProfilePage.");
        return;
    }
    qDebug() << "[MainMenuWindow] EditProfilePage created";

    qDebug() << "[MainMenuWindow] Adding widgets to stacked widget...";
    ui->stackedWidget->addWidget(mainMenuWidget);
    ui->stackedWidget->addWidget(gameDetailPage);
    ui->stackedWidget->addWidget(editProfilePage);
    ui->stackedWidget->setCurrentIndex(0);
    qDebug() << "[MainMenuWindow] Widgets added to stacked widget";

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
    
    connect(clientManager, &ClientManager::gameStarted,
            this, &MainMenuWindow::onGameStarted);
    connect(clientManager, &ClientManager::gameStateReceived,
            this, &MainMenuWindow::onGameStateReceived);
    connect(clientManager, &ClientManager::gameOverReceived,
            this, &MainMenuWindow::onGameOverReceived);
    connect(clientManager, &ClientManager::gameAborted,
            this, &MainMenuWindow::onGameAborted);        
    
            
    qDebug() << "[MainMenuWindow] Signals connected";

    qDebug() << "[MainMenuWindow] Setting window icon...";
    QIcon icon(":/icons/logo.png");
    if (!icon.isNull()) {
        setWindowIcon(icon);
        qDebug() << "[MainMenuWindow] Window icon set";
    } else {
        qDebug() << "[MainMenuWindow] WARNING: Logo icon not found!";
    }

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

void MainMenuWindow::onStartNewGame(GameType gameType, bool isHost)
{
    if (gameType == GameType::DotsAndBoxes) {
        m_isHost = isHost;
        m_currentRoom.port = 1234;
        m_currentRoom.gameSettings.boardSize = 5;
        m_currentRoom.hostUsername = currentUser ? currentUser->username : "Host";
        m_currentRoom.guestUsername = "Guest";

        showGameWidget();
    }
}

void MainMenuWindow::onEditProfileRequested() {
    qDebug() << "[MainMenuWindow] onEditProfileRequested";
    if (editProfilePage) {
        editProfilePage->loadUserData();
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void MainMenuWindow::showGameWidget()
{
    if (!m_gameWidget) {
        m_gameWidget = new GameWidget(this);
        m_gameWidget->setClientManager(clientManager);

        connect(m_gameWidget, &GameWidget::leaveGame,
                this, &MainMenuWindow::onLeaveGame);
        connect(m_gameWidget, &GameWidget::moveMade,
                this, [this](const QPoint& p1, const QPoint& p2) {
                    QJsonObject moveData;
                    moveData["port"] = static_cast<int>(m_currentRoom.port);
                    moveData["x1"] = p1.x();
                    moveData["y1"] = p1.y();
                    moveData["x2"] = p2.x();
                    moveData["y2"] = p2.y();
                    moveData["playerId"] = m_isHost ? 0 : 1;
                    clientManager->sendGameMove(moveData);
                });

        ui->stackedWidget->addWidget(m_gameWidget);
    }

    m_gameWidget->setRoomInfo(m_currentRoom.port, m_isHost);

    if (m_isHost) {
        int boardSize = m_currentRoom.gameSettings.boardSize;
        clientManager->sendGameStart(m_currentRoom.port, boardSize);
        m_gameWidget->startGame(boardSize);
    }

    ui->stackedWidget->setCurrentWidget(m_gameWidget);
}

void MainMenuWindow::onGameStarted(const QJsonObject& data)
{
    int boardSize = data["boardSize"].toInt();

    if (!m_isHost && m_gameWidget) {
        m_gameWidget->startGame(boardSize);
    }

    if (m_gameWidget) {
        QString hostName = data["hostUsername"].toString();
        QString guestName = data["guestUsername"].toString();
        m_gameWidget->setPlayers(hostName, guestName);
    }
}

void MainMenuWindow::onGameStateReceived(const QJsonObject& state)
{
    if (m_gameWidget) {
        m_gameWidget->onGameStateReceived(state);
    }
}

void MainMenuWindow::onGameOverReceived(const QJsonObject& data)
{
    if (m_gameWidget) {
        m_gameWidget->onGameOverReceived(data);
    }
}

void MainMenuWindow::onGameAborted(const QString& message)
{
    QMessageBox::information(this, "Game Aborted", message);
    onLeaveGame();
}

void MainMenuWindow::onLeaveGame()
{
    if (m_gameWidget) {
        ui->stackedWidget->removeWidget(m_gameWidget);
        m_gameWidget->deleteLater();
        m_gameWidget = nullptr;
    }
    onBackToMenu();
}