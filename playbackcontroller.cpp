//
// Created by jwwwb on 2017-01-16.
//

#include "playbackcontroller.h"

// pass reference to the model here so I can send slots to that too.
PlaybackController::PlaybackController()
{
    volumeMultiplier = 1.0f;
    currentProgress = 0;
    paused = 0;
    decoder = new AudioDecoder();
    dataPointer = decoder->getDataAddress();
    streamer = new PortAudioInterface(dataPointer);

    // threading it again:
    decoder->moveToThread(&decoderThread);
    connect(&decoderThread, SIGNAL(finished()), decoder, SLOT(deleteLater()));
    connect(&decoderThread, SIGNAL(started()), decoder, SLOT(slotInitialize()));

    // outgoing signals to decoder
    connect(this, SIGNAL(stopSignal()), decoder, SLOT(slotStop()));
    connect(this, SIGNAL(playSignal()), decoder, SLOT(slotPlay()));
    connect(this, SIGNAL(volumeSignal(float)), decoder, SLOT(slotSetVolume(float))); // do directly?
    connect(this, SIGNAL(seekSignal(qlonglong)), decoder, SLOT(slotSeekTo(qlonglong)));

    // incoming signals from decoder
    connect(decoder, SIGNAL(signalDuration(qlonglong)), this, SLOT(slotDuration(qlonglong)));
    connect(decoder, SIGNAL(signalPlaybackProgress(qlonglong)), this, SLOT(slotPlaybackProgress(qlonglong)));

    // signals from decoder to portaudio
    connect(decoder, SIGNAL(bufferReadyForPlayback()), streamer, SLOT(slotEnsureStarted()));
    connect(decoder, SIGNAL(finishedPlaying()), streamer, SLOT(slotEnsureStopped()));
    decoderThread.start();

    // why are these slots? doesn't really matter for now.
    streamer->slotInitialize();
    streamer->slotOpenStream();
}

PlaybackController::~PlaybackController()
{
    // TODO: make streamer a QChild, so destructor is automatic
    streamer->slotEnsureStopped();
    streamer->slotCloseStream();
}

// methods from gui, very much TODO

qlonglong PlaybackController::getDuration()
{
    return (qlonglong)fileDuration;
}

void PlaybackController::pausePushed()
{
    // TODO if file open
    if (paused) {
        streamer->slotStartStream();
        paused = 0;
    } else {
        streamer->slotStopStream();
        paused = 1;
    }
}

void PlaybackController::playPushed()
{
    if (paused) {
        streamer->slotStartStream();
        paused = 0;
    } else {
        emit playSignal();
    }
}

void PlaybackController::stopPushed()
{
    emit stopSignal();
    paused = 0;
}

// methods from gui (or hotkeys)

void PlaybackController::seekDifferential(qlonglong diff)
{
    qlonglong seekto = currentProgress+diff;
    if (seekto > fileDuration) seekto = fileDuration - 2;
    if (seekto < 0) seekto = 0;
    emit seekSignal(seekto);
}

void PlaybackController::seekFile(qlonglong seekto)
{
    emit seekSignal(seekto);
    qDebug() << "        trying to seek to" << seekto;
}

void PlaybackController::setVolume(float volumeMultiplier) {
    this->volumeMultiplier = volumeMultiplier;
    emit volumeSignal(volumeMultiplier);
}

void PlaybackController::setVolume(int decibels) {
    if (decibels > 0) this->volumeMultiplier = 1.0f;
    else {
        this->volumeMultiplier = pow(10.0f, decibels/10.0f);
    }
    emit volumeSignal(volumeMultiplier);
}

// slots from decoder, might be deprecated now?

void PlaybackController::slotDuration(qlonglong duration)
{
    fileDuration = duration;
}

void PlaybackController::slotPlaybackProgress(qlonglong timePlayed)
{
    currentProgress = timePlayed;
}
