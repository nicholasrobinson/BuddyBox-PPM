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

static unsigned int running;

void initializeBuddyBoxThread(PASBuddyBox *pasBB)
{    
    initializePortAudioStream(&pasBB->pas, pasBB->sampleRate);
}

void startBuddyBoxThread(PASBuddyBox *pasBB)
{
    running = 1;
    
    initializeBuddyBox(&pasBB->bb, pasBB->sampleRate);
    
    pthread_create(&pasBB->buddyBoxThread, NULL, runBuddyBoxThread, pasBB);
}

void stopBuddyBoxThread()
{
    running = 0;
}

void joinBuddyBoxThread(PASBuddyBox *pasBB)
{
    pthread_join(pasBB->buddyBoxThread, NULL);
}

void* runBuddyBoxThread(void *arguments)
{
    PASBuddyBox *pasBB = (PASBuddyBox *) arguments;
    
    while(running && pasBB->bb.active && writePortAudioStream(&pasBB->pas) && readPortAudioStream(&pasBB->pas))
    {
        readBufferIntoBuddyBoxInputChannelBuffer(&pasBB->bb, pasBB->pas.bufferedSamples, pasBB->pas.bufferSize);
        
        writeBuddyBoxOutputChannelBufferIntoBuffer(&pasBB->bb, pasBB->pas.bufferedSamples, pasBB->pas.bufferSize);
    }
    
    pthread_exit(0);
}