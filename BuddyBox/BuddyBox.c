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
    printf("Initializing Buddy Box...\n");
    
    bb->wireSignal                  = SIGNAL_LOW;
    bb->channelCount                = 0;
    bb->active                      = 1;
    
    bb->localMaxSample              = 0.0f;
    bb->localMaxElapsedCount        = 0;
    bb->sampleCount                 = 0;
    bb->lastSignalEdgeSampleCount   = 0;
    bb->elapsedSampleCounts         = 0;
    bb->currentSignalChannel        = 0;
}

void readBufferIntoBuddyBox(BuddyBox *bb, float* buffer, unsigned int bufferSize)
{
    float tmpLocalMaxSample, bufferSampleMagnitude;
    unsigned int i, tmpLocalMaxElapsedCount;
    
    detectBuddyBoxTimeout(bb, buffer, bufferSize);
    
    tmpLocalMaxSample = 0.0f;
    tmpLocalMaxElapsedCount = 0;
    for (i = 0; bb->active && i < bufferSize; i++)
    {
        bufferSampleMagnitude = getBuddyBoxSampleMagnitude(buffer[i]);
        if (isBuddyBoxSignalEdge(bb, bufferSampleMagnitude))
            processBuddyBoxSignalEdge(bb);
        tmpLocalMaxSample = getBuddyBoxTmpLocalMaxSample(bufferSampleMagnitude, tmpLocalMaxSample);
        tmpLocalMaxElapsedCount = getBuddyBoxTmpLocalMaxElapsedCount(bb, tmpLocalMaxElapsedCount, bufferSize);
        bb->sampleCount++;
    }
    bb->localMaxSample = tmpLocalMaxSample; // Allow local max to change (after a reasonable one is found)
    bb->localMaxElapsedCount = tmpLocalMaxElapsedCount; // Allow local max elapsed to change (after a reasonable one is found)
}

void disconnectBuddyBox(BuddyBox *bb)
{
    bb->active = 0;
    printf("Buddy Box Disconnected...\n");
}

void detectBuddyBoxTimeout(BuddyBox *bb, float* buffer, unsigned int bufferSize)
{
    unsigned int i;
    for (i = 0; i < bufferSize; i++)
        if (buffer[i] != 0.0f)
            return;
    if (!isBuddyBoxCalibrating(bb))
    {
        printf("Timeout...\n");
        disconnectBuddyBox(bb);
    }
}

float getBuddyBoxSampleMagnitude(float sample)
{
    return fabs(sample);
}

float getBuddyBoxTmpLocalMaxSample(float bufferSampleMagnitude, float tmpLocalMaxSample)
{
    return (bufferSampleMagnitude > tmpLocalMaxSample) ? bufferSampleMagnitude : tmpLocalMaxSample;
}

float getBuddyBoxTmpLocalMaxElapsedCount(BuddyBox *bb, unsigned int tmpLocalMaxElapsedCount, unsigned int bufferSize)
{
    return (bb->elapsedSampleCounts > tmpLocalMaxElapsedCount && bb->elapsedSampleCounts < bufferSize) ? bb->elapsedSampleCounts : tmpLocalMaxElapsedCount;
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
    if (bb->lastSignalEdgeSampleCount > bb->sampleCount)
        bb->elapsedSampleCounts = bb->sampleCount + (MAXUINT - bb->lastSignalEdgeSampleCount);
    else
        bb->elapsedSampleCounts = bb->sampleCount - bb->lastSignalEdgeSampleCount;
}

unsigned int isBuddyBoxWireSignalHigh(BuddyBox *bb)
{
    return (bb->wireSignal == SIGNAL_HIGH);
}

void processHighBuddyBoxWireSignal(BuddyBox *bb)
{    
    if (isBuddyBoxSynchroFrameEncountered(bb))
        processBuddyBoxSynchroFrame(bb);
    else if (isBuddyBoxChannelValid(bb))
        targetNextBuddyBoxPacketChannel(bb);
    else if (!isBuddyBoxCalibrating(bb))
    {
        printf("Invalid Channel Received...\n");
        disconnectBuddyBox(bb);
    }
}

unsigned int isBuddyBoxSynchroFrameEncountered(BuddyBox *bb)
{
    return (bb->currentSignalChannel >= MIN_CHANNELS && (bb->elapsedSampleCounts > bb->localMaxElapsedCount / 2));
}

void processBuddyBoxSynchroFrame(BuddyBox *bb)
{
    if (!isBuddyBoxCalibrating(bb))
    {
        if (isBuddyBoxChannelCountValid(bb))
            processBuddyBoxPacket(bb);
        else
        {
            printf("Channel Count has Changed...\n");
            disconnectBuddyBox(bb);
        }
    }
    storeBuddyBoxChannelCount(bb);
    targetNextBuddyBoxPacket(bb);
}

void processBuddyBoxPacket(BuddyBox *bb)
{
    unsigned int i;
    
    for (i = 0; i < MAX_CHANNELS; i++)
        if (i < bb->currentSignalChannel)
            printf("%d\t,", bb->signal[i]);
    printf("%u\n", bb->sampleCount);
}

void storeBuddyBoxChannelCount(BuddyBox *bb)
{
    bb->channelCount = bb->currentSignalChannel - 1;
}

void targetNextBuddyBoxPacket(BuddyBox *bb)
{
    bb->currentSignalChannel = 0;
}

unsigned int isBuddyBoxChannelValid(BuddyBox *bb)
{
    return (bb->currentSignalChannel < MAX_CHANNELS);
}

unsigned int isBuddyBoxCalibrating(BuddyBox *bb)
{
    return (bb->sampleCount < CALIBRATION_SAMPLES);
}

unsigned int isBuddyBoxChannelCountValid(BuddyBox *bb)
{
    return (bb->currentSignalChannel - 1 == bb->channelCount);
}

void targetNextBuddyBoxPacketChannel(BuddyBox *bb)
{
    bb->signal[bb->currentSignalChannel] = bb->elapsedSampleCounts;
    bb->currentSignalChannel++;
}
