#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "loginpage.h"
#include "signuppage.h"
#include "forgotpasspage.h"
#include "client_manager.h"
#include "user.h"
#include "mainmenuwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

Loginwindow::Loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Loginwindow)
    , clientManager(nullptr)
    , m_loginPage(nullptr)
    , m_signupPage(nullptr)
    , m_forgotPage(nullptr)
    , m_mainMenuWindow(nullptr)
{
    qDebug() << "[Loginwindow] Constructor started";

    ui->setupUi(this);
    qDebug() << "[Loginwindow] ui->setupUi done";

    this->setWindowIcon(QIcon(":/icons/logo.png"));

    clientManager = new ClientManager(this);
    if (!clientManager) {
        qDebug() << "[Loginwindow] ERROR: Failed to create ClientManager!";
        return;
    }
    qDebug() << "[Loginwindow] ClientManager created";

    connect(clientManager, &ClientManager::connected,
            this, &Loginwindow::onConnectedToServer);
    connect(clientManager, &ClientManager::disconnected,
            this, &Loginwindow::onServerDisconnected);
    connect(clientManager, &ClientManager::error,
            this, &Loginwindow::onServerError);

    qDebug() << "[Loginwindow] Creating login page...";
    m_loginPage = new loginPage(clientManager, this);
    ui->verticalLayoutPageLogin->addWidget(m_loginPage);
    qDebug() << "[Loginwindow] Login page created";

    qDebug() << "[Loginwindow] Creating signup page...";
    m_signupPage = new signuppage(clientManager, this);
    ui->verticalLayoutPagesignup->addWidget(m_signupPage);
    qDebug() << "[Loginwindow] Signup page created";

    qDebug() << "[Loginwindow] Creating forgot password page...";
    m_forgotPage = new forgotpasspage(clientManager, this);
    ui->verticalLayoutPageforgotpass->addWidget(m_forgotPage);
    qDebug() << "[Loginwindow] Forgot password page created";

    // ============================================
    // Connect signals between pages
    // ============================================

    connect(m_loginPage, &loginPage::createAccountClicked, this, [this]() {
        qDebug() << "[Loginwindow] Switching to signup page";
        ui->stackedWidget->setCurrentWidget(ui->page_signup);
    });

    connect(m_signupPage, &signuppage::loginClicked, this, [this]() {
        qDebug() << "[Loginwindow] Switching to login page";
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_signupPage, &signuppage::signUpSuccessful, this, [this]() {
        qDebug() << "[Loginwindow] Signup successful, switching to login";
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_loginPage, &loginPage::forgotPasswordClicked, this, [this]() {
        qDebug() << "[Loginwindow] Switching to forgot password page";
        ui->stackedWidget->setCurrentWidget(ui->pageforgotpass);
    });

    connect(m_forgotPage, &forgotpasspage::backToLoginClicked, this, [this]() {
        qDebug() << "[Loginwindow] Back to login from forgot password";
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_forgotPage, &forgotpasspage::passwordChanged, this, [this]() {
        qDebug() << "[Loginwindow] Password changed, switching to login";
        ui->stackedWidget->setCurrentWidget(ui->pageLogin);
    });

    connect(m_loginPage, &loginPage::loginSuccessful, this, &Loginwindow::onLoginSuccessful);

    ui->stackedWidget->setCurrentWidget(ui->pageLogin);

    QTimer::singleShot(100, this, &Loginwindow::tryConnectToServer);

    qDebug() << "[Loginwindow] Constructor finished";
}

void Loginwindow::onLoginSuccessful(std::shared_ptr<user> loggedInUser)
{
    qDebug() << "[Loginwindow] ===== onLoginSuccessful called =====";
    qDebug() << "[Loginwindow] User:" << (loggedInUser ? loggedInUser->username : "null");

    if (!loggedInUser) {
        qDebug() << "[Loginwindow] ERROR: loggedInUser is null!";
        return;
    }

    m_currentUser.reset(new user(*loggedInUser));   // Make a copy to own via unique_ptr
    qDebug() << "[Loginwindow] Current user saved:" << m_currentUser->username;

    qDebug() << "[Loginwindow] Creating MainMenuWindow...";
    m_mainMenuWindow = new MainMenuWindow(m_currentUser.get(), clientManager, nullptr);

    if (!m_mainMenuWindow) {
        qDebug() << "[Loginwindow] ERROR: Failed to create MainMenuWindow!";
        return;
    }

    m_mainMenuWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    qDebug() << "[Loginwindow] MainMenuWindow created successfully";

    connect(m_mainMenuWindow, &MainMenuWindow::destroyed, this, &Loginwindow::onMainMenuClosed);
    connect(m_mainMenuWindow, &MainMenuWindow::exitRequested, this, [this]() {
        qDebug() << "[Loginwindow] Exit requested, closing main menu";
        if (m_mainMenuWindow) {
            m_mainMenuWindow->close();
        }
    });

    QFile styleFile(":/qss/main_menu.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QTextStream(&styleFile).readAll();
        m_mainMenuWindow->setStyleSheet(style);
        qDebug() << "[Loginwindow] Style applied";
    } else {
        qDebug() << "[Loginwindow] main_menu.qss not found, using default style";
        m_mainMenuWindow->setStyleSheet("QWidget { background-color: #1e1e2e; color: white; }");
    }

    qDebug() << "[Loginwindow] Showing main menu...";
    m_mainMenuWindow->show();
    qDebug() << "[Loginwindow] Main menu shown, hiding login window...";
    this->hide();
    qDebug() << "[Loginwindow] Login window hidden";
}

void Loginwindow::onMainMenuClosed()
{
    qDebug() << "[Loginwindow] Main menu closed, showing login window";
    m_mainMenuWindow = nullptr;
    this->show();
}

void Loginwindow::showLoginWindow()
{
    this->show();
    if (m_mainMenuWindow) {
        m_mainMenuWindow->close();
        m_mainMenuWindow = nullptr;
    }
}

bool Loginwindow::tryConnectToServer()
{
    if (!clientManager) {
        qDebug() << "[Loginwindow] clientManager is null!";
        return false;
    }

    if (clientManager->connectToServer()) {
        qDebug() << "[Loginwindow] Connecting to server...";
        return true;
    } else {
        QMessageBox::warning(this, "Connection Error",
                             "Could not connect to server. Please make sure the server is running.");
        return false;
    }
}

void Loginwindow::onConnectedToServer()
{
    statusBar()->showMessage("Connected to server", 3000);
}

void Loginwindow::onServerDisconnected()
{
    statusBar()->showMessage("Disconnected from server", 3000);
}

void Loginwindow::onServerError(const QString& error)
{
    statusBar()->showMessage("Server error: " + error, 5000);
}

Loginwindow::~Loginwindow()
{
    delete ui;
}