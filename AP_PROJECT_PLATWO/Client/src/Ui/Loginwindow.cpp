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
    , m_currentUser(nullptr)
    , m_mainMenuWindow(nullptr) {

    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/icons/logo.png"));

    clientManager = new ClientManager(this);


    connect(clientManager, &ClientManager::connected,
            this, &Loginwindow::onConnectedToServer);
    connect(clientManager, &ClientManager::disconnected,
            this, &Loginwindow::onServerDisconnected);
    connect(clientManager, &ClientManager::error,
            this, &Loginwindow::onServerError);


    m_loginPage = new loginPage(clientManager, this);
    ui->verticalLayoutPageLogin->addWidget(m_loginPage);

    m_signupPage = new signuppage(clientManager, this);
    ui->verticalLayoutPagesignup->addWidget(m_signupPage);

    m_forgotPage = new forgotpasspage(clientManager, this);
    ui->verticalLayoutPageforgotpass->addWidget(m_forgotPage);


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


    connect(m_loginPage, &loginPage::loginSuccessful, this, [this](user* loggedInUser) {
        qDebug() << "[Loginwindow] Login successful for user:" << (loggedInUser ? loggedInUser->username : "null");

    // اگر منوی قبلی وجود دارد، آن را ببند
        if (m_mainMenuWindow) {
        qDebug() << "[Loginwindow] Closing existing main menu";
        m_mainMenuWindow->close();
        m_mainMenuWindow = nullptr;
    }

    // اگر کاربر قبلی وجود داشت، حذفش کن
    if (m_currentUser) {
        delete m_currentUser;
        m_currentUser = nullptr;
    }

    // ذخیره کاربر فعلی
    m_currentUser = loggedInUser;
    qDebug() << "[Loginwindow] Current user set:" << (m_currentUser ? m_currentUser->username : "null");

    // ایجاد منوی اصلی
    qDebug() << "[Loginwindow] Creating MainMenuWindow...";
    m_mainMenuWindow = new MainMenuWindow(m_currentUser, clientManager, nullptr);
    m_mainMenuWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    qDebug() << "[Loginwindow] MainMenuWindow created";

    // وقتی منو بسته شد، پنجره اصلی را نشان بده
    connect(m_mainMenuWindow, &MainMenuWindow::destroyed, this, &Loginwindow::onMainMenuClosed);

    // وقتی کاربر خواست خارج شود، منو را ببند
    connect(m_mainMenuWindow, &MainMenuWindow::exitRequested, this, [this]() {
        qDebug() << "[Loginwindow] Exit requested, closing main menu";
        if (m_mainMenuWindow) {
            m_mainMenuWindow->close();
        }
    });

    // اعمال استایل (با بررسی وجود فایل)
    QFile styleFile(":/qss/main_menu.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QTextStream(&styleFile).readAll();
        m_mainMenuWindow->setStyleSheet(style);
        qDebug() << "[Loginwindow] Style applied successfully";
    } else {
        qDebug() << "[Loginwindow] WARNING: main_menu.qss not found!";
        m_mainMenuWindow->setStyleSheet("QWidget { background-color: #1e1e2e; color: white; }");
    }

    // نمایش منو
    qDebug() << "[Loginwindow] Calling show() on main menu...";
    m_mainMenuWindow->show();
    qDebug() << "[Loginwindow] show() returned, hiding login window...";
    this->hide();
    qDebug() << "[Loginwindow] Login window hidden. Main menu should be visible now.";
});

    ui->stackedWidget->setCurrentWidget(ui->pageLogin);


    QTimer::singleShot(100, this, &Loginwindow::tryConnectToServer);
}

void Loginwindow::onMainMenuClosed() {
    qDebug() << "[Loginwindow] Main menu closed, showing login window";

    this->show();

    m_mainMenuWindow = nullptr;

    // کاربر فعلی را حذف نکن، چون ممکن است دوباره استفاده شود
    // اما اگر می‌خواهید لاگین مجدد اجباری باشد، می‌توانید حذف کنید
    // if (m_currentUser) {
    //     delete m_currentUser;
    //     m_currentUser = nullptr;
    // }
}

void Loginwindow::showLoginWindow() {
    this->show();
    if (m_mainMenuWindow) {
        m_mainMenuWindow->close();
        m_mainMenuWindow = nullptr;
    }
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

    if (m_currentUser) {
        delete m_currentUser;
        m_currentUser = nullptr;
    }


    delete ui;
}