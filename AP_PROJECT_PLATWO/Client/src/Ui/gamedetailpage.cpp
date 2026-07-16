#include "gamedetailpage.h"
#include "ui_gamedetailpage.h"
#include "client_manager.h"
#include <QInputDialog>

GameDetailPage::GameDetailPage(user* currentUser, ClientManager* client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameDetailPage)
    , currentUser(currentUser)
    , clientManager(client)
{
    ui->setupUi(this);

    connect(ui->backButton, &QPushButton::clicked, this, &GameDetailPage::onBackClicked);

    connect(ui->startNewGameButton, &QPushButton::clicked, this, &GameDetailPage::onStartNewGameClicked);

    connect(clientManager, &ClientManager::getUserResponse,
            this, &GameDetailPage::onGetUserResponse);

}

GameDetailPage::~GameDetailPage()
{
    delete ui;
}

void GameDetailPage::setUser(user* u)
{
    currentUser = u;
    if (currentUser) {
        loadData();
    }
}

void GameDetailPage::setGameType(GameType type)
{
    currentGameType = type;
    QString gameName;
    switch(type) {
    case GameType::DotsAndBoxes: gameName = "Dots and Boxes"; break;
    case GameType::NineMensMorris: gameName = "Nine Men's Morris"; break;
    case GameType::Fanorona: gameName = "Fanorona"; break;
    }
    ui->titleLabel->setText(gameName + " - Details");
}

void GameDetailPage::loadData()
{
    if (!currentUser || !clientManager) return;

    clientManager->sendGetUser(currentUser->username);

}

void GameDetailPage::onGetUserResponse(bool success, user* userData, const QString& message) {
    if (!success || !userData) {
        return;
    }

    int score = 0;
    switch(currentGameType) {
    case GameType::DotsAndBoxes: score = userData->dotsAndBoxesScore; break;
    case GameType::NineMensMorris: score = userData->nineMensMorrisScore; break;
    case GameType::Fanorona: score = userData->fanoronaScore; break;
    }
    ui->scoreLabel->setText("Score: " + QString::number(score));

    const QList<GameRecord>& history = userData->history;
    ui->historyTable->setRowCount(history.size());
    for (int i = 0; i < history.size(); ++i) {
        const GameRecord& rec = history.at(i);
        ui->historyTable->setItem(i, 0, new QTableWidgetItem(rec.opponentUsername));
        ui->historyTable->setItem(i, 1, new QTableWidgetItem(rec.date));
        ui->historyTable->setItem(i, 2, new QTableWidgetItem(rec.role));
        ui->historyTable->setItem(i, 3, new QTableWidgetItem(rec.winner));
        ui->historyTable->setItem(i, 4, new QTableWidgetItem(QString::number(rec.finalScore)));
    }


}


void GameDetailPage::onBackClicked()
{
    emit backToMenu();
}

void GameDetailPage::onStartNewGameClicked()
{
    QStringList items = {"Host", "Guest"};
    bool ok;
    QString choice = QInputDialog::getItem(this, "New Game",
                                           "Select role:", items, 0, false, &ok);
    if (!ok) return;

    bool isHost = (choice == "Host");
    emit startNewGame(currentGameType, isHost);
}