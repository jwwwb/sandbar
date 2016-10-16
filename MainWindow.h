//
// Created by ych on 16.10.16.
//

#ifndef SANDBAR_MAINWINDOW_H
#define SANDBAR_MAINWINDOW_H

#include <QWidget>
#include <QtWidgets/QPushButton>

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget *parent = 0);
private:
    QPushButton *m_button;
};

#endif //SANDBAR_MAINWINDOW_H
