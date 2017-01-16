//
// Created by ych on 17.10.16.
//

#ifndef SANDBAR_AUDIOPLAYBACK_H
#define SANDBAR_AUDIOPLAYBACK_H

#include "portaudio.h"

typedef struct
{
    float left_phase;
    float right_phase;
} paTestData;

class AudioPlayback
{
    int space_filler;
    PaError err;
    static paTestData data;
    PaStream *stream;

public:
    AudioPlayback(int x);
    void playSine();
    void closeStream();

};


#endif //SANDBAR_AUDIOPLAYBACK_H
