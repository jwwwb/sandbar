//
// Created by jwwwb on 2017-01-17.
//

#ifndef GLOBALS_H
#define GLOBALS_H


const double sample_rate = 44100.0;
const int num_channels = 2;
const int num_buffers = 4;
const int timeBase = 1000000;   // my own timebase, just so happens to be the same as AV's
const double outSamplesToUSec = 1000000.0/sample_rate;
const double uSecToOutSamples = sample_rate/1000000.0;
const double avTimeBaseToUSec = 1.0;
const double uSecToAvTimeBase = 1.0;
const double uSecToSliderStep = 1.0/100000.0;
const double sliderStepToUSec = 100000.0;

const int samples_per_buffer = 262144; // might be too big, who cares, it's just 64k
const int cache_duration = samples_per_buffer*outSamplesToUSec; // in usec.
//const int timer_period = (int)(cache_duration/10000.0); // in msec (thanks Qt)
const int timer_period = 200;   // in msec

#endif // GLOBALS_H
