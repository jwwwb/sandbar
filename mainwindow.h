//
// Created by jwwwb on 2016-10-17.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "model.h"
#include "playbackcontroller.h"
#include <math.h>
#include <QWidget>
#include <QStringList>
#include <QObjectList>
#include <QTableWidget>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButtonHello_clicked();
    void on_pushButtonPlay_clicked();

    void on_actionOpen_triggered();

    void on_pushButtonClear_clicked();

    void on_volumeSlider_sliderMoved(int position);

    void on_actionNew_Playlist_triggered();

    void on_actionAdd_File_triggered();

    void on_pushButtonStop_clicked();

    void on_playlistTabs_currentChanged(int index);

    void on_playlistTabs_tabCloseRequested(int index);

    void on_playlistTabs_tabBarDoubleClicked(int index);

private:
    Ui::MainWindow *ui;
    Model *model;
    PlaybackController *playback;
    void updatePlaylist();
    QWidget *newPlaylist();
};

#endif // MAINWINDOW_H
