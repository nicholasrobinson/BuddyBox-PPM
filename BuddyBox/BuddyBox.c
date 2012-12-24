//
//  BuddyBox.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include "BuddyBox.h"
#include <stdio.h>
#include <math.h>

void initializeBuddyBox(BuddyBox *bb)
{
    printf("Initializing Buddy Box\n");
    
    bb->wireSignal                  = SIGNAL_LOW;
    bb->active                      = 1;
    
    bb->localMaxSample              = 0.0f;
    bb->sampleCount                 = 0;
    bb->lastSignalEdgeSampleCount   = 0;
    bb->elapsedSampleCounts         = 0;
    bb->currentSignalChannel        = 0;
}

void readBufferIntoBuddyBox(BuddyBox *bb, float* buffer, unsigned int bufferSize)
{
    float tmpLocalMaxSample, bufferSampleMagnitude;
    unsigned int i;
    
    tmpLocalMaxSample = 0.0f;
    for (i = 0; bb->active && i < bufferSize; i++)
    {
        bufferSampleMagnitude = getBuddyBoxSampleMagnitude(buffer[i]);
        if (isBuddyBoxSignalEdge(bb, bufferSampleMagnitude))
            processBuddyBoxSignalEdge(bb);
        tmpLocalMaxSample = getBuddyBoxTmpLocalMax(bufferSampleMagnitude, tmpLocalMaxSample);
        bb->sampleCount++;
    }
    bb->localMaxSample = tmpLocalMaxSample; // Allow local max to change (after a reasonable one is found)
}

void disconnectBuddyBox(BuddyBox *bb)
{
    bb->active = 0;
    printf("Buddy Box Disconnected\n");
}

float getBuddyBoxSampleMagnitude(float sample)
{
    return fabs(sample);
}

float getBuddyBoxTmpLocalMax(float bufferSampleMagnitude, float tmpLocalMaxSample)
{
    return (bufferSampleMagnitude > tmpLocalMaxSample) ? bufferSampleMagnitude : tmpLocalMaxSample;
}

unsigned int isBuddyBoxSignalEdge(BuddyBox *bb, float bufferSampleMagnitude)
{
    unsigned int signalEdge;
    
    if (isBuddyBoxRawSignalHigh(bb, bufferSampleMagnitude))
    {
        signalEdge = (bb->wireSignal == SIGNAL_LOW) ? 1 : 0;
        bb->wireSignal = SIGNAL_HIGH;
    }
    else
    {
        signalEdge = (bb->wireSignal == SIGNAL_HIGH) ? 1 : 0;
        bb->wireSignal = SIGNAL_LOW;
    }
    
    return signalEdge;
}

unsigned int isBuddyBoxRawSignalHigh(BuddyBox *bb, float bufferSampleMagnitude)
{
    return (bufferSampleMagnitude > bb->localMaxSample / 2);
}

void processBuddyBoxSignalEdge(BuddyBox *bb)
{    
    updateBuddyBoxElapsedCounts(bb);
    if (isBuddyBoxWireSignalHigh(bb))
        processHighBuddyBoxWireSignal(bb);
    bb->lastSignalEdgeSampleCount = bb->sampleCount;
}

void updateBuddyBoxElapsedCounts(BuddyBox *bb)
{
    bb->elapsedSampleCounts = bb->sampleCount - bb->lastSignalEdgeSampleCount;
}

unsigned int isBuddyBoxWireSignalHigh(BuddyBox *bb)
{
    return (bb->wireSignal == SIGNAL_HIGH);
}

void processHighBuddyBoxWireSignal(BuddyBox *bb)
{    
    if (isBuddyBoxSynchroFrameEncountered(bb))
    {
        outputBuddyBoxSignal(bb);
        processNextBuddyBoxPacket(bb);
    }
    else if (bb->currentSignalChannel < MAX_CHANNELS)
        targetNextBuddyBoxPacketChannel(bb);
    else
        disconnectBuddyBox(bb);
}

unsigned int isBuddyBoxSynchroFrameEncountered(BuddyBox *bb)
{
    return (bb->elapsedSampleCounts > SYNCHRO_FRAME_WIDTH);
}

void outputBuddyBoxSignal(BuddyBox *bb)
{
    unsigned int i;
    
    for (i = 0; i < MAX_CHANNELS; i++)
        if (i < bb->currentSignalChannel)
            printf("%d\t,", bb->signal[i]);
    printf("%d\n", bb->sampleCount);
}

void processNextBuddyBoxPacket(BuddyBox *bb)
{
    bb->currentSignalChannel = 0;
}

void targetNextBuddyBoxPacketChannel(BuddyBox *bb)
{
    bb->signal[bb->currentSignalChannel] = bb->elapsedSampleCounts;
    bb->currentSignalChannel++;
}
