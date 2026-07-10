QT += core gui widgets sql

CONFIG += c++17

TARGET = PLATWO
TEMPLATE = app

SOURCES += \
    main.cpp \
    src/Controller/auth_manager.cpp \
    src/Storage/storage_manager.cpp \
    src/User/user.cpp \
    src/Ui/Loginwindow.cpp \
    src/Ui/loginpage.cpp \
    src/Ui/signuppage.cpp \
    src/Ui/forgotpasspage.cpp

HEADERS += \
    src/Controller/auth_manager.h \
    src/Storage/storage_manager.h \
    src/User/user.h \
    src/Ui/Loginwindow.h \
    src/Ui/loginpage.h \
    src/Ui/signuppage.h \
    src/Ui/forgotpasspage.h

FORMS += \
    src/Ui/Loginwindow.ui \
    src/Ui/loginpage.ui \
    src/Ui/signuppage.ui \
    src/Ui/forgotpasspage.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/Controller \
    $$PWD/src/Storage \
    $$PWD/src/User \
    $$PWD/src/Ui \
    $$PWD/src/Core \
    $$PWD/src/Games \
    $$PWD/src/Network