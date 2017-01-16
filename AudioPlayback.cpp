//
// Created by ych on 17.10.16.
//

#include <stdio.h>
#include "AudioPlayback.h"

#define SAMPLE_RATE (44100)
#define NUM_SECONDS 2


// callback function from http://portaudio.com/docs/v19-doxydocs/writing_a_callback.html

/* This routine will be called by the PortAudio engine when audio is needed.
   It may called at interrupt level on some machines so don't do anything
   that could mess up the system like calling malloc() or free().
*/

static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */

    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = data->left_phase;  /* left */
        *out++ = data->right_phase;  /* right */
        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        data->left_phase += 0.01f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* higher pitch so we can distinguish left and right. */
        data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    return 0;
}

AudioPlayback::AudioPlayback(int x) {
    space_filler = x;
    err = Pa_Initialize();
    if( err != paNoError ) printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    static paTestData data;
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paFloat32,  /* 32 bit floating point output */
                                SAMPLE_RATE,
                                256,        /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                                patestCallback, /* this is your callback function */
                                &data ); /*This is a pointer that will be passed to
                                                   your callback*/
    if( err != paNoError ) printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
}

void AudioPlayback::playSine() {
    err = Pa_StartStream( stream );
    if( err != paNoError ) printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    /* Sleep for several seconds. */
    Pa_Sleep(NUM_SECONDS*1000);
    err = Pa_StopStream( stream );
    if( err != paNoError ) printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
}

void AudioPlayback::closeStream(){
    err = Pa_CloseStream( stream );
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    err = Pa_Terminate();
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
}





