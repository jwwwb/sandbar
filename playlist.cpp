//
// Created by jwwwb on 2017-01-25.
//

#include "playlist.h"

int Playlist::id = 0;

Playlist::Playlist(QString name)
{
    identifier = Playlist::id++;
    playlistName = name;
}

// getters

MediaFile * Playlist::getEntryAt(int index) { return mediaFiles.at(index); }
int Playlist::getIdentifier() { return identifier; }
QString Playlist::getName() { return this->playlistName; }

// setters

void Playlist::append(MediaFile *file) { mediaFiles.append(file); }

void Playlist::insert(MediaFile *file, int at) { mediaFiles.insert(at, file); }

void Playlist::setName(QString newName) { playlistName = newName; }

// pass-through methods

void Playlist::clear() { this->mediaFiles.clear(); }
bool Playlist::empty() { return this->mediaFiles.empty(); }
int Playlist::size() { return this->mediaFiles.size(); }
