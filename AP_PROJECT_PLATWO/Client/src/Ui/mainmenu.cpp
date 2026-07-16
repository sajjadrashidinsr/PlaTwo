#include "mainmenu.h"
#include "ui_mainmenu.h"
#include <QMessageBox>

MainMenu::MainMenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);


    connect(ui->btnGameDotsAndBoxes, &QPushButton::clicked, this, &MainMenu::onGameButtonClicked);
    connect(ui->btnGameNineMensMorris, &QPushButton::clicked, this, &MainMenu::onGameButtonClicked);
    connect(ui->btnGameFanorona, &QPushButton::clicked, this, &MainMenu::onGameButtonClicked);


    connect(ui->btnEditProfile, &QPushButton::clicked, this, &MainMenu::onEditProfileClicked);
    connect(ui->btnExit, &QPushButton::clicked, this, &MainMenu::onExitClicked);


    setWindowIcon(QIcon(":/icons/logo.png"));
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::setUserInfo(user* loggedInUser)
{
    currentUser = loggedInUser;
    if (currentUser) {
        ui->label_welcome->setText("Welcome, " + currentUser->name + "!");
    }
}

void MainMenu::onGameButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int gameId = -1;
    if (btn == ui->btnGameDotsAndBoxes) gameId = 0;
    else if (btn == ui->btnGameNineMensMorris) gameId = 1;
    else if (btn == ui->btnGameFanorona) gameId = 2;

    if (gameId != -1) {
        emit gameSelected(gameId);
    }
}

void MainMenu::onEditProfileClicked()
{
    emit editProfileRequested();
}

void MainMenu::onExitClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Log Out",
                                                              "Are you sure you want to log out?",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        currentUser = nullptr;
        emit exitRequested();
    }
}