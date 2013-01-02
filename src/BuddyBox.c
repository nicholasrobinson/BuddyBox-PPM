//
//  BuddyBox.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include "BuddyBox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void initializeBuddyBox(BuddyBox *bb, unsigned int sampleRate)
{
    int i;
    printf("BuddyBox:\tInitializing.\n");
    
    bb->active                      = 1;
    bb->sampleRate                  = sampleRate;
    bb->input                       = SIGNAL_LOW;
    bb->lastInput                   = SIGNAL_LOW;
    bb->inputChannel                = 0;
    bb->inputChannelCount           = 0;
    bb->outputChannelCount          = 0;
    bb->inputSampleCount            = 0;
    bb->outputSampleCount           = 0;
    bb->elapsedInputSampleCounts    = 0;
    bb->maxElapsedInputSampleCount  = 0;
    bb->lastInputEdgeSampleCount    = 0;
    bb->inputSynchroFrameCount      = 0;
    bb->badInputFrameCount          = 0;
    bb->outputOverflowSampleCount   = 0;
    for (i = 0; i < MAX_CHANNELS; i++)
        bb->inputChannelBuffer[i]   = 0;
    for (i = 0; i < MAX_CHANNELS; i++)
        bb->outputChannelBuffer[i]  = 0;
    bb->minInputSample              = 0.0f;
    bb->maxInputSample              = 0.0f;
    for (i = 0; i < MAX_CHANNELS; i++)
        bb->inputChannelValues[i]  = 0.0f;
    allocateOutputOverflowBuffer(bb);
}

    void allocateOutputOverflowBuffer(BuddyBox *bb)
    {
        bb->outputOverflowBufferSize = OVERFLOW_SAMPLES;
        bb->outputOverflowBuffer = (float *) malloc(bb->outputOverflowBufferSize * sizeof(float));
        if(bb->outputOverflowBuffer == NULL)
        {
            printf("BuddyBox:\tCould Not Allocate Output Overflow Buffer.\n");
            exit(1);
        }
        memset(bb->outputOverflowBuffer, 0, bb->outputOverflowBufferSize * sizeof(float));
    }

void readBufferIntoBuddyBoxInputChannelBuffer(BuddyBox *bb, float* buffer, unsigned int bufferSize)
{
    float localMinSample, localMaxSample;
    unsigned int i, localMaxElapsedCount;
    
    detectBuddyBoxInputTimeout(bb, buffer, bufferSize);
    
    localMinSample = 0.0f;
    localMaxSample = 0.0f;
    localMaxElapsedCount = 0;
    for (i = 0; bb->active && i < bufferSize; i++, bb->inputSampleCount++)
    {
        localMinSample = getBuddyBoxLocalMinSample(buffer[i], localMinSample);
        localMaxSample = getBuddyBoxLocalMaxSample(buffer[i], localMaxSample);
        localMaxElapsedCount = getBuddyBoxLocalMaxElapsedInputSampleCount(bb, localMaxElapsedCount, bufferSize);
        processBuddyBoxRawInput(bb, buffer[i]);
        if (isBuddyBoxInputEdge(bb, buffer[i]))
            processBuddyBoxInputEdge(bb);
    }
    if (isBuddyBoxInputCalibrating(bb))
        calibrateBuddyBoxInput(bb, localMinSample, localMaxSample, localMaxElapsedCount);
}

    void detectBuddyBoxInputTimeout(BuddyBox *bb, float* buffer, unsigned int bufferSize)
    {
        unsigned int i;
        for (i = 0; i < bufferSize; i++)
            if (isBuddyBoxRawInputAboveNoiseThreshold(buffer[i]))
                return;
        if (!isBuddyBoxInputCalibrating(bb))
            handleBuddyBoxInputTimeout(bb);
    }

        unsigned int isBuddyBoxRawInputAboveNoiseThreshold(float bufferSample)
        {
            return (getBuddyBoxSampleMagnitude(bufferSample) > SIGNAL_NOISE_THRESHOLD);
        }

        void handleBuddyBoxInputTimeout(BuddyBox *bb)
        {
            printf("BuddyBox:\tInput Timeout.\n");
            disconnectBuddyBox(bb);
        }

    float getBuddyBoxSampleMagnitude(float sample)
    {
        return fabs(sample);
    }

    float getBuddyBoxLocalMinSample(float bufferSample, float localMinSample)
    {
        return (bufferSample < localMinSample) ? bufferSample : localMinSample;
    }

    float getBuddyBoxLocalMaxSample(float bufferSample, float localMaxSample)
    {
        return (bufferSample > localMaxSample) ? bufferSample : localMaxSample;
    }

    float getBuddyBoxLocalMaxElapsedInputSampleCount(BuddyBox *bb, unsigned int localMaxElapsedInputSampleCount, unsigned int bufferSize)
    {
        return (bb->elapsedInputSampleCounts > localMaxElapsedInputSampleCount && bb->elapsedInputSampleCounts < bufferSize) ? bb->elapsedInputSampleCounts : localMaxElapsedInputSampleCount;
    }

    void processBuddyBoxRawInput(BuddyBox *bb, float bufferSample)
    {
        bb->lastInput = bb->input;
        bb->input = isBuddyBoxRawInputHigh(bb, bufferSample) ? SIGNAL_HIGH : SIGNAL_LOW;
    }

    unsigned int isBuddyBoxInputEdge(BuddyBox *bb, float bufferSample)
    {
        return (bb->input != bb->lastInput);
    }

        unsigned int isBuddyBoxRawInputHigh(BuddyBox *bb, float bufferSample)
        {
            return (isBuddyBoxRawInputAboveNoiseThreshold(bufferSample) && bufferSample > (bb->maxInputSample + bb->minInputSample) / 2);
        }

    void processBuddyBoxInputEdge(BuddyBox *bb)
    {
        updateBuddyBoxElapsedInputSampleCounts(bb);
        if (isBuddyBoxInputHigh(bb))
            processHighBuddyBoxInput(bb);
        bb->lastInputEdgeSampleCount = bb->inputSampleCount;
    }

        void updateBuddyBoxElapsedInputSampleCounts(BuddyBox *bb)
        {
            if (bb->lastInputEdgeSampleCount > bb->inputSampleCount)
                bb->elapsedInputSampleCounts = bb->inputSampleCount + (MAXUINT - bb->lastInputEdgeSampleCount);
            else
                bb->elapsedInputSampleCounts = bb->inputSampleCount - bb->lastInputEdgeSampleCount;
        }

            unsigned int isBuddyBoxInputHigh(BuddyBox *bb)
            {
                return (bb->input == SIGNAL_HIGH);
            }

            void processHighBuddyBoxInput(BuddyBox *bb)
            {
                if (isBuddyBoxInputSynchroFrame(bb))
                    processBuddyBoxInputSynchroFrame(bb);
                else if (isBuddyBoxInputChannelValid(bb))
                    targetNextBuddyBoxInputChannel(bb);
                else if (!isBuddyBoxInputCalibrating(bb))
                    handleInvalidBuddyBoxInputChannel(bb);
            }

                unsigned int isBuddyBoxInputSynchroFrame(BuddyBox *bb)
                {
                    return (bb->inputChannel >= MIN_CHANNELS && (bb->elapsedInputSampleCounts > bb->maxElapsedInputSampleCount / 2));
                }

                void processBuddyBoxInputSynchroFrame(BuddyBox *bb)
                {
                    bb->inputSynchroFrameCount++;
                    if (!isBuddyBoxInputCalibrating(bb))
                    {
                        if (isBuddyBoxInputChannelCountValid(bb))
                        {
                            processBuddyBoxInputFrame(bb);
                            storeBuddyBoxInputChannelCount(bb);
                        }
                        else
                            handleInvalidBuddyBoxInputChannelCount(bb);
                    }
                    else
                        storeBuddyBoxInputChannelCount(bb);
                    targetNextBuddyBoxInputFrame(bb);
                }

                    unsigned int isBuddyBoxInputChannelCountValid(BuddyBox *bb)
                    {
                        return (bb->inputChannel == bb->inputChannelCount);
                    }

                    void storeBuddyBoxInputChannelCount(BuddyBox *bb)
                    {
                        bb->inputChannelCount = bb->inputChannel;
                    }

                    void handleInvalidBuddyBoxInputChannelCount(BuddyBox *bb)
                    {
                        bb->badInputFrameCount++;
                        if (!isBuddyBoxInputViable(bb))
                        {
                            printf("BuddyBox:\tInput Channel Count Changed.%d\n", bb->inputSynchroFrameCount);
                            disconnectBuddyBox(bb);
                        }   
                    }

                    void targetNextBuddyBoxInputFrame(BuddyBox *bb)
                    {
                        bb->inputChannel = 0;
                    }

                unsigned int isBuddyBoxInputChannelValid(BuddyBox *bb)
                {
                    return (bb->inputChannel < MAX_CHANNELS);
                }

                void processBuddyBoxInputFrame(BuddyBox *bb)
                {
                    unsigned int i, chanelDuration;

                    for (i = 0; i < bb->inputChannelCount; i++)
                    {
                        chanelDuration = bb->inputChannelBuffer[i] * MICROSECONDS_PER_SECOND / bb->sampleRate;
                        if (chanelDuration < CHANNEL_MIN_DURATION)
                            bb->inputChannelValues[i] = 0.0f;
                        else
                            bb->inputChannelValues[i] = (float) (chanelDuration - CHANNEL_MIN_DURATION) / (CHANNEL_MAX_DURATION - CHANNEL_MIN_DURATION);
                    }
                    bb->badInputFrameCount = 0;
                }

                void targetNextBuddyBoxInputChannel(BuddyBox *bb)
                {
                    bb->inputChannelBuffer[bb->inputChannel] = bb->elapsedInputSampleCounts;
                    bb->inputChannel++;
                }

                void handleInvalidBuddyBoxInputChannel(BuddyBox *bb)
                {
                    bb->badInputFrameCount++;
                    if (!isBuddyBoxInputViable(bb))
                    {
                        printf("BuddyBox:\tInvalid Input Channel Received.\n");
                        disconnectBuddyBox(bb);
                    }
                }

                unsigned int isBuddyBoxInputViable(BuddyBox *bb)
                {
                    return (bb->badInputFrameCount < BAD_FRAME_THRESHOLD);
                }

    unsigned int isBuddyBoxInputCalibrating(BuddyBox *bb)
    {
        return (bb->inputSynchroFrameCount < CALIBRATION_FRAMES);
    }

    void calibrateBuddyBoxInput(BuddyBox *bb, float localMinSample, float localMaxSample, unsigned int localMaxElapsedCount)
    {
        bb->minInputSample = localMinSample;
        bb->maxInputSample = localMaxSample;
        bb->maxElapsedInputSampleCount = localMaxElapsedCount;
    }

void setBuddyBoxOutputChannelValue(BuddyBox *bb, unsigned int channel, float channelValue)
{
    unsigned int channelDuration;
    
    channelDuration = channelValue / 1.0f * (CHANNEL_MAX_DURATION - CHANNEL_MIN_DURATION) + CHANNEL_MIN_DURATION;
    setBuddyBoxOutputChannelDuration(bb, channel, channelDuration);
}

    void setBuddyBoxOutputChannelDuration(BuddyBox *bb, unsigned int channel, unsigned int channelDuration)
    {
        if (channel < MAX_CHANNELS)
        {
            if (channelDuration > CHANNEL_MAX_DURATION)
                channelDuration = CHANNEL_MAX_DURATION;
            else if (channelDuration < CHANNEL_MIN_DURATION)
                channelDuration = CHANNEL_MIN_DURATION;
            bb->outputChannelBuffer[channel] = channelDuration * bb->sampleRate / MICROSECONDS_PER_SECOND;
        }
    }

void writeBuddyBoxOutputChannelBufferIntoBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSize)
{
    unsigned int bufferSampleCount;

    bufferSampleCount = writeBuddyBoxOverflowBufferIntoBuffer(bb, buffer);
    while (bufferSampleCount < bufferSize)
        bufferSampleCount += writeBuddyBoxOutputChannelBufferIntoBufferFrame(bb, buffer, bufferSize, bufferSampleCount);
}

    unsigned int writeBuddyBoxOverflowBufferIntoBuffer(BuddyBox *bb, float* buffer)
    {
        unsigned int i;

        for (i = 0; i < bb->outputOverflowSampleCount; i++)
            buffer[i] = bb->outputOverflowBuffer[i];
        bb->outputOverflowSampleCount = 0;
        
        return i;
    }

    unsigned int writeBuddyBoxOutputChannelBufferIntoBufferFrame(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount)
    {
        unsigned int frameSampleCount;

        frameSampleCount = writeBuddyBoxOutputChannelBufferIntoBufferChannels(bb, buffer, bufferSize, bufferSampleCount);
        frameSampleCount += writeBuddyBoxOutputChannelBufferIntoBufferSynchro(bb, buffer, bufferSize, bufferSampleCount, frameSampleCount);
        
        return frameSampleCount;
    }

        unsigned int writeBuddyBoxOutputChannelBufferIntoBufferChannels(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount)
        {
            unsigned int channel, channelsSampleCount;

            channelsSampleCount = 0;
            for (channel = 0; channel < bb->outputChannelCount; channel++)
                channelsSampleCount += writeBuddyBoxOutputChannelBufferIntoBufferChannel(bb, buffer, bufferSize, bufferSampleCount, channelsSampleCount, channel);
            
            return channelsSampleCount;
        }

            unsigned int writeBuddyBoxOutputChannelBufferIntoBufferChannel(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount, unsigned int channelsSampleCount, unsigned int channel)
            {
                unsigned int channelSampleCount;
                
                channelSampleCount = writeBuddyBoxChannelSeperatorIntoBufferChannel(bb, buffer, bufferSize, bufferSampleCount + channelsSampleCount);                
                channelSampleCount += writeBuddyBoxChannelDurationIntoBufferChannel(bb, buffer, bufferSize, bufferSampleCount + channelsSampleCount + channelSampleCount, bb->outputChannelBuffer[channel]);
                
                return channelSampleCount;
            }

                unsigned int writeBuddyBoxChannelSeperatorIntoBufferChannel(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int startBufferSample)
                {
                    return writeBuddyBoxSignalsToBufferOrOverflowBuffer(bb, buffer, bufferSize, startBufferSample, 0, SEPARATOR_DURATION * bb->sampleRate / MICROSECONDS_PER_SECOND, SIGNAL_HIGH_FLOAT);
                }

                    unsigned int writeBuddyBoxSignalsToBufferOrOverflowBuffer(BuddyBox *bb, float *buffer, unsigned int bufferSize, unsigned int startBufferSample, unsigned int comparatorOffset, unsigned int endBufferSampleOffset, float signal)
                    {
                        unsigned int i;

                        for (i = 0; i + comparatorOffset < endBufferSampleOffset; i++)
                            writeBuddyBoxSignalToBufferOrOverflowBuffer(bb, buffer, bufferSize, startBufferSample + i, signal);
                        
                        return i;
                    }

                        void writeBuddyBoxSignalToBufferOrOverflowBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSample, float signal)
                        {
                            if (bufferSample < bufferSize)
                                writeBuddyBoxSignalToBuffer(bb, buffer, bufferSample, signal);
                            else
                                writeBuddyBoxSignalToOverflowBuffer(bb, bufferSample - bufferSize, signal);
                        }

                            void writeBuddyBoxSignalToBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSample, float signal)
                            {
                                buffer[bufferSample] = signal;
                                bb->outputSampleCount++;
                            }

                            void writeBuddyBoxSignalToOverflowBuffer(BuddyBox *bb, unsigned int bufferSample, float signal)
                            {
                                bb->outputOverflowBuffer[bufferSample] = signal;
                                bb->outputOverflowSampleCount++;
                            }

                unsigned int writeBuddyBoxChannelDurationIntoBufferChannel(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int startBufferSample, unsigned int endBufferSampleOffset)
                {
                    return writeBuddyBoxSignalsToBufferOrOverflowBuffer(bb, buffer, bufferSize, startBufferSample, 0, endBufferSampleOffset, SIGNAL_LOW_FLOAT);
                }

            unsigned int writeBuddyBoxOutputChannelBufferIntoBufferSynchro(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount, unsigned int frameSampleCount)
            {
                unsigned int synchroSampleCount;
                
                synchroSampleCount = writeBuddyBoxChannelSeperatorIntoBufferChannel(bb, buffer, bufferSize, bufferSampleCount + frameSampleCount);
                synchroSampleCount += writeBuddyBoxSynchroIntoBufferSynchro(bb, buffer, bufferSize, bufferSampleCount + frameSampleCount + synchroSampleCount, frameSampleCount);
                
                return synchroSampleCount;
            }

                unsigned int writeBuddyBoxSynchroIntoBufferSynchro(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int startBufferSample, unsigned int comparatorOffset)
                {
                    return writeBuddyBoxSignalsToBufferOrOverflowBuffer(bb, buffer, bufferSize, startBufferSample, comparatorOffset, bb->sampleRate * FRAME_DURATION / MICROSECONDS_PER_SECOND, SIGNAL_LOW_FLOAT);
                }

void disconnectBuddyBox(BuddyBox *bb)
{
    bb->active = 0;
    free(bb->outputOverflowBuffer);
    printf("BuddyBox:\tDisconnected.\n");
}
