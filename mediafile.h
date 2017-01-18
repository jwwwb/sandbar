//
// Created by jwwwb on 2017-01-17.
//

#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <map>
#include <string>
#include <QStringList>
#include <QString>

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

enum fileData {
    fileName,
    directoryName,
    filePath,
    fileSize,
    lastModified
};

enum mediaData {
    duration,
    samples,
    sampleRate,
    channels,
    bitsPerSample,
    bitrate,
    codec,
    encoding,
    encoderTool,
    audioMD5
};

class MediaFile
{
public:
    MediaFile(std::string);
    std::string location;
    std::map<QString, std::string> allData;

private:
    void extractData();
    void extractMetaData();
    void extractMediaData();
    void extractFileData();
};

#endif // MEDIAFILE_H
