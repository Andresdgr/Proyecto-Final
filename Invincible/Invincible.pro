QT += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Invincible
TEMPLATE = app

CONFIG += c++17
CONFIG -= app_bundle

SOURCES += \
    angstromlevy.cpp \
    difficultyconfig.cpp \
    enemigo.cpp \
    entidad.cpp \
    gameengine.cpp \
    gamestate.cpp \
    jugador.cpp \
    main.cpp \
    mainwindow.cpp \
    nivel.cpp \
    physicsengine.cpp \
    portal.cpp

HEADERS += \
    angstromlevy.h \
    difficultyconfig.h \
    enemigo.h \
    entidad.h \
    gameengine.h \
    gamestate.h \
    jugador.h \
    mainwindow.h \
    nivel.h \
    physicsengine.h \
    portal.h

FORMS += \
    mainwindow.ui