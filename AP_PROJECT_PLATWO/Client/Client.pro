# Client.pro
QT += core gui widgets network

CONFIG += c++17

TARGET = PLATWO_Client
TEMPLATE = app

DEFINES += BUILD_CLIENT

SOURCES += \
    main.cpp \
    src/Ui/Loginwindow.cpp \
    src/Ui/loginpage.cpp \
    src/Ui/signuppage.cpp \
    src/Ui/forgotpasspage.cpp \
    src/Ui/MainMenuWindow.cpp \
    src/Ui/mainmenu.cpp \
    src/Ui/editprofilepage.cpp \
    src/Ui/gamedetailpage.cpp \
    src/Network/client_manager.cpp \
    src/User/user.cpp

HEADERS += \
    src/Ui/Loginwindow.h \
    src/Ui/loginpage.h \
    src/Ui/signuppage.h \
    src/Ui/forgotpasspage.h \
    src/Ui/MainMenuWindow.h \
    src/Ui/mainmenu.h \
    src/Ui/editprofilepage.h \
    src/Ui/gamedetailpage.h \
    src/Network/client_manager.h \
    src/Network/network_constants.h \
    src/Network/network_protocol.h \
    src/User/user.h

FORMS += \
    src/Ui/Loginwindow.ui \
    src/Ui/loginpage.ui \
    src/Ui/signuppage.ui \
    src/Ui/forgotpasspage.ui \
    src/Ui/MainMenuWindow.ui \
    src/Ui/mainmenu.ui \
    src/Ui/editprofilepage.ui \
    src/Ui/gamedetailpage.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/Ui \
    $$PWD/src/Network \
    $$PWD/src/User