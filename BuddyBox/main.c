//
//  main.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include <stdio.h>

#define SIGNAL_HIGH (1)
#define SIGNAL_LOW (0)
#define SYNCHRO_FRAME_WIDTH (200)
#define MAX_CHANNELS (10)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "portaudio.h"

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024)
#define NUM_CHANNELS    (1)

/* @todo Underflow and overflow is disabled until we fix priming of blocking write. */
#define CHECK_OVERFLOW  (0)
#define CHECK_UNDERFLOW  (0)

/* Select sample format. */
#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)
#define CLEAR(a) memset( (a), 0, FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE )
#define PRINTF_S_FORMAT "%.8f"

int main(int argc, const char * argv[])
{
    /* -- init -- */
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream = NULL;
    PaError err;
    unsigned int i, j, numFloats, sampleCount, bufferCount, powerWindow, signal, signalEdge, signalEdgeSampleCount, elapsedCounts, signalChannel;
    unsigned int signals[MAX_CHANNELS];
    float bufferSampleMagnitude, accumulator, power, globalMaxSample, localMaxSample, tmpLocalMaxSample;
    numFloats = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE / sizeof(float);
    float *bufferSamples;
    
    /* -- setup -- */
    
    fflush(stdout);
    
    bufferSamples = (float *) malloc(numFloats);
    if( bufferSamples == NULL )
    {
        printf("Could not allocate record array.\n");
        exit(1);
    }
    CLEAR(bufferSamples);
    
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
    
    inputParameters.device = 1; /* line in */ // Pa_GetDefaultInputDevice(); /* default input device */
    printf( "Input device # %d.\n", inputParameters.device );
    printf( "Input LL: %g s\n", Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency );
    printf( "Input HL: %g s\n", Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency );
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    printf( "Output device # %d.\n", outputParameters.device );
    printf( "Output LL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency );
    printf( "Output HL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency );
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    err = Pa_OpenStream(
                        &stream,
                        &inputParameters,
                        &outputParameters,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,          /* we won't output out of range samples so don't bother clipping them */
                        NULL,               /* no callback, use blocking API */
                        NULL                /* no callback, so no callback userData */
                        );
    if( err != paNoError ) goto error;
    
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    
    /* -- main loop -- */
    
    // Initialize counters
    sampleCount             = 0;
    elapsedCounts           = 0;
    bufferCount             = 0;
    power                   = 0.0f;
    localMaxSample          = 0.0f;
    globalMaxSample         = 0.0f;
    signal                  = SIGNAL_LOW;
    signalEdge              = 0;
    signalEdgeSampleCount   = 0;
    signalChannel           = 0;
    
    // Loop indefiniately
    while(1)
    {
        // Read from input
        err = Pa_ReadStream( stream, bufferSamples, FRAMES_PER_BUFFER );
        if( err && CHECK_OVERFLOW ) goto xrun;
        
        // Loop through samples
        tmpLocalMaxSample = 0.0f;
        for (i = 0; i < numFloats; i++)
        {
            // Measure sample magnitude
            bufferSampleMagnitude = fabs(bufferSamples[i]);
            
            // Determine local max
            if (bufferSampleMagnitude > tmpLocalMaxSample)
            {
                tmpLocalMaxSample = bufferSampleMagnitude;
            }
            
            // Detect signal level changes
            if (bufferSampleMagnitude > localMaxSample / 2)
            {
                if (signal == SIGNAL_LOW)
                {
                    signalEdge = 1;
                }
                signal = SIGNAL_HIGH;
            }
            else
            {
                if (signal == SIGNAL_HIGH)
                {
                    signalEdge = 1;
                }
                signal = SIGNAL_LOW;
            }
            if (signalEdge == 1)
            {
                signalEdge = 0;
                elapsedCounts = sampleCount - signalEdgeSampleCount;
                if (signal == SIGNAL_HIGH)
                {
                    if (elapsedCounts > SYNCHRO_FRAME_WIDTH)
                    {
                        for (j = 0; j < MAX_CHANNELS; j++)
                        {
                            if (j < signalChannel)
                            {
                                printf("%d\t,", signals[j]);
                            }
                        }
                        printf("%d\n", sampleCount);
                        signalChannel = 0;
                    }
                    else if (signalChannel < MAX_CHANNELS)
                    {
                        signals[signalChannel] = elapsedCounts;
                        signalChannel++;
                    }
                }
                
                signalEdgeSampleCount = sampleCount;
            }
            
            // Increment sample counter
            sampleCount++;
        }
        localMaxSample = tmpLocalMaxSample;
        
        bufferCount = bufferCount++;
    }
    
    /* -- tear down -- */
    
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
    
    free( bufferSamples );
    
    Pa_Terminate();
    return 0;
    
xrun:
    if( stream ) {
        Pa_AbortStream( stream );
        Pa_CloseStream( stream );
    }
    free( bufferSamples );
    Pa_Terminate();
    if( err & paInputOverflow )
        fprintf( stderr, "Input Overflow.\n" );
    if( err & paOutputUnderflow )
        fprintf( stderr, "Output Underflow.\n" );
    return -2;
    
error:
    if( stream ) {
        Pa_AbortStream( stream );
        Pa_CloseStream( stream );
    }
    free( bufferSamples );
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return -1;
}

