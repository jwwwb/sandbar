//
// Created by jwwwb on 2016-10-17.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "model.h"
#include "mediafile.h"
#include "playbackcontroller.h"
#include <math.h>
#include <vector>
#include <QWidget>
#include <QStringList>
#include <QObjectList>
#include <QObject>
#include <QTableWidget>
#include <QDropEvent>
#include <QCoreApplication>
#include <QUrl>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateProgress();

public slots:
    void playPlaylistEntry();
    void rightClickPlaylist(const QPoint& pos);
    void slotColumnResized(int idx, int oldsize, int newsize);
    void slotDuration(qlonglong duration);
    void slotPlaybackEnded();
    void slotPlaybackProgress(qlonglong timePlayed);


private slots:
    void on_actionAdd_File_triggered();
    void on_actionNew_Playlist_triggered();
    void on_actionOpen_triggered();
    void on_playlistTabs_currentChanged(int index);
    void on_playlistTabs_tabBarDoubleClicked(int index);
    void on_playlistTabs_tabCloseRequested(int index);
    void on_progressSlider_sliderPressed();
    void on_progressSlider_sliderReleased();
    void on_pushButtonClear_clicked();
    void on_pushButtonHello_clicked();
    void on_pushButtonNext_clicked();
    void on_pushButtonPause_clicked();
    void on_pushButtonPlay_clicked();
    void on_pushButtonPrevious_clicked();
    void on_pushButtonRandom_clicked();
    void on_pushButtonStop_clicked();
    void on_volumeSlider_sliderMoved(int position);    


private:
    // methods
    void connectSignals();
    void deletePlaylist(int);
    QList<int> getSelectedInPlaylist();
    void initalizePlaylistTable();
    void newPlaylist();
    void updatePlaylist();


    // properties
    int playlistInFocus;
    std::vector<int> listOfPlaylistIDs;
    std::vector<QString> listOfPlaylistNames;
    QTableWidget *playlistTableWidget; // just one now, that moves.
    Model *model;
    PlaybackController *playback;
    Ui::MainWindow *ui;
    int freezeUpdates = 0;

signals:
    void signalNextPushed();
    void signalPreviousPushed();
};

#endif // MAINWINDOW_H
