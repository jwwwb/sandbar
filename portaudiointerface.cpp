//
// Created by jwwwb on 2017-01-25.
//

#include "portaudiointerface.h"
#include <QDebug> // temporary, this should NOT be doing IO.

static int paQueueCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    bufferQueue* data = (bufferQueue*)userData;
    if (data->switchBuffer) { // controller has requested we skip to a new buffer
        data->readIndex = 0;
        data->readBuffer = data->nextReadBuffer;
        data->switchBuffer = 0;
    }
    SAMPLE* wptr = (SAMPLE*)outputBuffer;
    SAMPLE* rptr = &(data->buffer[data->readBuffer][data->readIndex*num_channels]);
    float volume = data->volume;
    int finished;
    uint32_t framesLeft = data->bufferLen[data->readBuffer] - data->readIndex;
    qlonglong i;
    // if we're in the start or middle of stream buffer
    if( framesLeft >= framesPerBuffer ) {
        for( i=0; i<framesPerBuffer; i++ ) {
            *wptr++ = (SAMPLE)(*rptr++ * volume); // left
            *wptr++ = (SAMPLE)(*rptr++ * volume); // right
        }
        data->readIndex += framesPerBuffer;
        finished = paContinue;
    } else { // near the end of the stream buffer
        // first finish up the rest of the buffer, then check how to continue
        for( i=0; i<framesLeft; i++ ) {
            *wptr++ = (SAMPLE)(*rptr++ * volume); // left
            *wptr++ = (SAMPLE)(*rptr++ * volume); // right
        }
        // if there's another buffer ready to continue on
        if (data->nextReadBuffer) {
            data->readBuffer = data->nextReadBuffer;
            data->readIndex = 0;
            rptr = data->buffer[data->readBuffer];
            for( ; i<framesPerBuffer; i++ )
            {
                *wptr++ = (SAMPLE)(*rptr++ * volume); // left
                *wptr++ = (SAMPLE)(*rptr++ * volume); // right
            }
            data->readIndex = framesPerBuffer - framesLeft;
            finished = paContinue;
        } else {    // playback stop has been requested, finish up.
            for( ; i<framesPerBuffer; i++ )
            {
                *wptr++ = 0;  // left
                *wptr++ = 0;  // right
            }
            data->readIndex += framesLeft;
            finished = paComplete;
        }
    }
    return finished;
}

PortAudioInterface::PortAudioInterface(bufferQueue *buffer)
{
    dataPointer = buffer;
}

void PortAudioInterface::slotEnsureStarted()
{
    if (Pa_IsStreamStopped( stream )) {
        err = Pa_StartStream( stream );
        if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    } else {
        qDebug() << "stream is already running.";
    }
}

void PortAudioInterface::slotEnsureStopped()
{
    if (!Pa_IsStreamStopped( stream )) {
        err = Pa_StopStream( stream );
        if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    } else {
        qDebug() << "stream is already stopped.";
    }
}

void PortAudioInterface::slotInitialize()
{
    // very first thing that needs to happen
    err = Pa_Initialize();
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
}

void PortAudioInterface::slotOpenStream()
{
    qDebug() << "PAI opening stream";
    err = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paInt16,    /* 16 bit signed int output */
                                sample_rate,
                                256,        /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                                paQueueCallback, /* this is your callback function */
                                dataPointer ); /*This is a pointer that will be passed to
                                                   your callback*/
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
}

void PortAudioInterface::slotStartStream()
{
    err = Pa_StartStream( stream );
    qDebug() << "PAI starting (resuming) stream";
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
}

void PortAudioInterface::slotStopStream()
{
    err = Pa_StopStream( stream );
    qDebug() << "PAI stopping (pausing) stream";
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
}

void PortAudioInterface::slotAbortStream()
{
    err = Pa_AbortStream( stream );
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
}

void PortAudioInterface::slotCloseStream()
{
    err = Pa_CloseStream( stream );
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
}

void PortAudioInterface::slotTerminate()
{
    err = Pa_Terminate();
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));
}
