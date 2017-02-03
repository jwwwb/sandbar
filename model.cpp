//
// Created by jwwwb on 2017-01-16.
//

#include "model.h"
#include <QDebug>


Model::Model()
{
    QString newName = "Default";
    playlistInFocus = -1;
    Playlist *emptyPlaylist = new Playlist(newName);
    this->listOfPlaylists.push_back(*emptyPlaylist);
    playlistInFocus = 0;
    QSettings settings;
}

// public methods

void Model::addToPlaylist(MediaFile *mediaFile)
{
    getInFocusPlaylist().append(mediaFile);
}

void Model::changePlaybackOrder(QString order)
{
    QSettings settings;
    settings.setValue("playback_order", order);
}

void Model::clearPlaylist()
{
    getInFocusPlaylist().clear();
}

QString Model::newPlaylist()
{
    QString newName = "Default";
    QStringList possibleNames{"New Playlist", "New Playlist (1)", "New Playlist (2)",
                              "New Playlist (3)", "New Playlist (4)", "New Playlist (5)",
                              "New Playlist (6)", "New Playlist (7)", "New Playlist (8)",
                              "New Playlist (9)", "New Playlist (10)", "New Playlist (11)"};
    for (QString &tempNewName : possibleNames) {
        if (!playlistNames().contains(tempNewName)) {
            newName = tempNewName;
            break;
        }
    }
    Playlist *emptyPlaylist = new Playlist(newName);
    listOfPlaylists.push_back(*emptyPlaylist);
    playlistInFocus = listOfPlaylists.size()-1;
    return newName;
}

void Model::removePlaylist(int index) {
    listOfPlaylists.erase(listOfPlaylists.begin()+index);
    if (playlistInFocus == index && playlistInFocus > 0) {
        playlistInFocus--;
    } else if (playlistInFocus > index) {
        playlistInFocus--;
    } else if (playlistInFocus == index && playlistInFocus == 0) {
        if (listOfPlaylists.size() == 0) {
            playlistInFocus = -1;
        }
    }
}

QString Model::renamePlaylist(QString newName) {
    this->getInFocusPlaylist().setName(newName);
    // TODO: make sure name is unique, and if not, return corrected
    return newName;
}

void Model::switchPlaylist(int newIndex) {
    playlistInFocus = newIndex;
}

// getters

Playlist& Model::getInFocusPlaylist()
{
    return listOfPlaylists.at(playlistInFocus);
}

Playlist& Model::getPlayingPlaylist()
{
    return listOfPlaylists.at(playlistPlaying);
}

int Model::getPlayingPlaylistIndex() {
    return playlistPlaying;
}

int Model::getInFocusPlaylistIndex() {
    return playlistInFocus;
}

// private methods

QStringList Model::playlistNames()
{
    QStringList playlistNames;
    for (Playlist list : listOfPlaylists) {
        playlistNames << list.getName();
    }
    return playlistNames;
}

QList<int> Model::playlistIDs()
{
    QList<int> playlistIDs;
    for (Playlist list : listOfPlaylists) {
        playlistIDs << list.getIdentifier();
    }
    return playlistIDs;
}

// slots

void Model::requestPreviousFile()
{
    qDebug() << "received previous request";
    if (entryNumPlaying > 0) {
        entryNumPlaying--;
        qDebug() << "not at beginning of playlist, so decrementing counter";
    }
    QString loc = getPlayingPlaylist().getEntryAt(entryNumPlaying)->location;
    qDebug() << "emitting currentFile signal with" << loc;
    emit signalCurrentFile(loc);
}

void Model::requestCurrentFile()
{
    if (listOfPlaylists.size() <= 0) return;
    if (playlistPlaying < 0) playlistPlaying = playlistInFocus;
    if (getPlayingPlaylist().size() <= 0) return;
    if (entryNumPlaying < 0) entryNumPlaying = 0;
    QString currentLoc = getPlayingPlaylist().getEntryAt(entryNumPlaying)->location;
    qDebug() << "received current request, emitting" << currentLoc;
    emit signalCurrentFile(currentLoc);
}

void Model::requestNextFile()
{
    qDebug() << "received next request";
    if (getPlayingPlaylist().size() > entryNumPlaying + 1) {
        entryNumPlaying++;
        QString currentLoc = getPlayingPlaylist().getEntryAt(entryNumPlaying)->location;
        qDebug() << "Found more files in the playlist, emitting" << currentLoc;
        emit signalCurrentFile(currentLoc);
    } else {
        emit signalNoMoreFiles();
    }
}

void Model::requestRandomFile()
{
    if (listOfPlaylists.size() <= 0) return;
    if (getPlayingPlaylistIndex() < 0) playlistPlaying = playlistInFocus;
    if (getPlayingPlaylist().size()) {
        int newNum = qrand();
        qDebug() << "newNum =" << newNum;
        newNum %= getPlayingPlaylist().size();
        qDebug() << "newNum =" << newNum;
        QString loc = getPlayingPlaylist().getEntryAt(newNum)->location;
        emit signalCurrentFile(loc);
    }
}

void Model::jumpToFile(int row)
{
    qDebug() << "received doubleclick from row" << row << ", current playlist is" << playlistInFocus;
    QString loc = getInFocusPlaylist().getEntryAt(row)->location;
    emit signalCurrentFile(loc);
    playlistPlaying = playlistInFocus;
    entryNumPlaying = row;
}
