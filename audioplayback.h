//
// Created by jwwwb on 2016-10-17.
//

#ifndef AUDIOPLAYBACK_H
#define AUDIOPLAYBACK_H

#include "portaudio.h"

typedef struct
{
    float left_phase;
    float right_phase;
} paTestData;

class AudioPlayback
{
    PaError err;
    static paTestData data;
    PaStream *stream;

public:
    AudioPlayback();
    void playSine();
    void closeStream();
};

#endif // AUDIOPLAYBACK_H
