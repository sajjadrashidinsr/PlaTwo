#include "GameWidget.h"
#include "ui_GameWidget.h"
#include "client_manager.h"
#include <QMessageBox>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , m_gameController(nullptr)
{
    ui->setupUi(this);

    m_gameBoard = ui->gameBoard;

    connect(m_gameBoard, &GameBoard::moveMade, this, &GameWidget::onMoveMade);
    connect(ui->backButton, &QPushButton::clicked, this, &GameWidget::onBackClicked);
}

GameWidget::~GameWidget()
{
    delete ui;
}

void GameWidget::setClientManager(ClientManager* client)
{
    m_clientManager = client;
}

void GameWidget::setRoomInfo(quint16 port, bool isHost)
{
    m_roomPort = port;
    m_isHost = isHost;
    m_playerId = isHost ? 0 : 1;
}

void GameWidget::startGame(int boardSize)
{
    if (m_gameController) {
        m_gameController->deleteLater();
    }

    m_gameController = new GameController(boardSize, this);
    m_gameBoard->setController(m_gameController);
    m_gameEnded = false;

    connect(m_gameController, &GameController::gameStateChanged,
            this, &GameWidget::onGameStateChanged);

    ui->statusLabel->setText("Game started!");
    onGameStateChanged();
}

void GameWidget::setPlayers(const QString& player1Name, const QString& player2Name)
{
    if (m_gameController) {
        m_gameController->setPlayerNames(player1Name, player2Name);
        updateUI();
    }
}

void GameWidget::onMoveMade(const QPoint& p1, const QPoint& p2)
{
    if (!m_gameController || m_gameEnded) return;
    if (m_gameController->isGameOver()) return;

    QJsonObject moveData;
    moveData["port"] = static_cast<int>(m_roomPort);
    moveData["x1"] = p1.x();
    moveData["y1"] = p1.y();
    moveData["x2"] = p2.x();
    moveData["y2"] = p2.y();
    moveData["playerId"] = m_playerId;

    if (m_clientManager) {
        m_clientManager->sendGameMove(moveData);
    }
}

void GameWidget::onGameStateReceived(const QJsonObject& state)
{
    if (!m_gameController) return;

    QJsonObject boardState = state["state"].toObject();

    if (boardState.contains("player1Score") && boardState.contains("player2Score")) {
        int p1Score = boardState["player1Score"].toInt();
        int p2Score = boardState["player2Score"].toInt();

        Player* p1 = const_cast<Player*>(m_gameController->getPlayer(0));
        Player* p2 = const_cast<Player*>(m_gameController->getPlayer(1));

        if (p1) {
            p1->resetScore();
            p1->addScore(p1Score);
        }
        if (p2) {
            p2->resetScore();
            p2->addScore(p2Score);
        }
    }

    if (boardState.contains("gameOver") && boardState["gameOver"].toBool()) {
        if (!m_gameEnded) {
            m_gameEnded = true;
            int winner = boardState.contains("winner") ? boardState["winner"].toInt() : -1;
            if (winner != -1) {
                ui->turnLabel->setText("Game Over!");
                QString winnerName = (winner == 0) ?
                    m_gameController->getPlayer(0)->getName() :
                    m_gameController->getPlayer(1)->getName();
                ui->statusLabel->setText(QString("%1 wins!").arg(winnerName));
            }
        }
    } else if (boardState.contains("currentPlayer")) {
        int currentPlayer = boardState["currentPlayer"].toInt();
        if (m_gameController) {
            // Update current player state
        }
    }

    updateUI();
    m_gameBoard->update();
}

void GameWidget::onGameOverReceived(const QJsonObject& data)
{
    if (m_gameEnded) return;
    m_gameEnded = true;

    int winner = data["winner"].toInt();
    int p1Score = data["player1Score"].toInt();
    int p2Score = data["player2Score"].toInt();

    ui->turnLabel->setText("Game Over!");

    if (winner == -1) {
        ui->statusLabel->setText("It's a Draw!");
    } else {
        QString winnerName = (winner == 0) ?
            m_gameController->getPlayer(0)->getName() :
            m_gameController->getPlayer(1)->getName();
        ui->statusLabel->setText(QString("%1 wins!").arg(winnerName));
    }

    updateUI();
}

void GameWidget::onGameAborted()
{
    QMessageBox::information(this, "Game Aborted", "The other player has left the game.");
    emit leaveGame();
}

void GameWidget::onGameStateChanged()
{
    updateUI();
}

void GameWidget::updateUI()
{
    if (!m_gameController) return;

    const Player* p1 = m_gameController->getPlayer(0);
    const Player* p2 = m_gameController->getPlayer(1);

    if (p1) {
        ui->player1NameLabel->setText(p1->getName() + ":");
        ui->player1ScoreLabel->setText(QString::number(p1->getScore()));
    }

    if (p2) {
        ui->player2NameLabel->setText(p2->getName() + ":");
        ui->player2ScoreLabel->setText(QString::number(p2->getScore()));
    }

    if (!m_gameController->isGameOver()) {
        const Player* current = m_gameController->getCurrentPlayer();
        if (current) {
            ui->turnLabel->setText(QString("Turn: %1").arg(current->getName()));
        }
    }

    m_gameBoard->update();
}

void GameWidget::onBackClicked()
{
    if (!m_gameEnded) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Leave Game",
            "Are you sure you want to leave the game?",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply != QMessageBox::Yes) {
            return;
        }

        if (m_clientManager) {
            m_clientManager->sendGameAbort(m_roomPort);
        }
    }

    emit leaveGame();
}