//
// Created by jwwwb on 2017-01-25.
//

#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include "portaudiointerface.h"
#include "globals.h"
extern "C" {
#include <libavresample/avresample.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
// for metadata
#include <libavutil/dict.h>
}

class AudioDecoder : public QObject
{
    Q_OBJECT
    QThread decoderThread;
public:
    AudioDecoder();
    ~AudioDecoder();
    bufferQueue *getDataAddress();

private:
    AVFormatContext* formatContext = NULL;
    AVCodecContext* codecContext = NULL;
    AVCodec* deCodec = NULL;
    AVAudioResampleContext* resampleContext = NULL;
    uint8_t *output; // This is the audio data buffer. av_samples_alloc wants uint8_t
    int out_linesize; // Used internally by libAV.
    int out_samples; // How many samples we will play, AFTER resampling.
    qlonglong out_sample_fmt; // Bit-depth.
    AVPacket pkt;
    AVFrame* frm;

    // the actual data buffer
    bufferQueue bufferData; // this guy owns the data? we'll see.

    // my own state variables
    int fileOpen = 0;
    int initialized = 0;
    int audioStream = -1;
    QTimer *timer;
    qlonglong duration;
    qlonglong nextDuration;
    qlonglong currentTime;

// methods
private:
    void fillUpBuffer(int switchBuffer);
    void finishFile(); // this combines cleanUp and the rest in one func.
    void cleanUpFile(); // has to be split after all, cause otherwise it stops too soon
    qlonglong getTime();
    qlonglong enqueueFile(const QString &);
    qlonglong loadFile(const QString &); // TODO don't pass this, use property
    int readNextPacket();

public slots:
    // upon thread start
    void slotInitialize();

    // from model
    void slotFile(const QString &fileName);
    void slotFileSoon(const QString &fileName);

    // from playback controller
    void slotPlay();    // combine these two
    void slotSeekTo(qlonglong time);
    void slotSetVolume(float newVolume);     // do directly
    void slotStop();

    // from Qtimer
    void slotTryToProgress();


signals:
    // to mainwindow now, I think?
    void signalDuration(qlonglong duration);
    void signalPlaybackProgress(qlonglong timePlayed);

    // to portaudio
    void bufferReadyForPlayback();
    void finishedPlaying();

    // to model mostly, I think
    void signalRequestFile(); // TODO: do this earlier
    // this one is mainwindow, or at least soon to be, for sure.
    void signalFileReadEnded();
};

#endif // AUDIODECODER_H
