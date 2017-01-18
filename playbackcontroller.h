//
// Created by jwwwb on 2017-01-16.
//

#ifndef PLAYBACKCONTROLLER_H
#define PLAYBACKCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QString>
#include "audioplayback.h"
#include "globals.h"
#include <math.h>
extern "C" {
#include <libavresample/avresample.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
}


class PlaybackController : public QObject
{
    Q_OBJECT
            QThread playerThread;

public slots:
    void handleResults(const QString &);

    signals:
    void fileSignal(const QString &);
    void stopSignal();
    void playSignal();

public:
    PlaybackController();
    ~PlaybackController();
    void stopPlaying();
    void startPlaying();
    void setFile(QString);
    void setVolume(double);
    void setVolume(int);

private:
    AudioPlayback *player;
    double volumeMultiplier;
    QString currentFile;
};

#endif // PLAYBACKCONTROLLER_H
