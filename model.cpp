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
    shuffleOn = 0;
    repeatOn = 0;
}

// public methods

void Model::addToPlaylist(MediaFile *mediaFile)
{
    getInFocusPlaylist().append(mediaFile);
    generateShuffleOrder(getInFocusPlaylistIndex());
}

// TODO make an enum for order types
void Model::changePlaybackOrder(int order)
{
    shuffleOn = order;
    if (!order) shuffledEntryNumPlaying = -1;
    else {
        if (entryNumPlaying >= 0) {
            for (shuffledEntryNumPlaying = 0; shuffledEntryNumPlaying < getPlayingPlaylist().size(); ++shuffledEntryNumPlaying) {
                if (shuffleOrders.at(getPlayingPlaylistIndex()).at(shuffledEntryNumPlaying) == entryNumPlaying) {
                    break;
                }
            }
        } else {
            shuffledEntryNumPlaying = 0;
        }
    }
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
    // Maybe we don't need uniqueness...
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

void Model::generateShuffleOrder(int playlistNum)
{
    if (shuffleOrders.size() < playlistNum) return;
    if (shuffleOrders.size() == playlistNum) {
        shuffleOrders.push_back(std::vector<int>());
    }
    if (shuffleOrders.at(playlistNum).size()) shuffleOrders.at(playlistNum).clear();
    for (int i = 0; i < listOfPlaylists.at(playlistNum).size(); ++i) {
        shuffleOrders.at(playlistNum).push_back(i);
    }
    std::random_shuffle(shuffleOrders.at(playlistNum).begin(), shuffleOrders.at(playlistNum).end());
}

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
    if (shuffleOn) {
        if (!shuffledEntryNumPlaying) shuffledEntryNumPlaying = getPlayingPlaylist().size();
        --shuffledEntryNumPlaying;
        entryNumPlaying = shuffleOrders.at(getPlayingPlaylistIndex()).at(shuffledEntryNumPlaying);
    } else {
        if (entryNumPlaying > 0) {
            entryNumPlaying--;
        } else {
            if (repeatOn) entryNumPlaying = getPlayingPlaylist().size();
        }
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
    if (requestNextFileString() > 0) {
        qDebug() << "Found more files in the playlist, emitting now" << currentLoc;
        emit signalCurrentFile(currentLoc);
    }
}

void Model::requestNextFileSoon() {
    if (requestNextFileString() > 0) {
        qDebug() << "Found more files in the playlist, emitting soon" << currentLoc;
        emit signalCurrentFileSoon(currentLoc);
    }
}

int Model::requestNextFileString() {
    qDebug() << "received next request";
    if (shuffleOn) {
        shuffledEntryNumPlaying = (shuffledEntryNumPlaying + 1) % getPlayingPlaylist().size();
        entryNumPlaying = shuffleOrders.at(getPlayingPlaylistIndex()).at(shuffledEntryNumPlaying);
        currentLoc = getPlayingPlaylist().getEntryAt(entryNumPlaying)->location;
    } else {
        if (getPlayingPlaylist().size() > entryNumPlaying + 1) {
            entryNumPlaying++;
        } else {
            if (repeatOn) {
                entryNumPlaying = 0;
            } else {
                emit signalNoMoreFiles();
                return -1;
            }
        }
    }
    currentLoc = getPlayingPlaylist().getEntryAt(entryNumPlaying)->location;
    qDebug() << "Found more files in the playlist, setting" << currentLoc;
    return 1;
}

void Model::requestRandomFile()
{
    if (listOfPlaylists.size() <= 0) return;
    if (getPlayingPlaylistIndex() < 0) playlistPlaying = playlistInFocus;
    if (getPlayingPlaylist().size()) {
        int newNum = qrand();
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
