# Server.pro
QT += core network sql

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = PLATWO_Server
TEMPLATE = app

DEFINES += BUILD_SERVER

SOURCES += \
    main.cpp \
    src/Models/room.cpp \
    src/Server/roommanager.cpp \
    src/Server/threaded_server.cpp \
    src/Storage/storage_manager.cpp \
    src/Controller/auth_manager.cpp \
    src/User/user.cpp \
    src/Worker/request_worker.cpp

HEADERS += \
    src/Models/GameSettings.h \
    src/Models/room.h \
    src/Server/roommanager.h \
    src/Server/threaded_server.h \
    src/Storage/storage_manager.h \
    src/Controller/auth_manager.h \
    src/User/user.h \
    src/Worker/request_worker.h \
    src/Network/network_constants.h \
    src/Network/network_protocol.h

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/Server \
    $$PWD/src/Storage \
    $$PWD/src/Controller \
    $$PWD/src/User \
    $$PWD/src/Worker \
    $$PWD/src/Network