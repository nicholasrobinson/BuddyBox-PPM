//
//  main.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "PortAudioStream.h"
#include "BuddyBox.h"

static const unsigned int DEFAULT_SAMPLE_RATE = 124000;

static unsigned int running = 1;

void intHandler(int sig) {
    running = 0;
}

void generateOutput(BuddyBox *bb)
{
    unsigned int i;
    
    bb->outputChannelCount = 9;
    for (i = 0; i < bb->outputChannelCount; i++)
        setBuddyBoxOutputChannelValue(bb, i, rand() % 800 / 1000.0f + 0.1f);
}

void displayInput(BuddyBox *bb)
{
    unsigned int i;
    
    if (bb->active && !isBuddyBoxInputCalibrating(bb))
    {
        printf("%u - ", bb->inputChannelCount);
        for (i = 0; i < bb->inputChannelCount; i++)
            printf("%f\t,", bb->inputChannelValues[i]);
        printf("%u\n", bb->inputSynchroFrameCount);
    }
}

int main(int argc, const char * argv[])
{
    PortAudioStream pas;
    BuddyBox bb;
    unsigned int sampleRate;
    
    sampleRate = (argc > 1) ? (unsigned int) strtol(argv[1], NULL, 0) : DEFAULT_SAMPLE_RATE;
    
    signal(SIGKILL, intHandler);
    signal(SIGINT, intHandler);
    
    initializePortAudioStream(&pas, sampleRate);
    
    while(running)
    {
        initializeBuddyBox(&bb, sampleRate);
        
        while(running && bb.active && writePortAudioStream(&pas) && readPortAudioStream(&pas))
        {
            readBufferIntoBuddyBoxInputChannelBuffer(&bb, pas.bufferedSamples, pas.bufferSize);
            
            writeBuddyBoxOutputChannelBufferIntoBuffer(&bb, pas.bufferedSamples, pas.bufferSize);
            
            generateOutput(&bb);
            
            displayInput(&bb);
        }
        
        sleep(1);
    }
    
    closePortAudioStream(&pas);
    
    printf("Program Halted...\n");
}

