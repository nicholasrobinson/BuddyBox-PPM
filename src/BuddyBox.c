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
                        return (getBuddyBoxInputChannel(bb) == bb->inputChannelCount);
                    }

                        unsigned int getBuddyBoxInputChannel(BuddyBox *bb)
                        {
                            return bb->inputChannel - 1;
                        }

                    void storeBuddyBoxInputChannelCount(BuddyBox *bb)
                    {
                        bb->inputChannelCount = getBuddyBoxInputChannel(bb);
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
                    unsigned int i;

                    for (i = 0; i < MAX_CHANNELS; i++)
                        if (i < bb->inputChannel)
                            printf("%u\t,", bb->inputChannelBuffer[i]);
                    printf("%u\n", bb->inputSynchroFrameCount);
                    
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

void setBuddyBoxOutputChannelDuration(BuddyBox *bb, unsigned int channel, unsigned int channelDuration)
{
    if (channel < MAX_CHANNELS)
    {
        if (channelDuration > CHANNEL_MAX_DURATION)
            channelDuration = CHANNEL_MAX_DURATION;
        else if (channelDuration < CHANNEL_MIN_DURATION)
            channelDuration = CHANNEL_MIN_DURATION;
        bb->inputChannelBuffer[channel] = channelDuration * bb->sampleRate / MICROSECONDS_PER_SECOND;
    }
}

void writeBuddyBoxOutputChannelBufferIntoBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSize)
{
    unsigned int i, j, endJ, channel;

    for (i = 0; i < bb->outputOverflowSampleCount; i++)
    {
        buffer[i] = bb->outputOverflowBuffer[i];
    }
    
    bb->outputOverflowSampleCount = 0;
    
    while (i < bufferSize)
    {
        channel = 0;
        j = 0;
//printf("LOOPING FOR %u, i=%d\n", bb->sampleRate * FRAME_DURATION / MICROSECONDS_PER_SECOND, i);
        while (j < bb->sampleRate * FRAME_DURATION / MICROSECONDS_PER_SECOND)
        {
            if (channel < 9)//bb->inputChannelCount)
            {
                endJ = j + SEPARATOR_DURATION * bb->sampleRate / MICROSECONDS_PER_SECOND;
                while (j < endJ)
                {
                    if (i + j < bufferSize)
                    {
//printf("SEPARATOR %u: %u / %u - %f\n", channel, i + j, endJ - j, SIGNAL_HIGH_FLOAT);
                        buffer[i + j] = SIGNAL_HIGH_FLOAT;
                        bb->outputSampleCount++;
                    }
                    else
                    {
//printf("[BUFF] SEPARATOR %u: %u / %u - %f\n", channel, i + j, endJ - j, SIGNAL_HIGH_FLOAT);
                        bb->outputOverflowBuffer[i + j - bufferSize] = SIGNAL_HIGH_FLOAT;
                        bb->outputOverflowSampleCount++;
                    }
                    j++;
                }
                endJ = j + bb->inputChannelBuffer[channel];
                while (j < endJ)
                {
                    if (i + j < bufferSize)
                    {
//printf("SIGNAL %u: %u / %u - %f\n", channel, i + j, endJ - j, SIGNAL_LOW_FLOAT);
                        buffer[i + j] = SIGNAL_LOW_FLOAT;
                        bb->outputSampleCount++;
                    }
                    else
                    {
//printf("[BUFF] SIGNAL %u: %u / %u - %f\n", channel, i + j, endJ - j, SIGNAL_LOW_FLOAT);
                        bb->outputOverflowBuffer[i + j - bufferSize] = SIGNAL_LOW_FLOAT;
                        bb->outputOverflowSampleCount++;
                    }
                    j++;
                }
            }
            else
            {
                if (i + j < bufferSize)
                {
//printf("SYNCHRO: %u / %u - %f\n", i + j, bb->sampleRate * FRAME_DURATION / MICROSECONDS_PER_SECOND, SIGNAL_LOW_FLOAT);
                    buffer[i + j] = SIGNAL_LOW_FLOAT;
                    bb->outputSampleCount++;
                }
                else
                {
//printf("[BUFF] SYNCHRO: %u / %u - %f\n", i + j, bb->sampleRate * FRAME_DURATION / MICROSECONDS_PER_SECOND, SIGNAL_LOW_FLOAT);
                    bb->outputOverflowBuffer[i + j - bufferSize] = SIGNAL_LOW_FLOAT;
                    bb->outputOverflowSampleCount++;
                }
                j++;
            }
            channel++;
        }
        i += bb->sampleRate * FRAME_DURATION / MICROSECONDS_PER_SECOND;
    }
    
//    for (i = 0; i < bufferSize; i++)
//        printf("%f\n",buffer[i]);
}

void disconnectBuddyBox(BuddyBox *bb)
{
    bb->active = 0;
    printf("BuddyBox:\tDisconnected.\n");
}
