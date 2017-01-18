//
// Created by jwwwb on 2017-01-17.
//

#include "mediafile.h"
#include <QDebug>

static QStringList metaDataKeys = (QStringList() << "artistName" << "trackTitle" <<
                                                 "albumTitle" << "date" << "genre" <<
                                                 "albumArtist" << "trackNumber" << "totalTracks" <<
                                                 "discNumber" << "totalDiscs" << "comment");

static QStringList fileDataKeys = (QStringList() << "fileName" << "directoryName" <<
                                                 "filePath" << "fileSize" << "lastModified");

static QStringList mediaDataKeys  = (QStringList() << "duration" << "samples" <<
                                                   "sampleRate" << "channels" << "bitsPerSample" <<
                                                   "bitrate" << "codec" << "encoding" <<
                                                   "encoderTool" << "audioMD5");

MediaFile::MediaFile(std::string location)
{
    this->location = location;
    this->extractData();
}

void MediaFile::extractData()
{
    this->extractFileData();
    this->extractMediaData();
    this->extractFileData();
}

void MediaFile::extractFileData()
{
    std::size_t idx = this->location.rfind('/');
    if(idx != std::string::npos)
    {
        this->allData[QString("fileName")] = this->location.substr(idx+1);
        this->allData[QString("directoryName")] = this->location.substr(0, idx+1);
    } else {
        this->allData[QString("fileName")] = this->location;
        this->allData[QString("directoryName")] = (std::string)"";
    }
    this->allData[QString("filePath")] = this->location;
    this->allData[QString("fileSize")] = (std::string)"";
    this->allData[QString("lastModified")] = (std::string)"";
}

void MediaFile::extractMediaData()
{
    for(QString mediaDataKey : mediaDataKeys) {
        this->allData[mediaDataKey] = (std::string)"";
    }
}

void MediaFile::extractMetaData()
{
    for(QString metaDataKey : metaDataKeys) {
        this->allData[metaDataKey] = (std::string)"";
    }
}
