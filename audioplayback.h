//
// Created by jwwwb on 2016-10-17.
//

#ifndef AUDIOPLAYBACK_H
#define AUDIOPLAYBACK_H

#include "portaudio.h"
#include <QObject>
#include <QDebug>
extern "C" {
#include <libavresample/avresample.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
}

typedef struct paPlayData {
    qlonglong    frameIndex;  /* Index into sample array. */
    qlonglong    maxFrameIndex;
    int_fast16_t *recordedSamples;
} paPlayData;

class AudioPlayback : public QObject
{
    Q_OBJECT

private:
    PaError err;
    static double volume;
    PaStream *stream;

    // the following six from https://socapex.wordpress.com/2015/04/11/libav-libao-and-qt5-audio-player-tutorial/#more-112
    void setupResampler();
    void readAudioFile();
    // moving pointers away from the controller

    AVFormatContext* audio_context_ = NULL;
    AVCodecContext* codec_context_ = NULL;
    AVCodec* codec_ = NULL;
    AVAudioResampleContext* resample_context_ = NULL;


public:
    AudioPlayback();
    ~AudioPlayback();
    static double getVolume();
    static void setVolume(double);
    paPlayData data;


public slots:
    void closeStream();
    void stopPlay();
    void playFile();
    void openFile(const QString &fileName);

signals:
    void resultReady(const QString &result);
    void playbackProgress(int samplesPlayed);
};

#endif // AUDIOPLAYBACK_H
