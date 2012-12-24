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
    bb->wireSignal              = SIGNAL_LOW;
    bb->localMaxSample          = 0.0f;
    bb->sampleCount             = 0;
    bb->signalEdgeSampleCount   = 0;
    bb->elapsedCounts           = 0;
    bb->signalChannel           = 0;
}

void readBufferIntoBuddyBox(BuddyBox *bb, float* buffer, unsigned int bufferSize)
{
    float tmpLocalMaxSample, bufferSampleMagnitude;
    unsigned int i;
    
    tmpLocalMaxSample = 0.0f;
    for (i = 0; i < bufferSize; i++)
    {
        bufferSampleMagnitude = getBuddyBoxSampleMagnitude(buffer[i]);
        if (isBuddyBoxSignalEdge(bb, bufferSampleMagnitude))
            processBuddyBoxSignalEdge(bb);
        tmpLocalMaxSample = getTmpLocalMax(bufferSampleMagnitude, tmpLocalMaxSample);
        bb->sampleCount++;
    }
    bb->localMaxSample = tmpLocalMaxSample; // Allow local max to change (eventually)
}

float getBuddyBoxSampleMagnitude(float sample)
{
    return fabs(sample);
}

float getTmpLocalMax(float bufferSampleMagnitude, float tmpLocalMaxSample)
{
    return (bufferSampleMagnitude > tmpLocalMaxSample) ? bufferSampleMagnitude : tmpLocalMaxSample;
}

unsigned int isBuddyBoxSignalEdge(BuddyBox *bb, float bufferSampleMagnitude)
{
    unsigned int signalEdge;
    
    if (isBuddyBoxSignalHigh(bb, bufferSampleMagnitude))
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

unsigned int isBuddyBoxSignalHigh(BuddyBox *bb, float bufferSampleMagnitude)
{
    return (bufferSampleMagnitude > bb->localMaxSample / 2);
}

void processBuddyBoxSignalEdge(BuddyBox *bb)
{    
    updateBuddyBoxElapsedCounts(bb);
    if (isBuddyBoxWireSignalHigh(bb))
        processHighBuddyBoxWireSignal(bb);
    bb->signalEdgeSampleCount = bb->sampleCount;
}

void updateBuddyBoxElapsedCounts(BuddyBox *bb)
{
    bb->elapsedCounts = bb->sampleCount - bb->signalEdgeSampleCount;
}

unsigned int isBuddyBoxWireSignalHigh(BuddyBox *bb)
{
    return (bb->wireSignal == SIGNAL_HIGH);
}

void processHighBuddyBoxWireSignal(BuddyBox *bb)
{    
    if (isBuddyBoxSynchroFrameEncountered(bb))
        processBuddyBoxPacket(bb);
    else if (bb->signalChannel < MAX_CHANNELS)
        targetNextBuddyBoxPacketChannel(bb);
}

unsigned int isBuddyBoxSynchroFrameEncountered(BuddyBox *bb)
{
    return bb->elapsedCounts > SYNCHRO_FRAME_WIDTH;
}

void processBuddyBoxPacket(BuddyBox *bb)
{
    unsigned int i;
    
    for (i = 0; i < MAX_CHANNELS; i++)
        if (i < bb->signalChannel)
            printf("%d\t,", bb->signals[i]);
    printf("%d\n", bb->sampleCount);
    bb->signalChannel = 0;
}

void targetNextBuddyBoxPacketChannel(BuddyBox *bb)
{
    bb->signals[bb->signalChannel] = bb->elapsedCounts;
    bb->signalChannel++;
}