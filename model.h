//
// Created by jwwwb on 2017-01-16.
//

#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <QStringList>
#include "mediafile.h"

class Playlist
{
private:
    int currentEntry;
    QString playlistName;
    int identifier;

public:
    std::vector<MediaFile> MediaFiles;
    Playlist(QString name);
    int getIdentifier();
    QString getName();
    MediaFile& getCurrentEntry();
    void addMediaFile(MediaFile *);
    static int id;
    bool empty();
    int size();
    void clear();
};


class Model
{
public:
    Model();
    QString newPlaylist();
    void addMediaFile(MediaFile *);
    void clearPlaylist();
    Playlist& getCurrentPlaylist();
    MediaFile& getCurrentEntry();
    QStringList getPlaylistSettings();

private:
    QStringList playlistSettings;
    QStringList playlistNames;
    int playlistInFocus;
    std::vector<Playlist> playlists;
};

#endif // MODEL_H
