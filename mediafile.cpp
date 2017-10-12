//
// Created by jwwwb on 2017-01-17.
//

#include "mediafile.h"

AVFormatContext* MediaFile::formatContext = NULL;
AVDictionaryEntry *MediaFile::tag = NULL;
AVCodecContext* MediaFile::codecContext = NULL;

QMap<QString, IntString> MediaFile::getData(const QString& location)
{
    QMap<QString, IntString> allData;
    qDebug() << "trying to obtain data for" << location;

    // filedata:
    if (location.contains('/')) {
        int splitPoint = location.lastIndexOf('/');
        allData["File Name"] = IntString(location.mid(splitPoint+1));
        allData["Directory Name"] = IntString(location.left(splitPoint));
    } else {
        allData["File Name"] = IntString(location);
        allData["Directory  Name"] = IntString(QString(""));
    }
    allData["File Path"] = IntString(location);

    qDebug() << "location stuff done";

    struct stat stat_buf;
    int rc = stat(location.toStdString().c_str(), &stat_buf);
    struct tm * timeinfo;
    time_t modificationTime = stat_buf.st_mtimespec.tv_sec;
    size_t fileSize = rc == 0 ? stat_buf.st_size : 0;
    float fsize = (float)fileSize;
    char date[20];
    char size[10];
    int i = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (fsize > 1024) {
        fsize /= 1024;
        i++;
    }
    sprintf(size, "%.*f %s", i, fsize, units[i]);
    timeinfo = localtime (&modificationTime);
    strftime(date, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    allData["File Size"] = IntString(QString(size), (int)fileSize);
    allData["Last Modified"] = IntString(QString(date));

    // media and metadata:
    if (formatContext != NULL) avformat_close_input(&formatContext);
    int error = avformat_open_input(&formatContext, location.toStdString().c_str(), NULL, NULL);
    if (error < 0) {
        qDebug() << "error opening av format. error no:" << error;
    } else {
        while ((tag = av_dict_get(formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
            // I don't think I can do case/switch here, C only supports ints
            if (!QString::compare(tag->key, "album", Qt::CaseInsensitive) ||
                    !QString::compare(tag->key, "album title", Qt::CaseInsensitive)) {
                allData["Album Title"] = IntString(QString(tag->value));
            } else if (!QString::compare(tag->key, "artist", Qt::CaseInsensitive)) {
                allData["Artist Name"] = IntString(QString(tag->value));
            } else if (!QString::compare(tag->key, "genre", Qt::CaseInsensitive)) {
                allData["Genre"] = IntString(QString(tag->value));
            } else if (!QString::compare(tag->key, "title", Qt::CaseInsensitive)) {
                allData["Track Title"] =IntString(QString(tag->value));
            } else if (!QString::compare(tag->key, "track", Qt::CaseInsensitive)) {
                allData["Track Number"] = IntString(QString(tag->value), std::atoi(tag->value));
            } else {
                allData["Other"] = IntString(QString(tag->key) + QString(": ") + QString(tag->value));
            }
        }
        if (avformat_find_stream_info(formatContext, NULL) < 0) {
            qDebug() << "Could not find file info.";
            avformat_close_input(&formatContext);
        } else {
            qDebug() << "checkpoint 1";
            qlonglong dur = formatContext->duration;
            if (dur < 0) dur = 0;
            double pseudohours = (double)dur/(3600.0 * AV_TIME_BASE);
            qlonglong hours = (qlonglong)floor((double)dur/(3600.0 * AV_TIME_BASE));
            int minutes = (int)floor((dur-AV_TIME_BASE*hours*3600)/(60*AV_TIME_BASE));
            qDebug() << "minutes =" << minutes;
            double seconds = ((double)dur-AV_TIME_BASE*(hours*3600.0+minutes*60.0))/AV_TIME_BASE;
            qDebug() << "getting codec context";
            codecContext = formatContext->streams[0]->codec;
            int samprate = codecContext->sample_rate;
            qDebug() << "sample rate" << samprate;
            qlonglong samps = (qlonglong)dur*samprate/AV_TIME_BASE;
            qDebug() << "samples" << samps;
            int chans = codecContext->channels;
            qDebug() << "channels" << chans;
            int bitrt = codecContext->bit_rate;
            if (bitrt <= 0) {
                qDebug() << "can't determine bitrate";
                if (dur > 0) {
                    qDebug() << "using file size" << allData["File Size"].in << "and positive duration" << dur << "to estimate bitrate";
                    bitrt = (int)(allData["File Size"].in * AV_TIME_BASE / (dur * 0.125)) ;
                }
            }
            qDebug() << "bitrate" << bitrt;
            allData["Duration"] = IntString(QString("%0:%1:%2").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 6, 'f', 3, QChar('0')), dur);
            allData["Sample Rate"] = IntString(QString("%0 Hz").arg(samprate), samprate);
            qDebug() << "samplerate string" << allData["Sample Rate"].st;
            allData["Samples"] = IntString(samps);
            allData["Channels"] = IntString(chans);
            allData["Bit Rate"] = IntString(QString("%0 kb/s").arg((int)(bitrt/1000)), bitrt);
            qDebug() << "bitrate string" << allData["Bit Rate"].st;
        }
    }
    avformat_close_input(&formatContext);
    qDebug() << "all done, returning allData";
    return allData;
}

MediaFile::MediaFile(QString location)
{
    this->location = location;
    allData = MediaFile::getData(location);
    qDebug() << "MediaFile constructed.";
}
