//
// Created by jwwwb on 2017-01-17.
//

#ifndef GLOBALS_H
#define GLOBALS_H
extern "C" {
#include <libavutil/opt.h>
}


const double sample_rate = 44100.0;
const int num_channels = 2;
const int num_buffers = 4;
const double outSamplesToUSec = 1000000.0/sample_rate;
const double uSecToOutSamples = sample_rate/1000000.0;
const double avTimeBaseToUSec = 1.0;
const double uSecToAvTimeBase = 1.0;
const double uSecToSliderStep = 1.0/100000.0;
const double sliderStepToUSec = 100000.0;

const int samples_per_buffer = 65536; // might be too big, who cares, it's just 64k
const int cache_duration = samples_per_buffer*outSamplesToUSec; // in usec.
const int timer_period = (int)(cache_duration/10000.0); // in msec (thanks Qt)


#endif // GLOBALS_H
