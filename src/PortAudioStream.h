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
#define PA_INPUT_DEVICE     1 // Line In - could also be: Pa_GetDefaultInputDevice()

#if 1
static const unsigned int SAMPLE_RATE           = 192000; // Line In @ 192kHz -> resolution of ~154 per channel
#elif 0
static const unsigned int SAMPLE_RATE           = 44100;  // Line In @ 44.1kHz -> resolution of ~36 per channel
#endif
static const unsigned int FRAMES_PER_BUFFER     = 4096;
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

void initializePortAudioStream(PortAudioStream *pas);
    void allocatePortAudioStreamBuffer(PortAudioStream *pas);
    void initializePortAudio(PortAudioStream *pas);
        void handlePortAudioStreamError(PortAudioStream *pas, PaError err);
            void terminatePortAudioStream(PortAudioStream *pas);
        void configurePortAudioInputParameters(PaStreamParameters *inputParameters);
    void configurePortAudioOutputParameters(PaStreamParameters *outputParameters);
    void openPortAudioStream(PaStreamParameters outputParameters, PaStreamParameters inputParameters, PortAudioStream *pas);

unsigned int readPortAudioStream(PortAudioStream *pas);
    unsigned int handlePortAudioStreamOverflow(PortAudioStream *pas, PaError err);

void closePortAudioStream(PortAudioStream *pas);

#endif
