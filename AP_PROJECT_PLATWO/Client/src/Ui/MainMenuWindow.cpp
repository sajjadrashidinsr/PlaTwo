#include "mainmenuwindow.h"
#include "ui_mainmenuwindow.h"
#include "client_manager.h"
#include "Dialogs/hostgamedialog.h"
#include "Dialogs/joingamedialog.h"
#include "Dialogs/waitingroomdialog.h"
#include "GameWidget/GameWidget.h"
#include <QMessageBox>
#include <QDebug>

MainMenuWindow::MainMenuWindow(user* loggedInUser, ClientManager* client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainMenuWindow)
    , currentUser(loggedInUser)
    , clientManager(client)
    , mainMenuWidget(nullptr)
    , gameDetailPage(nullptr)
    , editProfilePage(nullptr)
    , m_gameWidget(nullptr)
    , m_isHost(false)
{
    qDebug() << "[MainMenuWindow] ===== CONSTRUCTOR STARTED =====";

    if (!clientManager) {
        qDebug() << "[MainMenuWindow] ERROR: clientManager is null!";
        return;
    }

    ui->setupUi(this);
    qDebug() << "[MainMenuWindow] ui->setupUi done";

    this->setAttribute(Qt::WA_DeleteOnClose, true);
    qDebug() << "[MainMenuWindow] setAttribute done";

    qDebug() << "[MainMenuWindow] Creating MainMenu...";
    mainMenuWidget = new MainMenu(this);
    qDebug() << "[MainMenuWindow] MainMenu created";

    qDebug() << "[MainMenuWindow] Creating GameDetailPage...";
    gameDetailPage = new GameDetailPage(currentUser, clientManager, this);
    qDebug() << "[MainMenuWindow] GameDetailPage created";

    qDebug() << "[MainMenuWindow] Creating EditProfilePage...";
    editProfilePage = new EditProfilePage(currentUser, clientManager, this);
    qDebug() << "[MainMenuWindow] EditProfilePage created";

    qDebug() << "[MainMenuWindow] Adding widgets to stackedWidget...";
    ui->stackedWidget->addWidget(mainMenuWidget);
    ui->stackedWidget->addWidget(gameDetailPage);
    ui->stackedWidget->addWidget(editProfilePage);
    ui->stackedWidget->setCurrentIndex(0);
    qDebug() << "[MainMenuWindow] Widgets added";

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
            [this](bool success, std::shared_ptr<user> userData, const QString&) {
                qDebug() << "[MainMenuWindow] getUserResponse received, success:" << success;

                if (success && userData && currentUser) {
                    qDebug() << "[MainMenuWindow] Updating user data, history size:" << userData->history.size();
                    *currentUser = *userData;
                    // ⚠️ DO NOT delete userData - shared_ptr auto-deletes
                    if (mainMenuWidget) mainMenuWidget->setUserInfo(currentUser);
                    if (gameDetailPage) {
                        gameDetailPage->loadHistory();
                    }
                } else {
                    qDebug() << "[MainMenuWindow] getUserResponse failed or invalid data";
                    // shared_ptr will auto-delete, no manual delete needed
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

    setWindowIcon(QIcon(":/icons/logo.png"));

    if (mainMenuWidget) mainMenuWidget->setUserInfo(currentUser);

    qDebug() << "[MainMenuWindow] ===== CONSTRUCTOR FINISHED SUCCESSFULLY =====";
}

MainMenuWindow::~MainMenuWindow() {
    qDebug() << "[MainMenuWindow] Destroyed";
    delete ui;
}


void MainMenuWindow::setUserInfo(user* loggedInUser) {
    currentUser = loggedInUser;
    if (mainMenuWidget) mainMenuWidget->setUserInfo(currentUser);
}

void MainMenuWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
}

void MainMenuWindow::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
}

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
        ui->stackedWidget->setCurrentIndex(1);
        gameDetailPage->loadData();
    }
}

void MainMenuWindow::onBackToMenu() {
    qDebug() << "[MainMenuWindow] onBackToMenu";
    ui->stackedWidget->setCurrentIndex(0);
}

void MainMenuWindow::onStartNewGame(GameType gameType, bool isHost)
{
    qDebug() << "[MainMenuWindow] onStartNewGame called, gameType:" << static_cast<int>(gameType) << "isHost:" << isHost;

    if (gameType != GameType::DotsAndBoxes) {
        return;
    }

    m_isHost = isHost;

    if (isHost) {
        HostGameDialog hostDlg(this);
        hostDlg.setClientManager(clientManager);

        connect(&hostDlg, &HostGameDialog::roomCreationSuccess,
                this, [this](const Room& room) {
                    m_currentRoom = room;
                    qDebug() << "[MainMenuWindow] Room created with boardSize:"
                             << m_currentRoom.gameSettings.boardSize;

                    m_waitingDialog = new WaitingRoomDialog(true, m_currentRoom, this);
                    m_waitingDialog->setAttribute(Qt::WA_DeleteOnClose);
                    m_waitingDialog->setClientManager(clientManager);

                    connect(m_waitingDialog, &WaitingRoomDialog::startGame, this, [this](int boardSize) {
                        qDebug() << "[MainMenuWindow] Start game with boardSize:" << boardSize;
                        showGameWidget(boardSize);
                    });

                    connect(m_waitingDialog, &WaitingRoomDialog::leaveRoom, this, &MainMenuWindow::onBackToMenu);
                    m_waitingDialog->show();
                });

        connect(&hostDlg, &HostGameDialog::roomCreationFailed,
                this, [](const QString& error) {
                    qDebug() << "Room creation failed:" << error;
                });

        hostDlg.exec();
    } else {
        JoinGameDialog joinDlg(this);
        joinDlg.setClientManager(clientManager);

        connect(&joinDlg, &JoinGameDialog::joinSuccess,
                this, [this](const Room& room) {
                    m_currentRoom = room;
                    qDebug() << "[MainMenuWindow] Joined room with boardSize:"
                             << m_currentRoom.gameSettings.boardSize;

                    m_waitingDialog = new WaitingRoomDialog(false, m_currentRoom, this);
                    m_waitingDialog->setAttribute(Qt::WA_DeleteOnClose);
                    m_waitingDialog->setClientManager(clientManager);

                    connect(m_waitingDialog, &WaitingRoomDialog::startGame, this, [this](int boardSize) {
                        qDebug() << "[MainMenuWindow] Start game with boardSize:" << boardSize;
                        showGameWidget(boardSize);
                    });

                    connect(m_waitingDialog, &WaitingRoomDialog::leaveRoom, this, &MainMenuWindow::onBackToMenu);
                    m_waitingDialog->show();
                });

        connect(&joinDlg, &JoinGameDialog::joinFailed,
                this, [](const QString& error) {
                    qDebug() << "Join failed:" << error;
                });

        joinDlg.exec();
    }
}

void MainMenuWindow::onEditProfileRequested() {
    qDebug() << "[MainMenuWindow] onEditProfileRequested";
    if (editProfilePage) {
        editProfilePage->loadUserData();
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void MainMenuWindow::onGameStarted(const QJsonObject& data)
{
    qDebug() << "[MainMenuWindow] onGameStarted called!";
    int boardSize = data["boardSize"].toInt(5);
    quint16 port = static_cast<quint16>(data["port"].toInt(1234));
    qDebug() << "[MainMenuWindow] Board size from server:" << boardSize;
    qDebug() << "[MainMenuWindow] Port from server:" << port;

    bool isHost = data["isHost"].toBool(false);
    if (!data.contains("isHost")) {
        QString hostName = data["hostUsername"].toString();
        isHost = (currentUser && hostName == currentUser->username);
    }
    m_isHost = isHost;
    qDebug() << "[MainMenuWindow] isHost:" << m_isHost;

    if (m_currentRoom.port != port) {
        qDebug() << "[MainMenuWindow] Updating room port from" << m_currentRoom.port << "to" << port;
        m_currentRoom.port = port;
    }

    if (!m_gameWidget) {
        m_gameWidget = new GameWidget(this);
        m_gameWidget->setClientManager(clientManager);
        connect(m_gameWidget, &GameWidget::leaveGame, this, &MainMenuWindow::onLeaveGame);
        ui->stackedWidget->addWidget(m_gameWidget);
    }

    m_gameWidget->setRoomInfo(m_currentRoom.port, m_isHost);
    m_gameWidget->startGame(boardSize);

    QString hostName = data["hostUsername"].toString();
    QString guestName = data["guestUsername"].toString();
    if (!hostName.isEmpty() && !guestName.isEmpty()) {
        m_gameWidget->setPlayers(hostName, guestName);
    }

    ui->stackedWidget->setCurrentWidget(m_gameWidget);
}

void MainMenuWindow::showGameWidget(int boardSize)
{
    qDebug() << "[MainMenuWindow] showGameWidget with boardSize:" << boardSize;
    if (!m_gameWidget) {
        m_gameWidget = new GameWidget(this);
        m_gameWidget->setClientManager(clientManager);
        connect(m_gameWidget, &GameWidget::leaveGame, this, &MainMenuWindow::onLeaveGame);
        ui->stackedWidget->addWidget(m_gameWidget);
    }
    m_gameWidget->setRoomInfo(m_currentRoom.port, m_isHost);
    m_gameWidget->startGame(boardSize);
    ui->stackedWidget->setCurrentWidget(m_gameWidget);
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