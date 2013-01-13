//
//  main.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include "BuddyBoxThread.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static const unsigned int DEFAULT_SAMPLE_RATE = 124000;

static unsigned int running = 1;

void intHandler(int sig) {
    running = 0;
}

void generateOutput(PASBuddyBox *pasBB)
{
    unsigned int i, outputChannelCount;
    
    outputChannelCount = getBuddyBoxThreadInputChannelCount(pasBB);
    setBuddyBoxThreadOutputChannelCount(pasBB, outputChannelCount);
    for (i = 0; i < outputChannelCount; i++)
        setBuddyBoxThreadOutputChannelValue(pasBB, i, getBuddyBoxThreadInputChannelValue(pasBB, i));
}

void displayInput(PASBuddyBox *pasBB)
{
    unsigned int i, inputChannelCount;
    
    inputChannelCount = getBuddyBoxThreadInputChannelCount(pasBB);
    for (i = 0; i < inputChannelCount; i++)
        printf("%f\t", getBuddyBoxThreadInputChannelValue(pasBB, i));
    printf("\n");
}

int main(int argc, const char * argv[])
{
    signal(SIGKILL, intHandler);
    signal(SIGINT, intHandler);
    
    PASBuddyBox pasBB;
    
    pasBB.sampleRate = (argc > 1) ? (unsigned int) strtol(argv[1], NULL, 0) : DEFAULT_SAMPLE_RATE;
    
    initializeBuddyBoxThread(&pasBB);
    
    while(running)
    {   
        startBuddyBoxThread(&pasBB);
        
        while(running && isBuddyBoxThreadRunning(&pasBB))
        {
            generateOutput(&pasBB);
            
            if (isBuddyBoxThreadCalibrated(&pasBB))
                displayInput(&pasBB);
            
            usleep(100000);
        }
        
        stopBuddyBoxThread(&pasBB);
        
        joinBuddyBoxThread(&pasBB);
    }
    
    cleanupBuddyBoxThread(&pasBB);
    
    printf("Program Halted...\n");
}

