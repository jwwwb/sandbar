//
// Created by jwwwb on 2017-01-16.
//

#ifndef MODEL_H
#define MODEL_H

// TODO: Q-ify this whole thing.
// no more stdlib

#include <QObject>
#include <QSettings>
#include <algorithm>
#include <string>
#include <vector>
#include <QStringList>
#include <QMap>
#include <QtGlobal>
#include <QList>
#include <QCoreApplication>
#include "mediafile.h"
#include "playlist.h"

class Model : public QObject
{
    Q_OBJECT
public:
    // constructor
    Model();

    // methods
    void addToPlaylist(MediaFile *);
    void changePlaybackOrder(int);
    void clearPlaylist();
    QString newPlaylist();
    void removePlaylist(int);
    QString renamePlaylist(QString);
    void switchPlaylist(int);

    // getters
    Playlist& getInFocusPlaylist();
    Playlist& getPlayingPlaylist();
    int getInFocusPlaylistIndex();
    int getPlayingPlaylistIndex();

private:
    // properties
    std::vector<Playlist> listOfPlaylists;
    QString currentLoc;
    int playlistInFocus;
    int entryNumSelected;
    int playlistPlaying = -1;
    int entryNumPlaying = -1;
    int shuffledEntryNumPlaying = -1;
    int shuffleOn;
    int repeatOn;
    std::vector<std::vector<int> > shuffleOrders;

    // methods
    void generateShuffleOrder(int playlistNum);
    int requestNextFileString();
    QStringList playlistNames();
    QList<int> playlistIDs();

public slots:
    void jumpToFile(int);
    void requestCurrentFile();
    void requestNextFile();
    void requestNextFileSoon();
    void requestPreviousFile();
    void requestRandomFile();

signals:
    void signalCurrentFile(QString location);
    void signalCurrentFileSoon(QString location);
    void signalNoMoreFiles();
};

#endif // MODEL_H
