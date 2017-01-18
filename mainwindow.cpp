//
// Created by jwwwb on 2016-10-17.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <iomanip> // setprecision
#include <sstream> // stringstream

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new Model();
    playback = new PlaybackController();
    if (ui->playlistTabs->count() == 0) {
        ui->playlistTabs->addTab(newPlaylist(), "Default");
        ui->playlistTabs->setCurrentIndex(ui->playlistTabs->count()-1);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete playback;
    delete model;
}

void MainWindow::on_pushButtonHello_clicked()
{
    MediaFile *mediaFile = new MediaFile("~/Music/test.flac");
    model->addMediaFile(mediaFile);
    this->updatePlaylist();
}

void MainWindow::updatePlaylist()
{
    QTableWidget *currentPlaylistTable = NULL;
    if (ui->playlistTabs->count() == 0) {
        ui->playlistTabs->addTab(newPlaylist(), "New Playlist");
        ui->playlistTabs->setCurrentIndex(ui->playlistTabs->count()-1);
    }
    QWidget* pWidget = ui->playlistTabs->currentWidget();
    QObjectList children = pWidget->children();
    foreach(QObject *child, children) {
        if (std::strcmp(child->metaObject()->className(), "QTableWidget") == 0) {
            currentPlaylistTable = (QTableWidget *)child;
        }
    }
    if (!model->getCurrentPlaylist().empty()) {
        if (currentPlaylistTable != NULL) {
            currentPlaylistTable->setRowCount(model->getCurrentPlaylist().size());
            int rowIndex = 0;
            for (std::vector<MediaFile>::iterator it = model->getCurrentPlaylist().MediaFiles.begin() ;
                 it != model->getCurrentPlaylist().MediaFiles.end(); ++it) {
                int columnIndex = 0;
                for(QString key : this->model->getPlaylistSettings()) {
                    QTableWidgetItem *doesThisNameMatter = new QTableWidgetItem(it->allData[key].c_str());
                    currentPlaylistTable->setItem(rowIndex, columnIndex, doesThisNameMatter);
                    columnIndex++;
                }
                rowIndex++;
            }
        } else {
            qDebug() << "table = NULL";
        }
    } else {
        if (currentPlaylistTable != NULL) {
            currentPlaylistTable->clearContents();
            currentPlaylistTable->setRowCount(0);
        }
    }
}

void MainWindow::on_pushButtonPlay_clicked()
{
    if (!model->getCurrentPlaylist().empty()) {
        QString fileName = model->getCurrentEntry().location.c_str();
        playback->setFile(fileName);
        playback->startPlaying();
    }
}

void MainWindow::on_pushButtonStop_clicked()
{
    qDebug() << "pushed stop button";
    playback->stopPlaying();
}

void MainWindow::on_actionOpen_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open File(s)", getenv("HOME"), "Audio Files (*.mp3 *.flac *.ogg *.wav)");
    if (!fileNames.isEmpty()) {
        this->model->clearPlaylist();
        for (QString fileName : fileNames) {
            MediaFile *mediaFile = new MediaFile(fileName.toStdString());
            model->addMediaFile(mediaFile);
        }
        this->updatePlaylist();
    }
}

void MainWindow::on_actionAdd_File_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Add File(s)", getenv("HOME"), "Audio Files (*.mp3 *.flac *.ogg *.wav)");
    if (!fileNames.isEmpty()) {
        for (QString fileName : fileNames) {
            MediaFile *mediaFile = new MediaFile(fileName.toStdString());
            model->addMediaFile(mediaFile);
        }
        this->updatePlaylist();
    }
}

void MainWindow::on_pushButtonClear_clicked()
{
    model->clearPlaylist();
    this->updatePlaylist();
}

void MainWindow::on_volumeSlider_sliderMoved(int position)
{
    double volumeMultiplier = 0;
    double dBLevel = 0;
    if (position == 0) {
        ui->volumeLabel->setText("-∞dB");
    } else {
        dBLevel = 10*log2(position/256.0);
        volumeMultiplier = pow(10.0, dBLevel/10.0);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << dBLevel;
        std::string dBString = ss.str();
        ui->volumeLabel->setText((dBString + "dB").c_str());
    }
    playback->setVolume(volumeMultiplier);
}

QWidget *MainWindow::newPlaylist() {
    QWidget *parent = new QWidget();
    QStringList columnHeaders = model->getPlaylistSettings();
    QTableWidget *tableWidget = new QTableWidget(0, columnHeaders.size(), parent);
    tableWidget->setHorizontalHeaderLabels(columnHeaders);
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(tableWidget);
    parent->setLayout(layout);
    return parent;
}

void MainWindow::on_actionNew_Playlist_triggered()
{
    // TODO model
    QString newListName = this->model->newPlaylist();
    ui->playlistTabs->addTab(newPlaylist(), newListName);
    ui->playlistTabs->setCurrentIndex(ui->playlistTabs->count()-1);
}



void MainWindow::on_playlistTabs_currentChanged(int index)
{
    // todo store tab / playlist correspondence somewhere.
}

void MainWindow::on_playlistTabs_tabCloseRequested(int index)
{

}

void MainWindow::on_playlistTabs_tabBarDoubleClicked(int index)
{

}
