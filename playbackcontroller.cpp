//
// Created by jwwwb on 2017-01-16.
//

#include "playbackcontroller.h"

PlaybackController::PlaybackController()
{
    volumeMultiplier = 1.0;
    currentFile = "";
    player = new AudioPlayback();
    player->moveToThread(&playerThread);
    connect(&playerThread, &QThread::finished, player, &QObject::deleteLater);
    connect(this, &PlaybackController::fileSignal, player, &AudioPlayback::openFile);
    connect(this, &PlaybackController::playSignal, player, &AudioPlayback::playFile);
    connect(this, &PlaybackController::stopSignal, player, &AudioPlayback::stopPlay);
    connect(player, &AudioPlayback::resultReady, this, &PlaybackController::handleResults);
    playerThread.start();
}

void PlaybackController::setFile(QString fileName)
{
    currentFile = fileName;
    fileSignal(fileName);
}

void PlaybackController::startPlaying()
{
    qDebug() << "about to send play signal.";
    playSignal();
}

void PlaybackController::stopPlaying()
{
    qDebug() << "pushed stop button";
    stopSignal();
    AudioPlayback::noInterrupts = false;
}

PlaybackController::~PlaybackController()
{
    playerThread.deleteLater();
    player->deleteLater();
}

void PlaybackController::handleResults(const QString &result)
{
    qDebug() << result;
}

void PlaybackController::setVolume(double volumeMultiplier) {
    this->volumeMultiplier = volumeMultiplier;
    player->setVolume(this->volumeMultiplier);
}

void PlaybackController::setVolume(int decibels) {
    if (decibels > 0) this->volumeMultiplier = 1.0;
    else {
        this->volumeMultiplier = pow(10.0, decibels/10.0);
    }
    player->setVolume(this->volumeMultiplier);
}
