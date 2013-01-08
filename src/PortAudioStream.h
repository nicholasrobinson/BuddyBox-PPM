//
//  PortAudio.h
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//
//  Sample Rate Notes:
//  (glitchy) Line In @ 192kHz -> resolution of ~154 per channel
//  (glitchy) Line In @ 176.4kHz -> resolution of ~143 per channel
//  (glitchy) Line In @ 176.4kHz -> resolution of ~128 per channel
//  Line In @ 132.3kHz -> resolution of ~107 per channel
//  Line In @ 124.0kHz -> resolution of ~100 per channel
//  Line In @ 88.2kHz -> resolution of ~72 per channel
//  Line In @ 44.1kHz -> resolution of ~36 per channel

#ifndef BuddyBox_PortAudio_h
#define BuddyBox_PortAudio_h

#include "portaudio.h"

#define CHECK_OVERFLOW  (0)                                 // Not checking for overflow
#define CHECK_UNDERFLOW (0)                                 // Not checking for underflow

#define PA_SAMPLE_TYPE      paFloat32

static const unsigned int FRAMES_PER_BUFFER     = 4096;
static const unsigned int NUM_CHANNELS          = 1;

typedef struct
{
    // Public
    unsigned int sampleRate;
    float* bufferedSamples;
    unsigned int bufferSize;
    
    // Private
    PaStream* stream;
} PortAudioStream;

void initializePortAudioStream(PortAudioStream *pas, unsigned int sampleRate);
    void allocatePortAudioStreamBuffer(PortAudioStream *pas);
    void initializePortAudio(PortAudioStream *pas);
        void handlePortAudioStreamInitializationError(PortAudioStream *pas, PaError err);
            void terminatePortAudioStream(PortAudioStream *pas);
        void configurePortAudioInputParameters(PaStreamParameters *inputParameters);
    void configurePortAudioOutputParameters(PaStreamParameters *outputParameters);
    void openPortAudioStream(PortAudioStream *pas, PaStreamParameters outputParameters, PaStreamParameters inputParameters);

unsigned int readPortAudioStream(PortAudioStream *pas);
    unsigned int handlePortAudioStreamFlowError(PortAudioStream *pas, PaError err);

unsigned int writePortAudioStream(PortAudioStream *pas);

void closePortAudioStream(PortAudioStream *pas);

#endif
