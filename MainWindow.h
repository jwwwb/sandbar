//
// Created by ych on 16.10.16.
//

#ifndef SANDBAR_MAINWINDOW_H
#define SANDBAR_MAINWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QtWidgets/QPushButton>
#include "AudioPlayback.h"
#include <QObject>


class MainWindow : public QWidget
{
    //Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
private slots:
    void slotButtonClicked();
private:
    QPushButton *m_button;
    QPushButton *play_button;
    AudioPlayback *Player;
};

#endif //SANDBAR_MAINWINDOW_H
