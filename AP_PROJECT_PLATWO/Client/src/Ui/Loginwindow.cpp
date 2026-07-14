#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "loginpage.h"
#include "signuppage.h"
#include "forgotpasspage.h"
#include "client_manager.h"
#include <QFile>
#include <QMessageBox>
#include <QTimer>

Loginwindow::Loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Loginwindow)
    , clientManager(nullptr)
    , m_loginPage(nullptr)
    , m_signupPage(nullptr)
    , m_forgotPage(nullptr) {

    ui->setupUi(this);

    // ایجاد ClientManager
    clientManager = new ClientManager(this);

    // اتصال سیگنال‌های ClientManager
    connect(clientManager, &ClientManager::connected,
            this, &Loginwindow::onConnectedToServer);
    connect(clientManager, &ClientManager::disconnected,
            this, &Loginwindow::onServerDisconnected);
    connect(clientManager, &ClientManager::error,
            this, &Loginwindow::onServerError);

    // ایجاد صفحات
    m_loginPage = new loginPage(clientManager, this);
    ui->verticalLayoutPageLogin->addWidget(m_loginPage);

    m_signupPage = new signuppage(clientManager, this);
    ui->verticalLayoutPagesignup->addWidget(m_signupPage);

    m_forgotPage = new forgotpasspage(clientManager, this);
    ui->verticalLayoutPageforgotpass->addWidget(m_forgotPage);

    // اتصال سیگنال‌های صفحات
    connect(m_loginPage, &loginPage::createAccountClicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->page_signup);
    });

    connect(m_signupPage, &signuppage::loginClicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_signupPage, &signuppage::signUpSuccessful, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_loginPage, &loginPage::forgotPasswordClicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageforgotpass);
    });

    connect(m_forgotPage, &forgotpasspage::backToLoginClicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_forgotPage, &forgotpasspage::passwordChanged, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_loginPage, &loginPage::loginSuccessful, this,
            [this](user* user) {
                QMessageBox::information(this, "Login Successful",
                                         "Welcome " + user->name + "!\n(Main menu will open here)");
                delete user;
            });

    // نمایش صفحه ورود
    ui->stackedWidget->setCurrentWidget(ui->pageLogin);

    // تلاش برای اتصال به سرور
    QTimer::singleShot(100, this, &Loginwindow::tryConnectToServer);
}

bool Loginwindow::tryConnectToServer() {
    if (!clientManager->connectToServer()) {
        QMessageBox::warning(this, "Connection Error",
                             "Could not connect to server. Please make sure the server is running.\n"
                             "The server must be started separately before running the client.");
        return false;
    }
    return true;
}

void Loginwindow::onConnectedToServer() {
    statusBar()->showMessage("Connected to server", 3000);
}

void Loginwindow::onServerDisconnected() {
    statusBar()->showMessage("Disconnected from server", 3000);
}

void Loginwindow::onServerError(const QString& error) {
    statusBar()->showMessage("Server error: " + error, 5000);
}

Loginwindow::~Loginwindow() {
    delete ui;
}