QT += core gui widgets multimedia
QT += multimedia

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
    physicsengine.cpp \
    portal.cpp \
    variante.cpp \
    varianteportal.cpp

HEADERS += \
    angstromlevy.h \
    difficultyconfig.h \
    enemigo.h \
    entidad.h \
    gameengine.h \
    gamestate.h \
    jugador.h \
    mainwindow.h \
    physicsengine.h \
    portal.h \
    variante.h \
    varianteportal.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    recursos.qrc
