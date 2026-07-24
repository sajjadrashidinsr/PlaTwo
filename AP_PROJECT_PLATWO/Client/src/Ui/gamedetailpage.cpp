#include "gamedetailpage.h"
#include "ui_gamedetailpage.h"
#include "client_manager.h"
#include "Dialogs/hostjoinselectiondialog.h"
#include "Dialogs/hostgamedialog.h"
#include "Dialogs/joingamedialog.h"
#include "Dialogs/waitingroomdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QHeaderView>

GameDetailPage::GameDetailPage(user* currentUser, QPointer<ClientManager> client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameDetailPage)
    , currentUser(currentUser)
    , clientManager(client)
    , currentGameType(GameType::DotsAndBoxes)
{
    qDebug() << "[GameDetailPage] Constructor started";
    ui->setupUi(this);

    if (ui->historyTable) {
        ui->historyTable->horizontalHeader()->setStretchLastSection(true);
        ui->historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    }

    connect(ui->backButton, &QPushButton::clicked, this, &GameDetailPage::onBackClicked);
    connect(ui->startNewGameButton, &QPushButton::clicked, this, &GameDetailPage::onStartNewGameClicked);

    if (clientManager) {
        connect(clientManager, &ClientManager::getUserResponse, this, &GameDetailPage::onGetUserResponse);
    }

    qDebug() << "[GameDetailPage] Constructor finished";
}

GameDetailPage::~GameDetailPage()
{
    delete ui;
}

void GameDetailPage::setUser(user* u)
{
    qDebug() << "[GameDetailPage] setUser called";
    currentUser = u;
    if (currentUser) {
        loadHistory();
    }
}

void GameDetailPage::setGameType(GameType type)
{
    qDebug() << "[GameDetailPage] setGameType called, type:" << static_cast<int>(type);
    if (!ui) return;
    currentGameType = type;
    QString gameName;
    switch(type) {
    case GameType::DotsAndBoxes: gameName = "Dots and Boxes"; break;
    case GameType::NineMensMorris: gameName = "Nine Men's Morris"; break;
    case GameType::Fanorona: gameName = "Fanorona"; break;
    default: gameName = "Unknown"; break;
    }
    if (ui->titleLabel) {
        ui->titleLabel->setText(gameName + " - Details");
    }
}

void GameDetailPage::loadData()
{
    qDebug() << "[GameDetailPage] loadData called";
    if (!ui || !currentUser || !clientManager) {
        qDebug() << "[GameDetailPage] loadData: ui, currentUser or clientManager is null!";
        return;
    }
    if (!clientManager->isConnected()) {
        qDebug() << "[GameDetailPage] loadData: not connected to server";
        return;
    }
    clientManager->sendGetUser(currentUser->username);
}

void GameDetailPage::onGetUserResponse(bool success, std::shared_ptr<user> userData, const QString& message)
{
    qDebug() << "[GameDetailPage] onGetUserResponse called, success:" << success;

    if (!success || !userData) {
        qDebug() << "[GameDetailPage] Failed to get user data:" << message;
        return;
    }

    if (currentUser) {
        *currentUser = *userData;  // Copy data, shared_ptr will auto-delete
    }
    loadHistory();
}

void GameDetailPage::loadHistory()
{
    qDebug() << "[GameDetailPage] loadHistory called";

    if (!ui || !currentUser || !ui->historyTable) {
        qDebug() << "[GameDetailPage] loadHistory: ui, currentUser or historyTable is null!";
        return;
    }

    int score = 0;
    switch(currentGameType) {
    case GameType::DotsAndBoxes:
        score = currentUser->dotsAndBoxesScore;
        break;
    case GameType::NineMensMorris:
        score = currentUser->nineMensMorrisScore;
        break;
    case GameType::Fanorona:
        score = currentUser->fanoronaScore;
        break;
    default:
        score = 0;
        break;
    }
    ui->scoreLabel->setText("Score: " + QString::number(score));

    ui->historyTable->clearContents();
    ui->historyTable->setRowCount(0);

    const QList<GameRecord>& history = currentUser->history;
    ui->historyTable->setRowCount(history.size());

    for (int i = 0; i < history.size(); ++i) {
        const GameRecord& rec = history.at(i);
        ui->historyTable->setItem(i, 0, new QTableWidgetItem(rec.opponentUsername));
        ui->historyTable->setItem(i, 1, new QTableWidgetItem(rec.date));
        ui->historyTable->setItem(i, 2, new QTableWidgetItem(rec.role));

        QTableWidgetItem* winnerItem = new QTableWidgetItem(rec.winner);
        if (rec.winner == currentUser->username) {
            winnerItem->setBackground(QColor(76, 175, 80, 100));
            winnerItem->setForeground(QColor(0, 100, 0));
        } else if (rec.winner == "Draw") {
            winnerItem->setBackground(QColor(255, 193, 7, 100));
            winnerItem->setForeground(QColor(150, 100, 0));
        } else {
            winnerItem->setBackground(QColor(244, 67, 54, 100));
            winnerItem->setForeground(QColor(150, 0, 0));
        }
        ui->historyTable->setItem(i, 3, winnerItem);
        ui->historyTable->setItem(i, 4, new QTableWidgetItem(QString::number(rec.finalScore)));
    }

    ui->historyTable->resizeColumnsToContents();
    qDebug() << "[GameDetailPage] loadHistory finished, rows:" << history.size();
}

void GameDetailPage::onBackClicked()
{
    qDebug() << "[GameDetailPage] onBackClicked";
    emit backToMenu();
}

void GameDetailPage::onStartNewGameClicked()
{
    qDebug() << "[GameDetailPage] onStartNewGameClicked";

    if (!clientManager) {
        QMessageBox::critical(this, "Error", "Client manager not available.");
        return;
    }

    if (!clientManager->isConnected()) {
        QMessageBox::warning(this, "Connection Error",
                             "Not connected to server. Please check the server is running.");
        return;
    }

    HostJoinSelectionDialog selection(this);
    int res = selection.exec();

    if (res == QDialog::Accepted) {
        bool isHost = selection.isHostSelected();
        emit startNewGame(currentGameType, isHost);
    }
}