//
//  BuddyBoxThread.c
//  BuddyBox-PPM
//
//  Created by Nicholas Robinson on 1/10/13.
//  Copyright (c) 2013 Nicholas Robinson. All rights reserved.
//

#include "BuddyBoxThread.h"
#include "PortAudioStream.h"
#include "BuddyBox.h"

#include <signal.h>

void initializeBuddyBoxThread(PASBuddyBox *pasBB)
{    
    initializePortAudioStream(&pasBB->pas, pasBB->sampleRate);
    pasBB->running = 0;
}

void startBuddyBoxThread(PASBuddyBox *pasBB)
{
    pasBB->running = 1;
    
    initializeBuddyBox(&pasBB->bb, pasBB->sampleRate);
    
    pthread_create(&pasBB->buddyBoxThread, NULL, runBuddyBoxThread, pasBB);
}

void stopBuddyBoxThread(PASBuddyBox *pasBB)
{
    pasBB->running = 0;
}

void joinBuddyBoxThread(PASBuddyBox *pasBB)
{
    pthread_join(pasBB->buddyBoxThread, NULL);
}

void cleanupBuddyBoxThread(PASBuddyBox *pasBB)
{
    closePortAudioStream(&pasBB->pas);
}

void* runBuddyBoxThread(void *arguments)
{
    PASBuddyBox *pasBB = (PASBuddyBox *) arguments;
    
    while(pasBB->running && pasBB->bb.active && writePortAudioStream(&pasBB->pas) && readPortAudioStream(&pasBB->pas))
    {
        readBufferIntoBuddyBoxInputChannelBuffer(&pasBB->bb, pasBB->pas.bufferedSamples, pasBB->pas.bufferSize);
        
        writeBuddyBoxOutputChannelBufferIntoBuffer(&pasBB->bb, pasBB->pas.bufferedSamples, pasBB->pas.bufferSize);
    }
    pasBB->running = 0;
    
    pthread_exit(0);
}

unsigned int isBuddyBoxThreadRunning(PASBuddyBox *pasBB)
{
    return pasBB->running;
}

unsigned int isBuddyBoxThreadCalibrated(PASBuddyBox *pasBB)
{
    return !isBuddyBoxInputCalibrating(&pasBB->bb);
}

void setBuddyBoxThreadOutputChannelCount(PASBuddyBox *pasBB, unsigned int channelCount)
{
    pasBB->bb.outputChannelCount = channelCount;
}

void setBuddyBoxThreadOutputChannelValue(PASBuddyBox *pasBB, unsigned int channel, float channelValue)
{
    setBuddyBoxOutputChannelValue(&pasBB->bb, channel, channelValue);
}

unsigned int getBuddyBoxThreadInputChannelCount(PASBuddyBox *pasBB)
{
    return pasBB->bb.inputChannelCount;
}

float getBuddyBoxThreadInputChannelValue(PASBuddyBox *pasBB, unsigned int channel)
{
    return pasBB->bb.inputChannelValues[channel];
}
