//
// Created by jwwwb on 2016-10-17.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    player = new AudioPlayback();
    model = new Model();
}

MainWindow::~MainWindow()
{
    player->closeStream();
    delete ui;
}

void MainWindow::on_pushButtonHello_clicked()
{
    MediaFile *mediaFile = new MediaFile("World", "Hello", "33:33");
    model->addMediaFile(mediaFile);
    this->updatePlaylist();
}

void MainWindow::updatePlaylist()
{
    if (!model->playlist.empty()) {
        ui->tableWidgetPlaylist->setRowCount(model->playlist.size());
        int tableIndex = 0;
        for (std::vector<MediaFile>::iterator it = model->playlist.begin() ; it != model->playlist.end(); ++it) {
            QTableWidgetItem *newFileName = new QTableWidgetItem(it->fileName.c_str());
            QTableWidgetItem *newLocation = new QTableWidgetItem(it->location.c_str());
            QTableWidgetItem *newDirectory = new QTableWidgetItem(it->directory.c_str());
            ui->tableWidgetPlaylist->setItem(tableIndex, 0, newFileName);
            ui->tableWidgetPlaylist->setItem(tableIndex, 1, newLocation);
            ui->tableWidgetPlaylist->setItem(tableIndex, 2, newDirectory);
            tableIndex++;
        }
    } else {
        ui->tableWidgetPlaylist->clearContents();
        ui->tableWidgetPlaylist->setRowCount(0);
    }
}

void MainWindow::on_pushButtonPlay_clicked()
{
    player->playSine();
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", getenv("HOME"), "Mp3 File (*.mp3)");
    if (!fileName.isEmpty()) {
//        qDebug() << fileName;
        MediaFile *mediaFile = new MediaFile(fileName.toStdString());
        model->addMediaFile(mediaFile);
        this->updatePlaylist();
    }
}

void MainWindow::on_pushButtonClear_clicked()
{
    model->clearPlaylist();
    this->updatePlaylist();
}
