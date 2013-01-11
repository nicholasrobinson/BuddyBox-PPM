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

void initializePortAudioStream(PortAudioStream *pas, unsigned int sampleRate)
{
    PaStreamParameters inputParameters, outputParameters;
    
    pas->sampleRate = sampleRate;
    
    allocatePortAudioStreamBuffer(pas);
    initializePortAudio(pas);
    configurePortAudioInputParameters(&inputParameters);
    configurePortAudioOutputParameters(&outputParameters);
    openPortAudioStream(pas, outputParameters, inputParameters);
}

    void allocatePortAudioStreamBuffer(PortAudioStream *pas)
    {
        pas->bufferSize = FRAMES_PER_BUFFER * NUM_CHANNELS;
        pas->bufferedSamples = (float *) malloc(pas->bufferSize * sizeof(float));
        if(pas->bufferedSamples == NULL)
        {
            printf("PortAudioStream:\tCould Not Allocate Sample Buffer.\n");
            exit(1);
        }
        memset(pas->bufferedSamples, 0, pas->bufferSize * sizeof(float));
    }

    void initializePortAudio(PortAudioStream *pas)
    {
        PaError err;
        
        err = Pa_Initialize();
        if(err != paNoError)
            handlePortAudioStreamInitializationError(pas, err);
    }

        void handlePortAudioStreamInitializationError(PortAudioStream *pas, PaError err)
        {
            terminatePortAudioStream(pas);
            fprintf(stderr, "PortAudioStream:\tAn error occured.\n");
            fprintf(stderr, "Error number:\t%d\n", err);
            fprintf(stderr, "Error message:\t%s\n", Pa_GetErrorText(err));
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

    void configurePortAudioInputParameters(PaStreamParameters *inputParameters)
    {
        inputParameters->device = Pa_GetDefaultInputDevice();
        inputParameters->channelCount = NUM_CHANNELS;
        inputParameters->sampleFormat = PA_SAMPLE_TYPE;
        inputParameters->suggestedLatency = Pa_GetDeviceInfo(inputParameters->device)->defaultHighInputLatency ;
        inputParameters->hostApiSpecificStreamInfo = NULL;
    }

    void configurePortAudioOutputParameters(PaStreamParameters *outputParameters)
    {
        outputParameters->device = Pa_GetDefaultOutputDevice();
        outputParameters->channelCount = NUM_CHANNELS;
        outputParameters->sampleFormat = PA_SAMPLE_TYPE;
        outputParameters->suggestedLatency = Pa_GetDeviceInfo(outputParameters->device)->defaultHighOutputLatency;
        outputParameters->hostApiSpecificStreamInfo = NULL;
    }

    void openPortAudioStream(PortAudioStream *pas, PaStreamParameters outputParameters, PaStreamParameters inputParameters)
    {
        PaError err;
        
        err = Pa_OpenStream(
            &pas->stream,
            &inputParameters,
            &outputParameters,
            pas->sampleRate,
            FRAMES_PER_BUFFER,
            paClipOff,          // we won't output out of range samples so don't bother clipping them
            NULL,               // no callback, use blocking API
            NULL                // no callback, so no callback userData
        );
        if(err != paNoError)
            handlePortAudioStreamInitializationError(pas, err);
        err = Pa_StartStream(pas->stream);
        if(err != paNoError)
            handlePortAudioStreamInitializationError(pas, err);
    }

unsigned int readPortAudioStream(PortAudioStream *pas)
{
    PaError err;
    
    err = Pa_ReadStream(pas->stream, pas->bufferedSamples, FRAMES_PER_BUFFER);
    if(err && CHECK_OVERFLOW)
        return handlePortAudioStreamFlowError(pas, err);
    else
        return 1;
}

    unsigned int handlePortAudioStreamFlowError(PortAudioStream *pas, PaError err)
    {
        terminatePortAudioStream(pas);
        if(err & paInputOverflow)
            fprintf(stderr, "PortAudioStream:\tInput Overflow.\n");
        if(err & paOutputUnderflow)
            fprintf(stderr, "PortAudioStream:\tOutput Underflow.\n");
        return 0;
    }

unsigned int writePortAudioStream(PortAudioStream *pas)
{
    PaError err;
    
    err = Pa_WriteStream(pas->stream, pas->bufferedSamples, FRAMES_PER_BUFFER);
    if(err && CHECK_UNDERFLOW)
        return handlePortAudioStreamFlowError(pas, err);
    else
        return 1;
}

void closePortAudioStream(PortAudioStream *pas)
{
    PaError err;
    
    err = Pa_StopStream(pas->stream);
    if(err != paNoError)
        handlePortAudioStreamInitializationError(pas, err);
    terminatePortAudioStream(pas);
}
