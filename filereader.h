#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>
#include "portaudiointerface.h"
#include "globals.h"
extern "C" {
#include <libavresample/avresample.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
}

class FileReader : public QObject
{
    Q_OBJECT
public:
    explicit FileReader(QObject *parent = 0);

signals:

public slots:
    void openFile(QString);
    void setDataPointer(paRingBuffer *);

private:
    AVFormatContext* formatContext = NULL;
    AVCodecContext* codecContext = NULL;
    AVCodec* codec = NULL;
    AVAudioResampleContext* resampleContext = NULL;
    uint8_t *output; // This is the audio data buffer.
    int out_linesize; // Used internally by libAV.
    int out_samples; // How many samples we will play, AFTER resampling.
    int64_t out_sample_fmt; // Bit-depth.
    AVPacket pkt;
    AVFrame* frm;
    paRingBuffer *bufferStruct;


};

#endif // FILEREADER_H
