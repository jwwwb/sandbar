//
// Created by ych on 16.10.16.
//

#include "MainWindow.h"
#include <QApplication>
#include <QProgressBar>
#include <QSlider>


void MainWindow::slotButtonClicked()
{
    Player->playSine();
}

MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent)
{
    // Set size of the window
    setFixedSize(200, 220);
    Player = new AudioPlayback(1);
    // Create and position the button
    m_button = new QPushButton("Quit Player", this);
    play_button = new QPushButton("Play Sound", this);
    m_button->setGeometry(10, 10, 180, 60);
    play_button->setGeometry(10, 80, 180, 60);
    connect(m_button, SIGNAL(clicked()), QApplication::instance(), SLOT(quit()));
    connect(play_button, SIGNAL (clicked()), this, SLOT (slotButtonClicked()));

    // Create a progress bar
    // with the range between 0 and 100, and a starting value of 0
    QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setGeometry(10, 150, 180, 30);

    // Create a horizontal slider
    // with the range between 0 and 100, and a starting value of 0
    QSlider *slider = new QSlider(this);
    slider->setOrientation(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(0);
    slider->setGeometry(10, 180, 180, 30);

    QObject::connect(slider, SIGNAL (valueChanged(int)), progressBar, SLOT (setValue(int)));
}

