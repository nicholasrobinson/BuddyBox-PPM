//
//  main.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "PortAudioStream.h"
#include "BuddyBox.h"

static unsigned int running = 1;

void intHandler(int sig) {
    running = 0;
}

int main(int argc, const char * argv[])
{
    PortAudioStream pas;
    BuddyBox bb;
    
    signal(SIGKILL, intHandler);
    signal(SIGINT, intHandler);
    
    initializePortAudioStream(&pas);
    
    while(running)
    {
        initializeBuddyBox(&bb);
        
        while(running && bb.active && readPortAudioStream(&pas))
            readBufferIntoBuddyBox(&bb, pas.bufferedSamples, pas.bufferSize);
        
        sleep(1);
    }
    
    closePortAudioStream(&pas);
    
    printf("Program Halted...\n");
}

