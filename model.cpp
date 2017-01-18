#include "model.h"

Model::Model()
{

}

void Model::addMediaFile(MediaFile *mediaFile)
{
    this->playlist.push_back(*mediaFile);
}

void Model::clearPlaylist()
{
    this->playlist.clear();
}

MediaFile::MediaFile(std::string location)
{
    this->location = location;
    this->extractMetaData();
}

MediaFile::MediaFile(std::string location, std::string fileName, std::string directory)
{
    this->location = location;
    this->fileName = fileName;
    this->directory = directory;
}

void MediaFile::extractMetaData()
{
    std::size_t idx = this->location.rfind('/');
    if(idx != std::string::npos)
    {
        this->fileName = this->location.substr(idx+1);
        this->directory = this->location.substr(0, idx);
    } else {
        this->directory = "/no/directory/found/";
        this->fileName = "no_file.found";
    }
}
