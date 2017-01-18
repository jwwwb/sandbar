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
    model.cpp \
    playbackcontroller.cpp \
    mediafile.cpp

HEADERS  += mainwindow.h \
    audioplayback.h \
    model.h \
    playbackcontroller.h \
    mediafile.h \
    globals.h

FORMS    += mainwindow.ui

macx: LIBS += -L$$PWD/../../../../../usr/local/Cellar/portaudio/19.20140130/lib/ -lportaudio.2
INCLUDEPATH += $$PWD/../../../../../usr/local/Cellar/portaudio/19.20140130/include
DEPENDPATH += $$PWD/../../../../../usr/local/Cellar/portaudio/19.20140130/include
# macx: LIBS += -lportaudio.2


macx: LIBS += -L$$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/ -lavcodec
macx: LIBS += -L$$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/ -lavresample
macx: LIBS += -L$$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/ -lavformat
macx: LIBS += -L$$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/ -lavutil

INCLUDEPATH += $$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/include
DEPENDPATH += $$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/include

macx: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/libavcodec.a
macx: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/libavresample.a
macx: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/libavformat.a
macx: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/Cellar/ffmpeg/3.2.2/lib/libavutil.a
