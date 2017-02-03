//
// Created by jwwwb on 2017-01-17.
//

#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <map>
#include <stdio.h>
#include <string>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QObject>
#include <QMap>
#include <sstream>
#include <sys/stat.h>
#include <time.h>
extern "C" {
#include <libavresample/avresample.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
// for metadata
#include <libavutil/dict.h>
}

enum MetaData {
    artistName,
    trackTitle,
    albumTitle,
    date,
    genre,
    albumArtist,
    trackNumber,
    totalTracks,
    discNumber,
    totalDiscs,
    comment
};

enum fileDataT {
    fileName = 11,
    directoryName,
    filePath,
    fileSize,
    lastModified
};

enum mediaData {
    duration = 16,
    samples,
    sampleRate,
    channels,
    bitsPerSample,
    bitrate,
    codec,
    // TODO:
            encoding,
    encoderTool,
    audioMD5
};

typedef struct IntString
{
    IntString(){
        st = "";
        in = 0;
    }
    IntString(QString str){
        st = str;
        in = 0;
    }
    IntString(int integer){
        st = QString("%0").arg(integer);
        in = integer;
    }
    IntString(QString str, qlonglong integer) {
        st = str;
        in = integer;
    }
    QString st;
    qlonglong in;
} IntString;


class MediaFile
{
public:
    MediaFile(QString location);
    QString location;
    QMap<QString, IntString> allData;
    static QMap<QString, IntString> getData(const QString&);

private:
    static AVFormatContext* formatContext;
    static AVDictionaryEntry *tag;
    static AVCodecContext* codecContext;
};

#endif // MEDIAFILE_H
