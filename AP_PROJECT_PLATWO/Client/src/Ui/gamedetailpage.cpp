#include "gamedetailpage.h"
#include "ui_gamedetailpage.h"
#include "client_manager.h"
#include "Models/room.h"
#include "Dialogs/hostjoinselectiondialog.h"
#include "Dialogs/hostgamedialog.h"
#include "Dialogs/joingamedialog.h"
#include "Dialogs/waitingroomdialog.h"
#include <QInputDialog>
#include <QMessageBox>

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

    //clientManager->sendGetUser(currentUser->username);

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
    HostJoinSelectionDialog selection(this);
    connect(&selection, &HostJoinSelectionDialog::hostSelected, this, [this]() {
        HostGameDialog hostDlg(this);
        hostDlg.setClientManager(clientManager);  // Set client manager

        connect(&hostDlg, &HostGameDialog::roomCreationSuccess,
                this, [this](const Room& room) {
                    // Room created successfully, open waiting room as host
                    WaitingRoomDialog* waiting = new WaitingRoomDialog(true, room, this);
                    waiting->setAttribute(Qt::WA_DeleteOnClose);
                    waiting->setClientManager(clientManager);
                    waiting->show();
                });

        connect(&hostDlg, &HostGameDialog::roomCreationFailed,
                this, [](const QString& error) {
                    // Error already shown in dialog, but we could log
                    qDebug() << "Room creation failed:" << error;
                });

        hostDlg.exec();
    });

    connect(&selection, &HostJoinSelectionDialog::joinSelected, this, [this]() {
        JoinGameDialog joinDlg(this);
        joinDlg.setClientManager(clientManager);  // Set client manager

        connect(&joinDlg, &JoinGameDialog::joinSuccess,
                this, [this](const Room& room) {
                    // Joined successfully, open waiting room as guest
                    WaitingRoomDialog* waiting = new WaitingRoomDialog(false, room, this);
                    waiting->setAttribute(Qt::WA_DeleteOnClose);
                    waiting->setClientManager(clientManager);
                    waiting->show();
                });

        connect(&joinDlg, &JoinGameDialog::joinFailed,
                this, [](const QString& error) {
                    qDebug() << "Join failed:" << error;
                });

        joinDlg.exec();
    });

    selection.exec();
}