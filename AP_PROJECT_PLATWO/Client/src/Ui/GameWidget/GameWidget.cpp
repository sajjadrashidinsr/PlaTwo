#include "GameWidget.h"
#include "ui_GameWidget.h"
#include "client_manager.h"
#include <QMessageBox>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , m_gameEnded(false)
    , m_gameOver(false)
{
    ui->setupUi(this);

    m_gameBoard = ui->gameBoard;

    connect(m_gameBoard, &GameBoard::lineClicked, this, &GameWidget::onLineClicked);
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
    m_gameEnded = false;
    m_gameOver = false;
    m_player1Score = 0;
    m_player2Score = 0;

    m_gameBoard->setBoardSize(boardSize);
    m_gameBoard->clearBoard();

    ui->statusLabel->setText("Game started!");
    ui->turnLabel->setText("Waiting for server...");
    ui->player1ScoreLabel->setText("0");
    ui->player2ScoreLabel->setText("0");
}

void GameWidget::setPlayers(const QString& player1Name, const QString& player2Name)
{
    ui->player1NameLabel->setText(player1Name + ":");
    ui->player2NameLabel->setText(player2Name + ":");
}

void GameWidget::onLineClicked(const QPoint& p1, const QPoint& p2)
{
    if (m_gameEnded || m_gameOver) return;

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

void GameWidget::onGameStateReceived(const QJsonObject& data)
{
    QJsonObject state = data["state"].toObject();

    m_player1Score = state["player1Score"].toInt();
    m_player2Score = state["player2Score"].toInt();
    m_currentPlayer = state["currentPlayer"].toInt();
    m_gameOver = state["gameOver"].toBool();

    // به‌روزرسانی برد
    m_gameBoard->updateBoard(state);

    // به‌روزرسانی UI
    updateUI();
}

void GameWidget::onGameOverReceived(const QJsonObject& data)
{
    m_gameEnded = true;
    m_gameOver = true;

    int winner = data["winner"].toInt();

    ui->turnLabel->setText("Game Over!");

    if (winner == -1) {
        ui->statusLabel->setText("It's a Draw!");
    } else {
        QString winnerName = (winner == 0) ?
                                 ui->player1NameLabel->text().replace(":", "") :
                                 ui->player2NameLabel->text().replace(":", "");
        ui->statusLabel->setText(QString("%1 wins!").arg(winnerName));
    }

    ui->player1ScoreLabel->setText(QString::number(m_player1Score));
    ui->player2ScoreLabel->setText(QString::number(m_player2Score));
}

void GameWidget::onGameAborted()
{
    QMessageBox::information(this, "Game Aborted", "The other player has left the game.");
    emit leaveGame();
}

void GameWidget::updateUI()
{
    ui->player1ScoreLabel->setText(QString::number(m_player1Score));
    ui->player2ScoreLabel->setText(QString::number(m_player2Score));

    if (!m_gameOver) {
        QString playerName = (m_currentPlayer == 0) ?
                                 ui->player1NameLabel->text().replace(":", "") :
                                 ui->player2NameLabel->text().replace(":", "");
        ui->turnLabel->setText(QString("Turn: %1").arg(playerName));
    }
}

void GameWidget::onBackClicked()
{
    if (!m_gameEnded && !m_gameOver) {
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