//
// Created by jwwwb on 2017-01-16.
//

#ifndef MODEL_H
#define MODEL_H

// TODO: Q-ify this whole thing.
// no more stdlib

#include <QObject>
#include <QSettings>
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
    void changePlaybackOrder(QString);
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
    int playlistInFocus;
    int entryNumSelected;
    int playlistPlaying = -1;
    int entryNumPlaying = -1;

    // methods
    QStringList playlistNames();
    QList<int> playlistIDs();

public slots:
    void jumpToFile(int);
    void requestCurrentFile();
    void requestNextFile();
    void requestPreviousFile();
    void requestRandomFile();

    signals:
    void signalCurrentFile(QString location);
    void signalNoMoreFiles();
};

#endif // MODEL_H
