QT += core gui widgets
QT += core gui widgets sql

CONFIG += c++17

TARGET = AP_PROJECT_PLATWO
TEMPLATE = app

SOURCES += \
    main.cpp \
    src/Ui/Loginwindow.cpp \
    src/Ui/loginpage.cpp \
    src/Ui/signuppage.cpp \
    src/Ui/forgotpasspage.cpp \

HEADERS += \
    src/Ui/Loginwindow.h \
    src/Ui/loginpage.h \
    src/Ui/signuppage.h \
    src/Ui/forgotpasspage.h \

FORMS += \
    src/Ui/Loginwindow.ui \
    src/Ui/loginpage.ui \
    src/Ui/signuppage.ui \
    src/Ui/forgotpasspage.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/Ui