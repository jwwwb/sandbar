//
// Created by jwwwb on 2017-01-25.
//

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QString>
#include "mediafile.h"
#include <vector>

class Playlist
{
public:
    // constructor
    Playlist(QString name);

    // properties
    static int id; // TODO do something with this
    QList<MediaFile *> mediaFiles;

    // getters
    MediaFile *getEntryAt(int index);
    int getIdentifier();
    QString getName();

    // setters
    void append(MediaFile *file);
    void insert(MediaFile *file, int at);
    void setName(QString name);

    // pass-through methods
    void clear();
    bool empty();
    int size();

private:
    int identifier;
    QString playlistName;
};

#endif // PLAYLIST_H
