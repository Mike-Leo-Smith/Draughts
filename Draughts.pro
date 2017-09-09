#-------------------------------------------------
#
# Project created by QtCreator 2017-09-06T16:09:16
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Draughts
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    UI/DraughtsView.cpp \
    Model/Board.cpp \
    Model/Draughts.cpp \
    Model/Path.cpp \
    Network/Connection.cpp \
    Model/Player.cpp \
    UI/GameController.cpp \
    UI/LoginDialog.cpp

HEADERS += \
    UI/DraughtsView.h \
    Model/Board.h \
    Model/Draughts.h \
    Model/Path.h \
    Model/Piece.h \
    Model/Player.h \
    Network/Connection.h \
    CheatingConfig.h \
    UI/GameController.h \
    UI/LoginDialog.h

FORMS += \
    UI/Draughts.ui \
    UI/Login.ui

RESOURCES += \
    assets.qrc
