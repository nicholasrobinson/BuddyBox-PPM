//
//  PortAudioStream.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include "PortAudioStream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initializePortAudioStream(PortAudioStream *pas)
{
    PaStreamParameters inputParameters, outputParameters;
    
    allocatePortAudioStreamBuffer(pas);
    initializePortAudio(pas);
    configurePortAudioInputParameters(&inputParameters);
    configurePortAudioOutputParameters(&outputParameters);
    openPortAudioStream(outputParameters, inputParameters, pas);
}

void allocatePortAudioStreamBuffer(PortAudioStream *pas)
{   
    pas->bufferSize = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE / sizeof(float);
    pas->bufferedSamples = (float *) malloc(pas->bufferSize);
    if(pas->bufferedSamples == NULL)
    {
        printf("Could not allocate record array.\n");
        exit(1);
    }
    memset(pas->bufferedSamples, 0, FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE);
}

void initializePortAudio(PortAudioStream *pas)
{
    PaError err;
    
    err = Pa_Initialize();
    if(err != paNoError)
        handlePortAudioStreamError(pas, err);
}

void openPortAudioStream(PaStreamParameters outputParameters, PaStreamParameters inputParameters, PortAudioStream *pas)
{
    PaError err;
    
    err = Pa_OpenStream(
        &pas->stream,
        &inputParameters,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,          // we won't output out of range samples so don't bother clipping them
        NULL,               // no callback, use blocking API
        NULL                // no callback, so no callback userData
    );
    if(err != paNoError)
        handlePortAudioStreamError(pas, err);
    err = Pa_StartStream(pas->stream);
    if(err != paNoError)
        handlePortAudioStreamError(pas, err);
}

void configurePortAudioInputParameters(PaStreamParameters *inputParameters_p)
{
    inputParameters_p->device = PA_INPUT_DEVICE;
    inputParameters_p->channelCount = NUM_CHANNELS;
    inputParameters_p->sampleFormat = PA_SAMPLE_TYPE;
    inputParameters_p->suggestedLatency = Pa_GetDeviceInfo(inputParameters_p->device)->defaultHighInputLatency ;
    inputParameters_p->hostApiSpecificStreamInfo = NULL;
}

void configurePortAudioOutputParameters(PaStreamParameters *outputParameters_p)
{
    outputParameters_p->device = Pa_GetDefaultOutputDevice(); // default output device
    outputParameters_p->channelCount = NUM_CHANNELS;
    outputParameters_p->sampleFormat = PA_SAMPLE_TYPE;
    outputParameters_p->suggestedLatency = Pa_GetDeviceInfo(outputParameters_p->device)->defaultHighOutputLatency;
    outputParameters_p->hostApiSpecificStreamInfo = NULL;
}

unsigned int readPortAudioStream(PortAudioStream *pas)
{
    PaError err;
    
    err = Pa_ReadStream(pas->stream, pas->bufferedSamples, FRAMES_PER_BUFFER);
    if(err && CHECK_OVERFLOW)
        return handlePortAudioStreamOverflow(pas, err);
    else
        return 1;
}

void closePortAudioStream(PortAudioStream *pas)
{
    PaError err;
    
    err = Pa_StopStream(pas->stream);
    if(err != paNoError)
        handlePortAudioStreamError(pas, err);
    free(pas->bufferedSamples);
    Pa_Terminate();
}

unsigned int handlePortAudioStreamOverflow(PortAudioStream *pas, PaError err)
{
    terminatePortAudioStream(pas);
    if(err & paInputOverflow)
        fprintf(stderr, "Input Overflow.\n");
    if(err & paOutputUnderflow)
        fprintf(stderr, "Output Underflow.\n");
    return 0;
}

void handlePortAudioStreamError(PortAudioStream *pas, PaError err)
{
    terminatePortAudioStream(pas);
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    exit(-1);
}

void terminatePortAudioStream(PortAudioStream *pas)
{
    if(pas->stream)
    {
        Pa_AbortStream(pas->stream);
        Pa_CloseStream(pas->stream);
    }
    free(pas->bufferedSamples);
    Pa_Terminate();
}