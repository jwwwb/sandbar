#-------------------------------------------------
#
# Project created by QtCreator 2017-01-10T19:55:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sandbar
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    audioplayback.cpp \
    model.cpp

HEADERS  += mainwindow.h \
    audioplayback.h \
    model.h

FORMS    += mainwindow.ui

macx: LIBS += -L$$PWD/../../../../../usr/local/Cellar/portaudio/19.20140130/lib/ -lportaudio.2

INCLUDEPATH += $$PWD/../../../../../usr/local/Cellar/portaudio/19.20140130/include
DEPENDPATH += $$PWD/../../../../../usr/local/Cellar/portaudio/19.20140130/include

#macx: LIBS += -lportaudio.2
