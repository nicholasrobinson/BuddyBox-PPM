//
//  BuddyBoxThread.h
//  BuddyBox-PPM
//
//  Created by Nicholas Robinson on 1/10/13.
//  Copyright (c) 2013 Nicholas Robinson. All rights reserved.
//

#ifndef BuddyBox_PPM_BuddyBoxThread_h
#define BuddyBox_PPM_BuddyBoxThread_h

#include "PortAudioStream.h"
#include "BuddyBox.h"

#include <pthread.h>

typedef struct
{
    PortAudioStream pas;
    BuddyBox bb;
    pthread_t buddyBoxThread;
    unsigned int sampleRate;
} PASBuddyBox;

void initializeBuddyBoxThread(PASBuddyBox *pasBB);

void startBuddyBoxThread(PASBuddyBox *pasBB);

void stopBuddyBoxThread();

void joinBuddyBoxThread(PASBuddyBox *pasBB);

void* runBuddyBoxThread(void *arguments);

#endif
