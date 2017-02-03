//
// Created by jwwwb on 2017-01-16.
//

#ifndef PLAYBACKCONTROLLER_H
#define PLAYBACKCONTROLLER_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QThread>
#include "portaudiointerface.h"
#include "audiodecoder.h"
#include "globals.h"
#include <math.h>

class PlaybackController : public QObject
{
    Q_OBJECT
    QThread decoderThread;

public:
    PlaybackController();
    ~PlaybackController();

    // these fuckers have to be public for slots
    AudioDecoder *decoder;
    PortAudioInterface *streamer;

    // from main/gui
    qlonglong getDuration();
    void pausePushed();
    void playPushed();
    void stopPushed();

    // from gui or hotkeys
    void setVolume(float);
    void setVolume(int);
    void seekFile(qlonglong);
    void seekDifferential(qlonglong);

public slots:
    // from decoder
    void slotPlaybackProgress(qlonglong timePlayed);
    void slotDuration(qlonglong duration);

private:
    // pointer to the data queue
    bufferQueue *dataPointer;

    // playback variables
    int paused;
    float volumeMultiplier;
    qlonglong fileDuration;
    qlonglong currentProgress;

    signals:
    // to decoder
    void playSignal();
    void seekSignal(qlonglong);
    void stopSignal();
    void volumeSignal(float); // do directly?

    // to portaudiointerface
    void pauseSignal();
    void resumeSignal();
};

#endif // PLAYBACKCONTROLLER_H
