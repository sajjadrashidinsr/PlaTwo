# Client.pro
QT += core gui widgets network

CONFIG += c++17

TARGET = PLATWO_Client
TEMPLATE = app

DEFINES += BUILD_CLIENT

SOURCES += \
    main.cpp \
    src/Models/room.cpp \
    src/Ui/Dialogs/hostgamedialog.cpp \
    src/Ui/Dialogs/hostjoinselectiondialog.cpp \
    src/Ui/Dialogs/joingamedialog.cpp \
    src/Ui/Dialogs/waitingroomdialog.cpp \
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
    src/Models/GameSettings.h \
    src/Models/room.h \
    src/Ui/Dialogs/hostgamedialog.h \
    src/Ui/Dialogs/hostjoinselectiondialog.h \
    src/Ui/Dialogs/joingamedialog.h \
    src/Ui/Dialogs/waitingroomdialog.h \
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
    src/Ui/Dialogs/hostgamedialog.ui \
    src/Ui/Dialogs/hostjoinselectiondialog.ui \
    src/Ui/Dialogs/joingamedialog.ui \
    src/Ui/Dialogs/waitingroomdialog.ui \
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
    $$PWD/src/Models