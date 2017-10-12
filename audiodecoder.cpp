//
// Created by jwwwb on 2017-01-25.
//

#include "audiodecoder.h"
#include <qDebug>

AudioDecoder::AudioDecoder()
{
    // this class deliberately left empty
    // make sure to call initalize after moving it to its thread.
}

AudioDecoder::~AudioDecoder() {
    free(output);
}

bufferQueue * AudioDecoder::getDataAddress()
{
    return &bufferData;
}

// public slots

void AudioDecoder::slotInitialize()
{
    qDebug() << "seeing if we can initialize";
    if (!initialized) {
        av_register_all();
        avformat_network_init();
        fileOpen = 0;
        currentTime = 0;
        bufferData.volume = 1.0;
        // initialize with the libav default value, change on each load
        bufferData.outSampleIndexToTrackTimeBase = (float)AV_TIME_BASE/(float)sample_rate;
        bufferData.trackTimeBaseToOutSampleIndex = (float)sample_rate/(float)AV_TIME_BASE;
        bufferData.uSecToTrackTimeBase = 1.0;
        bufferData.trackTimeBaseToUSec = 1.0;
        // zeros for all of these
        bufferData.readIndex = 0;
        bufferData.readBuffer = 0;
        bufferData.nextReadBuffer = 0;
        bufferData.switchBuffer = 0;
        bufferData.writeBuffer = 0;
        bufferData.buffer[0] = NULL;
        bufferData.bufferLen[0] = 0;
        bufferData.positionInFile[0] = 0;
        bufferData.fileChangeIndex = -1;
        for (int i=1 ; i<4 ; i++) {
            // doing this so I can throw away and re-alloc individual buffers while keeping others
            // malloc is allocating: 1 (second) * 2 (channels) * 44100 (Hz) * 16 (bits) = 172 kBytes
            bufferData.buffer[i] = (SAMPLE*)malloc((int)(cache_duration*outSamplesToUSec*num_channels*sizeof(SAMPLE)));
            bufferData.bufferLen[i] = 0;
            bufferData.positionInFile[i] = 0;
        }
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(slotTryToProgress()));
        timer->start(timer_period);
        initialized = 1;
        qDebug() << "initialized!";
    }
}

void AudioDecoder::slotFile(const QString &fileName)
{
    qDebug() << "received filename" << fileName << ", loading file.";
        if (fileOpen) {
        qDebug() << "File is open right now, so finishing file first";
        finishFile();
    }
    if (!timer->isActive()) {
        timer->start(timer_period);
    }
    qlonglong dur = loadFile(fileName);
    nextDuration = dur;
    emit signalDuration(dur);
    qDebug() << "playing back file" << fileName << "of length" << dur;
}

void AudioDecoder::slotFileSoon(const QString &fileName) {
    qDebug() << "received filename" << fileName << ", loading file.";
        if (fileOpen) {
        qDebug() << "File is open right now, so cleaning up old file";
        cleanUpFile();
    }
    if (!timer->isActive()) {
        timer->start(timer_period);
    }
    qlonglong dur = loadFile(fileName);
    // TODO: don't emit the duration until the other file has finished playing.
    nextDuration = dur;
//    emit signalDuration(dur);
    qDebug() << "playing back file" << fileName << "of length" << dur;
}

void AudioDecoder::slotPlay()
{
    //
    qDebug() << "decoder received play signal";
    if (fileOpen) {
        slotSeekTo(0);
        qDebug() << "file already open, seeking to start";
    } else {
        emit signalRequestFile();
        timer->start(timer_period);
        qDebug() << "no file open, requested file name.";
    }
}

void AudioDecoder::slotSeekTo(qlonglong time)
{
    // TODO figure out why the hell some mp3s allegedly don't have headers
    // and why I can only seek in the very beginning of some mp3s
    int flags = 0;
//    flags += AVSEEK_FLAG_FRAME;
    flags += AVSEEK_FLAG_ANY;
    if (time < getTime()) {
        flags += AVSEEK_FLAG_BACKWARD;
    }

    int desired_seek = (int)(time*bufferData.uSecToTrackTimeBase);
    int min_seek = (int)((time-2*cache_duration)*bufferData.uSecToTrackTimeBase);
    avformat_seek_file(formatContext, audioStream, min_seek, desired_seek,
                       desired_seek, flags);
    fillUpBuffer(1);
}

void AudioDecoder::slotSetVolume(float newVolume)
{
    // may have some lag? not guaranteed to be atomic, since 32 bit dtype:
    // http://preshing.com/20130618/atomic-vs-non-atomic-operations/
    bufferData.volume = newVolume;
}

void AudioDecoder::slotStop()
{
    qDebug() << "decoder received stop signal";
    if (fileOpen) {
        qDebug() << "file is currently open, so stopping everything.";
        bufferData.nextReadBuffer = 0;
        finishFile();
    }
    if (timer->isActive()) {
        qDebug() << "definitely stopping timer";
        timer->stop();
    } else {
        qDebug() << "timer is already stopped??";
    }
}

/* just checks whether the next buffer can be filled and fills it */
void AudioDecoder::slotTryToProgress()
{
    if (fileOpen) {
        emit signalPlaybackProgress(getTime());
        // if we're playing the most recent buffer already
        if (bufferData.readBuffer == bufferData.nextReadBuffer) {
            fillUpBuffer(0);
        }
    }
}

// private methods

void AudioDecoder::cleanUpFile()
{
    // File complete, clean up leftover resample data.
    int out_delay = avresample_get_delay(resampleContext);
    while (out_delay) {
        fprintf(stderr, "Flushed %d delayed resampler samples.\n", out_delay);
        out_samples = avresample_get_out_samples(resampleContext, out_delay);
        av_samples_alloc(&output, &out_linesize, num_channels,
                out_delay, (AVSampleFormat)out_sample_fmt, 0);
        out_delay = avresample_convert(resampleContext, &output, out_linesize,
                out_delay, NULL, 0, 0);
        free(output);
    }
    av_frame_free(&frm);
    av_free_packet(&pkt);
    // set local variables
}

void AudioDecoder::finishFile()
{
    bufferData.nextReadBuffer = 0; // switch it to playing 0s
    bufferData.switchBuffer = 1;
    cleanUpFile();
    fileOpen = 0;
    emit finishedPlaying(); // this one stops the streamer
    qDebug() << "file finished (forced or naturally), set fileOpen to 0";
}

void AudioDecoder::fillUpBuffer(int switchBuffer = 0)
{
    if (!fileOpen) return;
    // increment only if the current buffer has already been used.
    if (bufferData.writeBuffer == bufferData.nextReadBuffer) bufferData.writeBuffer++;
    if (bufferData.writeBuffer > 3) bufferData.writeBuffer = 1;
    bufferData.bufferLen[bufferData.writeBuffer] = 0;
    int first_loop = 0;
    int samples_put_into_buffer = 0;

    // so what's the deal with 4096 samples? does this mean 4096 mono samples? so 2048 stereo ones?
    // actually no, it means 4096 stereo samples, aka 16kB. Our buffer is 172 kBytes, so we can fit about 10.

    int samplesRead = 4096; // this is a bit tricky... we need to stop before a sample doesn't fit in
    // this code only works assuming constant packet size, though we'll add in a bit of extra margin
    while (samples_put_into_buffer < cache_duration*uSecToOutSamples - 4096) { // just to be safe.
        samplesRead = 0;
        while (samplesRead == 0) {
            samplesRead = readNextPacket();
        }
        if (samplesRead < 0) {
            // file complete.
            // TODO: don't finish the file so abruptly, wait for it to finish playing. Currently chops off the last 7 seconds or so.
//            finishFile();
            cleanUpFile();
            qDebug() << "file finished naturally, requesting next file";
            emit signalFileReadEnded(); // this one requests a new file, don't do this after stop button
            // the rest of finishFile still has to occur here, but only if file ended triggers a "signalNoMoreFiles"
            return;
        } else {
            // get the WB'th buffer, and jump to cur_buf_len*2 (so after one loop, 8192*16bit)
            SAMPLE *dest = &(bufferData.buffer[bufferData.writeBuffer][bufferData.bufferLen[bufferData.writeBuffer]*num_channels]);
            // we also copy 2Bytes * 2channels * 4096 samples, so again 16kBytes.
            memcpy((void*)dest, (void*)output, num_channels*samplesRead*sizeof(SAMPLE));
            free(output);
            // increment everything by 4096
            bufferData.bufferLen[bufferData.writeBuffer] += samplesRead;
            samples_put_into_buffer += samplesRead;
        }
        if (!first_loop) {
            first_loop++;
            currentTime = (qlonglong)(av_frame_get_best_effort_timestamp(frm) * bufferData.trackTimeBaseToUSec);
            bufferData.positionInFile[bufferData.writeBuffer] = currentTime;
            bufferData.nextReadBuffer = bufferData.writeBuffer; // only set if at least one package was read.
            if (switchBuffer) bufferData.switchBuffer = 1;
        }
    }
}

qlonglong AudioDecoder::getTime()
{
    return (qlonglong)(bufferData.positionInFile[bufferData.readBuffer] + bufferData.readIndex * outSamplesToUSec);
}

qlonglong AudioDecoder::loadFile(const QString &fileName) {
    qlonglong duration;
    duration = enqueueFile(fileName);
    bufferData.switchBuffer = 1;
    return duration;
}

qlonglong AudioDecoder::enqueueFile(const QString &fileName) {
    if (formatContext != NULL)
        avformat_close_input(&formatContext);
    // Try and open up the file (check if file exists).
    int error = avformat_open_input(&formatContext,
                fileName.toStdString().c_str(), NULL, NULL);
    if (error < 0) {
        qDebug() << "Could not open input file." << fileName;
        qDebug() << "Error code:" << error;
        formatContext = NULL;
        return 0;
    }
    // Read packets of a media file to get stream information.
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        qDebug() << "Could not find file info.";
        avformat_close_input(&formatContext);
        return 0;
    }

    // If there are multiple streams in the file, find the best one
    // TODO what is the best one?
    int best = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    qDebug() << "best stream is apparently" << best;

    for (int i=0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            audioStream = i;
            qDebug() << "found audiostream in stream" << audioStream;
            break;
        }
    }
    if (audioStream == -1) {
        qDebug() << "Could not find one audio input stream, among the"
                << formatContext->nb_streams;
        avformat_close_input(&formatContext);
        return 0;
    }

    // If all went well, we have an audio file and have identified a
    // codec. Lets try and find the required decoder.
    deCodec = avcodec_find_decoder(formatContext->streams[audioStream]->codec->codec_id);
    if (!deCodec) {
        qDebug() << "Could not find input codec.";
        avformat_close_input(&formatContext);
        return 0;
    }

    // And lets open the decoder.
    error = avcodec_open2(formatContext->streams[audioStream]->codec,
            deCodec, NULL);
    if (error < 0) {
        qDebug() << "Could not open input deCodec.";
        avformat_close_input(&formatContext);
        return 0;
    }
    // All went well, we store the codec information for later
    // use in the resampler setup.
    codecContext = formatContext->streams[audioStream]->codec;
    qDebug() << "Opened" << deCodec->name << "codec_.";

    // Prepare resampler.
    if (!(resampleContext = avresample_alloc_context())) {
        qDebug() <<  "Could not allocate resample context";
        return 0;
    }
    // The file channels.
    av_opt_set_int(resampleContext, "in_channel_layout",
                   av_get_default_channel_layout(codecContext->channels), 0);
    // The device channels.
    av_opt_set_int(resampleContext, "out_channel_layout",
                   av_get_default_channel_layout(num_channels), 0);
    // The file sample rate.
    av_opt_set_int(resampleContext, "in_sample_rate",
                   codecContext->sample_rate, 0);
    // The device sample rate.
    av_opt_set_int(resampleContext, "out_sample_rate",
                   sample_rate, 0);
    // The file bit-depth.
    av_opt_set_int(resampleContext, "in_sample_fmt",
                   codecContext->sample_fmt, 0);

    // The device bit-depth.
    // FIXME: If you change the device bit-depth, you have to change
    // this value manually.
    av_opt_set_int(resampleContext, "out_sample_fmt",
                   AV_SAMPLE_FMT_S16, 0);
    // And now open the resampler. Hopefully all went well.
    if (avresample_open(resampleContext) < 0) {
        qDebug() << "Could not open resample context.";
        avresample_free(&resampleContext);
        return 0;
    }
    // Start at the file beginning.
    avformat_seek_file(formatContext, audioStream, 0, 0, 0, 0);

    // We need to use this "getter" for the output sample format.
    av_opt_get_int(resampleContext, "out_sample_fmt", 0, &out_sample_fmt);

//    pkt = { 0 };
    av_init_packet(&pkt);
    frm = av_frame_alloc();
    currentTime = 0;
    bufferData.trackTimeBaseToUSec = (float)formatContext->streams[audioStream]->time_base.num /
            (float)formatContext->streams[audioStream]->time_base.den * 1000000.0;
    bufferData.uSecToTrackTimeBase = (float)formatContext->streams[audioStream]->time_base.den /
            (float)formatContext->streams[audioStream]->time_base.num / 1000000.0;
    bufferData.trackTimeBaseToOutSampleIndex = (float)formatContext->streams[audioStream]->time_base.num /
            (float)formatContext->streams[audioStream]->time_base.den * sample_rate;
    bufferData.outSampleIndexToTrackTimeBase = (float)formatContext->streams[audioStream]->time_base.den /
            (float)formatContext->streams[audioStream]->time_base.num / sample_rate;

    fileOpen = 1;
    qDebug() << "file loaded successfully, setting fileOpen to 1";

    // start reading the file and then tell portaudio to start playing it
    fillUpBuffer(0);

    duration = formatContext->duration;
    emit bufferReadyForPlayback();
    return duration; // number of samples the song is long
}

/* returns the number of samples read, which in the default case are 32 bit or 4 Byte
 * wide (2 channels * 16 bit int). It seems like the normal packet size is 4096 samples,
 * or 16 kiloBytes, at least for flac - aka about 0.1 seconds.
 */
int AudioDecoder::readNextPacket()
{
    int out_samples = 0;
    int gotFrame = 0;

    // Fill packets with data. If no data is read, we are done.
    if (av_read_frame(formatContext, &pkt)) {
        return -1;
    }
    // Decode the audio data. Put that in AVFrame.
    avcodec_decode_audio4(codecContext, frm, &gotFrame, &pkt);

    // No frame was decoded (doesn't have to be an error, can just be latency).
    if (!gotFrame) {
        return 0;
    }

    // Calculate how many samples we will have after resampling.
    out_samples = avresample_get_out_samples(resampleContext,
            frm->nb_samples);

    // Allocate our output buffer.
    // this motherfucker wants &output to be a uint8_t**, that's why
    // I can't make my output buffer be the type it actually is.
    av_samples_alloc(&output, &out_linesize, num_channels,
            out_samples, (AVSampleFormat)out_sample_fmt, 0);

    // Resample the audio data and store it in our output buffer.

    out_samples = avresample_convert(resampleContext, &output,
            out_linesize, out_samples, frm->extended_data,
            frm->linesize[0], frm->nb_samples);

    // This is why we store out_samples again, some issues may
    // have occured.
    int ret = avresample_available(resampleContext);
    if (ret) {
        fprintf(stderr, "%d converted samples left over\n", ret);
    }
    return out_samples;
    // out_samples includes the num_channels already
}
