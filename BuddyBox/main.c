//
//  main.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include <stdio.h>
#include "PortAudioStream.h"
#include "BuddyBox.h"

int main(int argc, const char * argv[])
{
    PortAudioStream pas;
    BuddyBox bb;
    
    initializePortAudioStream(&pas);
    initializeBuddyBox(&bb);
    
    while(readPortAudioStream(&pas))
        readBufferIntoBuddyBox(&bb, pas.bufferedSamples, pas.bufferSize);
    
    closePortAudioStream(&pas);
}

