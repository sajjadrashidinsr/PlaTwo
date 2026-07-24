#include "GameWidget.h"
#include "ui_GameWidget.h"
#include "client_manager.h"
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , m_gameEnded(false)
    , m_gameOver(false)
{
    ui->setupUi(this);
    m_gameBoard = ui->gameBoard;
    if (m_gameBoard) {
        connect(m_gameBoard, &GameBoard::lineClicked, this, &GameWidget::onLineClicked);
    }
    connect(ui->backButton, &QPushButton::clicked, this, &GameWidget::onBackClicked);

    setupGameOverOverlay();
}

GameWidget::~GameWidget()
{
    delete ui;
}

void GameWidget::setupGameOverOverlay()
{
    m_gameOverOverlay = new QWidget(this);
    m_gameOverOverlay->setGeometry(0, 0, width(), height());
    m_gameOverOverlay->setStyleSheet(
        "background-color: rgba(0, 0, 0, 150);"
        "border-radius: 10px;"
        );
    m_gameOverOverlay->hide();

    QVBoxLayout* mainLayout = new QVBoxLayout(m_gameOverOverlay);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(20);

    m_gameOverTitle = new QLabel("🎮 Game Over", m_gameOverOverlay);
    m_gameOverTitle->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #FFD700;"
        "background-color: transparent;"
        );
    m_gameOverTitle->setAlignment(Qt::AlignCenter);

    m_gameOverMessage = new QLabel("", m_gameOverOverlay);
    m_gameOverMessage->setStyleSheet(
        "font-size: 24px;"
        "font-weight: bold;"
        "color: white;"
        "background-color: transparent;"
        );
    m_gameOverMessage->setAlignment(Qt::AlignCenter);

    m_returnToMenuButton = new QPushButton("🏠 Return to Menu", m_gameOverOverlay);
    m_returnToMenuButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   padding: 15px 40px;"
        "   border-radius: 10px;"
        "   border: 2px solid #45a049;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d8b40;"
        "}"
        );
    m_returnToMenuButton->setCursor(Qt::PointingHandCursor);
    connect(m_returnToMenuButton, &QPushButton::clicked, this, &GameWidget::onReturnToMenuClicked);

    qDebug() << "[GameWidget] Return to Menu button connected!";

    mainLayout->addWidget(m_gameOverTitle);
    mainLayout->addWidget(m_gameOverMessage);
    mainLayout->addWidget(m_returnToMenuButton);

    m_gameOverOverlay->raise();
}

void GameWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_gameOverOverlay) {
        m_gameOverOverlay->setGeometry(0, 0, width(), height());
    }
}

void GameWidget::showGameOverDialog(const QString& winnerName, int winnerId)
{
    qDebug() << "[GameWidget] showGameOverDialog called!";
    qDebug() << "[GameWidget] winnerName:" << winnerName << "winnerId:" << winnerId;

    if (!m_gameOverOverlay) {
        qDebug() << "[GameWidget] ERROR: m_gameOverOverlay is null!";
        return;
    }

    QString message;
    if (winnerId == -1) {
        message = "🤝 It's a Draw!";
    } else {
        QString color = (winnerId == 0) ? "#2196F3" : "#F44336";
        message = QString(
                      "🏆 <span style='color: %1; font-weight: bold;'>%2</span> wins the game!<br>"
                      "🎉 Congratulations!"
                      ).arg(color, winnerName);
    }

    m_gameOverMessage->setText(message);
    m_gameOverMessage->setTextFormat(Qt::RichText);

    m_gameOverOverlay->show();
    m_gameOverOverlay->raise();
    qDebug() << "[GameWidget] Overlay shown!";

    ui->backButton->setEnabled(false);
    qDebug() << "[GameWidget] showGameOverDialog finished!";
}

void GameWidget::hideGameOverDialog()
{
    if (m_gameOverOverlay) {
        m_gameOverOverlay->hide();
    }
    ui->backButton->setEnabled(true);
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
    qDebug() << "[GameWidget] setRoomInfo - port:" << port << "isHost:" << isHost << "playerId:" << m_playerId;
}

void GameWidget::startGame(int boardSize)
{
    m_gameEnded = false;
    m_gameOver = false;
    m_player1Score = 0;
    m_player2Score = 0;
    if (m_gameBoard) {
        m_gameBoard->setBoardSize(boardSize);
        m_gameBoard->clearBoard();
    }
    ui->statusLabel->setText("Game started. In progress...");
    ui->player1ScoreLabel->setText("0");
    ui->player2ScoreLabel->setText("0");
}

void GameWidget::setPlayers(const QString& player1Name, const QString& player2Name)
{
    m_player1Name = player1Name;
    m_player2Name = player2Name;
    ui->player1NameLabel->setText(player1Name + ":");
    ui->player2NameLabel->setText(player2Name + ":");
}

void GameWidget::onLineClicked(const QPoint& p1, const QPoint& p2)
{
    qDebug() << "[GameWidget] ===== LINE CLICKED =====";
    qDebug() << "[GameWidget] p1:" << p1 << "p2:" << p2;
    qDebug() << "[GameWidget] m_gameEnded:" << m_gameEnded << "m_gameOver:" << m_gameOver;
    qDebug() << "[GameWidget] m_playerId:" << m_playerId << "m_isHost:" << m_isHost;

    if (m_gameEnded || m_gameOver) {
        qDebug() << "[GameWidget]  Game ended or over";
        return;
    }

    QJsonObject moveData;
    moveData["port"] = static_cast<int>(m_roomPort);
    moveData["x1"] = p1.x();
    moveData["y1"] = p1.y();
    moveData["x2"] = p2.x();
    moveData["y2"] = p2.y();
    moveData["playerId"] = m_playerId;

    qDebug() << "[GameWidget] Sending moveData:" << moveData;

    if (m_clientManager) {
        m_clientManager->sendGameMove(moveData);
        qDebug() << "[GameWidget]  Move sent!";
    } else {
        qDebug() << "[GameWidget]  m_clientManager is null!";
    }
}

void GameWidget::onGameStateReceived(const QJsonObject& data)
{
    QJsonObject state = data["state"].toObject();
    m_player1Score = state["player1Score"].toInt();
    m_player2Score = state["player2Score"].toInt();
    m_currentPlayer = state["currentPlayer"].toInt();
    m_gameOver = state["gameOver"].toBool();

    if (state.contains("boardSize")) {
        int sz = state["boardSize"].toInt();
        if (sz >= 3 && sz <= 10 && m_gameBoard) {
            m_gameBoard->setBoardSize(sz);
        }
    }

    if (m_gameBoard) {
        m_gameBoard->updateBoard(state);
    }
    updateUI();
    ui->statusLabel->setText("Game in progress...");
}

void GameWidget::onGameOverReceived(const QJsonObject& data)
{
    qDebug() << "========================================";
    qDebug() << "[GameWidget] ===== GAME OVER RECEIVED =====";
    qDebug() << "[GameWidget] Full data:" << data;

    m_gameEnded = true;
    m_gameOver = true;

    int winner = data["winner"].toInt();
    int p1Score = data["player1Score"].toInt();
    int p2Score = data["player2Score"].toInt();

    qDebug() << "[GameWidget] winner:" << winner;
    qDebug() << "[GameWidget] p1Score:" << p1Score << "p2Score:" << p2Score;

    m_player1Score = p1Score;
    m_player2Score = p2Score;

    ui->player1ScoreLabel->setText(QString::number(m_player1Score));
    ui->player2ScoreLabel->setText(QString::number(m_player2Score));
    ui->turnLabel->setText("Game Over!");

    QString winnerName;
    if (winner == -1) {
        qDebug() << "[GameWidget] It's a Draw!";
        ui->statusLabel->setText("🤝 It's a Draw!");
        showGameOverDialog("", -1);
    } else {
        winnerName = (winner == 0) ? m_player1Name : m_player2Name;
        if (winnerName.isEmpty()) {
            winnerName = (winner == 0) ? "Player 1" : "Player 2";
        }
        qDebug() << "[GameWidget] Winner name:" << winnerName;
        ui->statusLabel->setText(QString("🏆 %1 wins!").arg(winnerName));
        showGameOverDialog(winnerName, winner);
    }

    qDebug() << "[GameWidget] ===== GAME OVER END =====";
    qDebug() << "========================================";
}

void GameWidget::onReturnToMenuClicked()
{
    hideGameOverDialog();

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

void GameWidget::onGameAborted()
{
    hideGameOverDialog();
    QMessageBox::information(this, "Game Aborted", "The other player has left the game.");
    emit leaveGame();
}

void GameWidget::updateUI()
{
    ui->player1ScoreLabel->setText(QString::number(m_player1Score));
    ui->player2ScoreLabel->setText(QString::number(m_player2Score));
    if (!m_gameOver) {
        QString p1 = ui->player1NameLabel->text().replace(":", "").trimmed();
        QString p2 = ui->player2NameLabel->text().replace(":", "").trimmed();
        if (p1.isEmpty()) p1 = "Player 1";
        if (p2.isEmpty()) p2 = "Player 2";

        QString currentName = (m_currentPlayer == 0) ? p1 : p2;
        ui->turnLabel->setText(QString("Turn: %1").arg(currentName));
    }
}

void GameWidget::onBackClicked()
{
    if (m_gameOverOverlay && m_gameOverOverlay->isVisible()) {
        return;
    }

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