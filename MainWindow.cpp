//
// Created by ych on 16.10.16.
//

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent)
{
    // Set size of the window
    setFixedSize(200, 80);

    // Create and position the button
    m_button = new QPushButton("Hello World", this);
    m_button->setGeometry(10, 10, 180, 60);
}