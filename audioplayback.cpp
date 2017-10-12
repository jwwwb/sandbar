//
// Created by jwwwb on 2016-10-17.
//

#include "audioplayback.h"
#include <stdio.h>
#include <globals.h>
#include <QDebug>

#define NUM_SECONDS 20

double AudioPlayback::volume = 1.0;

// callback function from http://portaudio.com/docs/v19-doxydocs/writing_a_callback.html

/* This routine will be called by the PortAudio engine when audio is needed.
   It may called at interrupt level on some machines so don't do anything
   that could mess up the system like calling malloc() or free().
*/

static int paPlayCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    paPlayData *callbackData = (paPlayData*)userData;
    (void) inputBuffer; /* Prevent unused variable warning. */

    int_fast16_t *rptr = &callbackData->recordedSamples[callbackData->frameIndex * num_channels];
    int_fast16_t *wptr = (int_fast16_t*)outputBuffer;
    qlonglong i;
    double volume = AudioPlayback::getVolume();
    int finished;
    qlonglong framesLeft = callbackData->maxFrameIndex - callbackData->frameIndex;

    if( framesLeft < framesPerBuffer )
    {
        /* final buffer... */
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = int_fast16_t(*rptr++ * volume);  /* left */
            if( num_channels == 2 ) *wptr++ = int_fast16_t(*rptr++ * volume);  /* right */
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  /* left */
            if( num_channels == 2 ) *wptr++ = 0;  /* right */
        }
        callbackData->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = int_fast16_t(*rptr++* volume);  /* left */
            if( num_channels == 2 ) *wptr++ = int_fast16_t(*rptr++* volume);  /* right */
        }
        callbackData->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

double AudioPlayback::getVolume()
{
    return volume;
}

void AudioPlayback::setVolume(double newVolume)
{
    volume = newVolume;
}

AudioPlayback::AudioPlayback()
{
    // from socapex:
    // LibAV initialisation, do not forget this.
    av_register_all();
    // Libav network initialisation, for streaming sources.
    avformat_network_init();
    // end

    err = Pa_Initialize();
    if( err != paNoError ) printf(  "PortAudio1 error: %s\n", Pa_GetErrorText( err ) );
    paPlayData data = {0, 4096, NULL};

    /* Open an audio I/O stream. */
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
                                paPlayCallback, /* this is your callback function */
                                &this->data ); /*This is a pointer that will be passed to
                                                   your callback*/
    if( err != paNoError ) printf(  "PortAudio2 error: %s\n", Pa_GetErrorText( err ) );
    qDebug() << "stream seems to have been successfully init / opened.";
}

AudioPlayback::~AudioPlayback()
{
    err = Pa_StopStream( stream );
    if( err != paNoError ) printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    err = Pa_Terminate();
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

void AudioPlayback::stopPlay(){
    qDebug() << "pushed stop button, should jump to 5 seconds before the end.";
    data.frameIndex = data.maxFrameIndex - sample_rate*5;
}

void AudioPlayback::playFile(){
    readAudioFile();
}

void AudioPlayback::openFile(const QString &fileName)
{
    // from socapex:
    // Make sure the audio context is free.
    if (audio_context_ != NULL)
        avformat_close_input(&audio_context_);

    // Try and open up the file (check if file exists).
    int error = avformat_open_input(&audio_context_, fileName.toStdString().c_str(),
            NULL, NULL);
    if (error < 0) {
        qDebug() << "Could not open input file." << fileName;
        audio_context_ = NULL;
        return;
    }

    // Read packets of a media file to get stream information.
    if (avformat_find_stream_info(audio_context_, NULL) < 0) {
        qDebug() << "Could not find file info.";
        avformat_close_input(&audio_context_);
        return;
    }

    // We need an audio stream. Note that a stream can contain
    // many channels. Some formats support more streams.
    if (audio_context_->nb_streams != 1) {
        qDebug() << "Expected one audio input stream, but found"
                << audio_context_->nb_streams;
        avformat_close_input(&audio_context_);
        return;
    }

    // If all went well, we have an audio file and have identified a
    // codec. Lets try and find the required decoder.
    codec_ = avcodec_find_decoder(audio_context_->streams[0]->codec->codec_id);
    if (!codec_) {
            qDebug() << "Could not find input codec.";
            avformat_close_input(&audio_context_);
            return;
        }

    // And lets open the decoder.
    error = avcodec_open2(audio_context_->streams[0]->codec,
            codec_, NULL);
    if (error < 0) {
        qDebug() << "Could not open input codec_.";
        avformat_close_input(&audio_context_);
        return;
    }

    // All went well, we store the codec information for later
    // use in the resampler setup.
    codec_context_ = audio_context_->streams[0]->codec;
    qDebug() << "Opened" << codec_->name << "codec_.";
    setupResampler();
}

// socapex again:
/* Set up a resampler to convert our file to
the desired device playback. We set all the parameters we
used when initialising libAO in our constructor. */
void AudioPlayback::setupResampler()
{
    // Prepare resampler.
    if (!(resample_context_ = avresample_alloc_context())) {
        fprintf(stderr, "Could not allocate resample context\n");
        return;
    }

    // The file channels.
    av_opt_set_int(resample_context_, "in_channel_layout",
                   av_get_default_channel_layout(codec_context_->channels), 0);
    // The device channels.
    av_opt_set_int(resample_context_, "out_channel_layout",
                   av_get_default_channel_layout(num_channels), 0);
    // The file sample rate.
    av_opt_set_int(resample_context_, "in_sample_rate",
                   codec_context_->sample_rate, 0);
    // The device sample rate.
    av_opt_set_int(resample_context_, "out_sample_rate",
                   sample_rate, 0);
    // The file bit-depth.
    av_opt_set_int(resample_context_, "in_sample_fmt",
                   codec_context_->sample_fmt, 0);

    // The device bit-depth.
    // FIXME: If you change the device bit-depth, you have to change
    // this value manually.
    av_opt_set_int(resample_context_, "out_sample_fmt",
                   AV_SAMPLE_FMT_S16, 0);

    // And now open the resampler. Hopefully all went well.
    if (avresample_open(resample_context_) < 0) {
        qDebug() << "Could not open resample context.";
        avresample_free(&resample_context_);
        return;
    }
}

// more socapex:

/* Actual audio decoding, resampling and reading. Once a frame is
ready, copy the converted values to libAO's playback buffer. Since
you have access to the raw data, you could hook up a callback and
modify/analyse the audio data here. */
void AudioPlayback::readAudioFile()
{
    // Start at the file beginning.
    avformat_seek_file(audio_context_, 0, 0, 0, 0, 0);

    uint8_t *output; // This is the audio data buffer.
    int out_linesize; // Used internally by libAV.
    int out_samples; // How many samples we will play, AFTER resampling.
    qlonglong out_sample_fmt; // Bit-depth.

    // global (big) buffer
    int_fast16_t * wholeSongBuffer;
    qlonglong duration = audio_context_->duration;
    qDebug() << "Computed duration is" << duration << "or in seconds:" << duration/1000000.0;
    // make it 10% longer than AV_TIME_BASE would suggest just in case.
    qlonglong total_out_samples = (duration*num_channels*sample_rate)/900000;
    qDebug() << "Attemping to malloc" << total_out_samples*sizeof(int_fast16_t) << "Bytes";
    wholeSongBuffer = (int_fast16_t*) malloc (total_out_samples*sizeof(int_fast16_t));
    int_fast16_t *runningPointer = wholeSongBuffer;

    data.frameIndex = 0;
    data.maxFrameIndex = 0; // increment this as we go.
    data.recordedSamples = wholeSongBuffer;

    // We need to use this "getter" for the output sample format.
    av_opt_get_int(resample_context_, "out_sample_fmt", 0, &out_sample_fmt);

    // Initialize all packet values to 0.
    AVPacket pkt = { 0 };
    av_init_packet(&pkt);
    AVFrame* frm = av_frame_alloc();

    bool readingFile_ = true;

    // Loop till file is read.
    qDebug() << "Beginning loop till file is read.";
    int loopCounter = 0;

    while (readingFile_) {
        int gotFrame = 0;

        // Fill packets with data. If no data is read, we are done.
        readingFile_ = !av_read_frame(audio_context_, &pkt);

        // Len was used for debugging purpose. Decode the audio data.
        // Put that in AVFrame.
        int len = avcodec_decode_audio4(codec_context_, frm, &gotFrame,
                &pkt);

        loopCounter++;

        // We can play audio.
        if (gotFrame) {

            // Calculate how many samples we will have after resampling.
            out_samples = avresample_get_out_samples(resample_context_,
                    frm->nb_samples);

            // Allocate our output buffer.
            av_samples_alloc(&output, &out_linesize, num_channels,
                    out_samples, (AVSampleFormat)out_sample_fmt, 0);

            // Resample the audio data and store it in our output buffer.

            out_samples = avresample_convert(resample_context_, &output,
                    out_linesize, out_samples, frm->extended_data,
                    frm->linesize[0], frm->nb_samples);

            // This is why we store out_samples again, some issues may
            // have occured.
            int ret = avresample_available(resample_context_);
            if (ret)
                fprintf(stderr, "%d converted samples left over\n", ret);

            // DEBUG
//            printf("Finished reading Frame len : %d , nb_samples:%d buffer_size:%d line size: %d \n",
//                   len,frm->nb_samples,pkt.size,
//                   frm->linesize[0]);

            // Finally, play the raw data using libAO.
//            ao_play(outputDevice_, (char*)output, out_samples*4);
//            setPlayData(0, out_samples*2, (int_fast16_t *)output);
//            err = Pa_WriteStream( stream, output, out_samples );
//            if( err != paNoError ) printf(  "PortAudio4 error: %s\n", Pa_GetErrorText( err ) );

            // instead of playing, we memcopy to the whole song buffer, so it's all in RAM:
            memcpy((void*)runningPointer, (void*)output, 2*out_samples*sizeof(int_fast16_t));
            runningPointer += 2*out_samples;
            data.maxFrameIndex += out_samples;


        }
        free(output);
    }
    qDebug() << "We're done. Copied" << data.maxFrameIndex << "samples in" << loopCounter << "loops.";

    err = Pa_StartStream( stream );
    if( err != paNoError ) printf(  "PortAudio3 error: %s\n", Pa_GetErrorText( err ) );
    qDebug() << "started stream";
    while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) {

        // somehow I need to listen for signals during this loop so I can modify things.
        Pa_Sleep(100);

    }
    qDebug() << "stream no longer active, so we're stopping.";
    if( err != paNoError ) printf(  "PortAudio4 error: %s\n", Pa_GetErrorText( err ) );
    err = Pa_StopStream( stream );
    if( err != paNoError ) printf(  "PortAudio5 error: %s\n", Pa_GetErrorText( err ) );
    qDebug() << "all done stopping, time for libav cleanup now";

//    err = Pa_CloseStream( stream );
//    if( err != paNoError ) printf(  "PortAudio6 error: %s\n", Pa_GetErrorText( err ) );

    // Because of delays, there may be some leftover resample data.
    int out_delay = avresample_get_delay(resample_context_);

    // Clean it.
    while (out_delay) {
        fprintf(stderr, "Flushed %d delayed resampler samples.\n", out_delay);

        // You get rid of the remaining data by "resampling" it with a NULL
        // input.
        out_samples = avresample_get_out_samples(resample_context_, out_delay);
        av_samples_alloc(&output, &out_linesize, num_channels,
                out_delay, (AVSampleFormat)out_sample_fmt, 0);
        out_delay = avresample_convert(resample_context_, &output, out_linesize,
                out_delay, NULL, 0, 0);
        free(output);
    }

    // Cleanup.
    av_frame_free(&frm);
    av_free_packet(&pkt);

    // my own cleanup:
    free(wholeSongBuffer);

    qDebug() << "Thread stopping.";
}
