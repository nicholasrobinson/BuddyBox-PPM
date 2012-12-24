//
//  PortAudio.h
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#ifndef BuddyBox_PortAudio_h
#define BuddyBox_PortAudio_h

#include "portaudio.h"

#define CHECK_OVERFLOW  (0) // Not checking for overflow
#define CHECK_UNDERFLOW (0) // Not checking for underflow

#define PA_SAMPLE_TYPE      paFloat32
#define PA_INPUT_DEVICE     1 // Line In - could also be Pa_GetDefaultInputDevice()

static const unsigned int SAMPLE_RATE           = 44100;
static const unsigned int FRAMES_PER_BUFFER     = 1024;
static const unsigned int NUM_CHANNELS          = 1;
static const unsigned int SAMPLE_SIZE           = sizeof(float);

typedef struct
{
    // Public
    float* bufferedSamples;
    unsigned int bufferSize;
    
    // Private
    PaStream* stream;
} PortAudioStream;

// Public
void initializePortAudioStream(PortAudioStream *pas);
unsigned int readPortAudioStream(PortAudioStream *pas);
void closePortAudioStream(PortAudioStream *pas);

// Private
void allocatePortAudioStreamBuffer(PortAudioStream *pas);
void initializePortAudio(PortAudioStream *pas);
void openPortAudioStream(PaStreamParameters outputParameters, PaStreamParameters inputParameters, PortAudioStream *pas);
void configurePortAudioInputParameters(PaStreamParameters *inputParameters);
void configurePortAudioOutputParameters(PaStreamParameters *outputParameters);

unsigned int handlePortAudioStreamOverflow(PortAudioStream *pas, PaError err);
void handlePortAudioStreamError(PortAudioStream *pas, PaError err);
void terminatePortAudioStream(PortAudioStream *pas);

#endif
