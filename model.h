//
// Created by jwwwb on 2017-01-16.
//

#include "model.h"
#include <QDebug>

int Playlist::id = 0;

Playlist::Playlist(QString name)
{
    identifier = Playlist::id++;
    playlistName = name;
}

void Playlist::addMediaFile(MediaFile *mediaFile)
{
    this->MediaFiles.push_back(*mediaFile);
}

MediaFile& Playlist::getCurrentEntry() {
    return this->MediaFiles.at(currentEntry);
}

int Playlist::getIdentifier() { return this->identifier; }
QString Playlist::getName() { return this->playlistName; }
bool Playlist::empty() { return this->MediaFiles.empty(); }
int Playlist::size() { return this->MediaFiles.size(); }
void Playlist::clear() { this->MediaFiles.clear(); }

Model::Model()
{
    QString newName = "Default";
    Playlist *emptyPlaylist = new Playlist(newName);
    this->playlists.push_back(*emptyPlaylist);
    playlistInFocus = 0;
}

void Model::addMediaFile(MediaFile *mediaFile)
{
    this->playlists.at(playlistInFocus).addMediaFile(mediaFile);
}

QString Model::newPlaylist()
{
    QString newName = "New Playlist";
    for (Playlist list : this->playlists) {
        if (list.getName() == newName) {
            newName = "New Playlist (1)";
        }
    }
    Playlist *emptyPlaylist = new Playlist(newName);
    this->playlists.push_back(*emptyPlaylist);
    playlistInFocus++;
    return newName;
}

void Model::clearPlaylist()
{
    this->getCurrentPlaylist().clear();
}

Playlist& Model::getCurrentPlaylist()
{
    return this->playlists.at(playlistInFocus);
}

MediaFile& Model::getCurrentEntry()
{
    return this->getCurrentPlaylist().getCurrentEntry();
}

QStringList Model::getPlaylistSettings()
{
    static QStringList playSet = QStringList() << "fileName" << "filePath" << "duration" << "directoryName";
    return playSet;
}
