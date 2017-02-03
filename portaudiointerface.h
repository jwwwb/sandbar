//
// Created by jwwwb on 2017-01-25.
//

#ifndef PORTAUDIOINTERFACE_H
#define PORTAUDIOINTERFACE_H

#include <QObject>
#include "portaudio.h"
#include "globals.h"

/* *******
I'm kinda wondering right now if this thing even needs to be
in its own thread anymore at this point? It doesn't really do
anything except hold a pointer to a stream, and occasionally
send some small messages to it. And define the callback, but
that runs in its own process, I'm pretty sure.
******* */

typedef int_fast16_t SAMPLE;

typedef struct {
    // be very careful with sizes, because:
    SAMPLE *buffer [4];     // each item in a buffer is 16 bits
    uint32_t bufferLen [4];     // but the len is measured in multiples of 32bit
    uint32_t readIndex; // and these indices also indicate 32 bit jumps
    uint8_t readBuffer, nextReadBuffer, writeBuffer, switchBuffer;
    qlonglong positionInFile [4];    // now indicates position in usecs.
    float trackTimeBaseToOutSampleIndex;
    float outSampleIndexToTrackTimeBase;
    float trackTimeBaseToUSec;
    float uSecToTrackTimeBase;
    float volume;
} bufferQueue;

class PortAudioInterface : public QObject
{
    Q_OBJECT
public:
    PortAudioInterface(bufferQueue *);

private:
    bufferQueue *dataPointer;
    PaError err;
    PaStream *stream;

public slots:
    // not sure why they are slots... well, apart from the first
    void slotEnsureStarted();
    void slotEnsureStopped();
    void slotInitialize();
    void slotOpenStream();
    void slotStartStream();
    void slotStopStream();
    void slotAbortStream(); // last resort
    void slotCloseStream();
    void slotTerminate();
    // do I pause playback here or in the decoder? I think here.

    // no signals, I think
};

#endif // PORTAUDIOINTERFACE_H
