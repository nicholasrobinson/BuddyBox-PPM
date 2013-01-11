//
//  main.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include "BuddyBoxThread.h"
#include "PortAudioStream.h"
#include "BuddyBox.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static const unsigned int DEFAULT_SAMPLE_RATE = 124000;

static unsigned int running = 1;
static int killsig = 0;

void intHandler(int sig) {
    running = 0;
    killsig = sig;
}

void generateOutput(BuddyBox *bb)
{
    unsigned int i;
    
    bb->outputChannelCount = bb->inputChannelCount;
    for (i = 0; i < bb->outputChannelCount; i++)
        setBuddyBoxOutputChannelValue(bb, i, bb->inputChannelValues[i]);
}

void displayInput(BuddyBox *bb)
{
    unsigned int i;
    
    if (bb->active && !isBuddyBoxInputCalibrating(bb))
    {
        for (i = 0; i < bb->inputChannelCount; i++)
            printf("%f\t,", bb->inputChannelValues[i]);
        printf("%u\n", bb->inputSynchroFrameCount);
    }
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
        
        while(running && pasBB.bb.active)
        {
            generateOutput(&pasBB.bb);
            
            displayInput(&pasBB.bb);
            
            usleep(100000);
        }
        
        stopBuddyBoxThread();
        
        joinBuddyBoxThread(&pasBB);
        
        sleep(1);
    }
    
    cleanupBuddyBoxThread(&pasBB);
    
    printf("Program Halted...\n");
}

