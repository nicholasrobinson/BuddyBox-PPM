//
//  BuddyBox.c
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#include "BuddyBox.h"
#include <stdio.h>
#include <math.h>

void initializeBuddyBox(BuddyBox *bb)
{
    printf("Initializing Buddy Box...\n");
    
    bb->channelCount                = 0;
    bb->active                      = 1;
    
    bb->wireSignal                  = SIGNAL_LOW;
    bb->localMinSample              = 0.0f;
    bb->localMaxSample              = 0.0f;
    bb->localMaxElapsedCount        = 0;
    bb->sampleReadCount             = 0;
    bb->sampleWriteCount            = 0;
    bb->synchroFrameCount           = 0;
    bb->lastSignalEdgeSampleCount   = 0;
    bb->elapsedSampleCounts         = 0;
    bb->currentSignalChannel        = 0;
    bb->badPacketCount              = 0;
    bb->overflowSampleCount         = 0;
}

void readBufferIntoBuddyBox(BuddyBox *bb, float* buffer, unsigned int bufferSize)
{
    float tmpLocalMinSample, tmpLocalMaxSample;
    unsigned int i, tmpLocalMaxElapsedCount;
    
    detectBuddyBoxTimeout(bb, buffer, bufferSize);
    
    tmpLocalMinSample = 0.0f;
    tmpLocalMaxSample = 0.0f;
    tmpLocalMaxElapsedCount = 0;
    for (i = 0; bb->active && i < bufferSize; i++, bb->sampleReadCount++)
    {       
        tmpLocalMinSample = getBuddyBoxTmpLocalMinSample(buffer[i], tmpLocalMinSample);
        tmpLocalMaxSample = getBuddyBoxTmpLocalMaxSample(buffer[i], tmpLocalMaxSample);
        tmpLocalMaxElapsedCount = getBuddyBoxTmpLocalMaxElapsedCount(bb, tmpLocalMaxElapsedCount, bufferSize);
        if (isBuddyBoxSignalEdge(bb, buffer[i]))
            processBuddyBoxSignalEdge(bb);
    }
    if (isBuddyBoxCalibrating(bb))
        calibrateBuddyBox(bb, tmpLocalMinSample, tmpLocalMaxSample, tmpLocalMaxElapsedCount);
}

    void detectBuddyBoxTimeout(BuddyBox *bb, float* buffer, unsigned int bufferSize)
    {
        unsigned int i;
        for (i = 0; i < bufferSize; i++)
            if (isBuddyBoxSignalAboveNoiseThreshold(buffer[i]))
                return;
        if (!isBuddyBoxCalibrating(bb))
            handleBuddyBoxTimeout(bb);
    }

        unsigned int isBuddyBoxSignalAboveNoiseThreshold(float bufferSample)
        {
            return (getBuddyBoxSampleMagnitude(bufferSample) > SAMPLE_NOISE_THRESHOLD);
        }

        void handleBuddyBoxTimeout(BuddyBox *bb)
        {
            printf("Timeout...\n");
            disconnectBuddyBox(bb);
        }

    float getBuddyBoxSampleMagnitude(float sample)
    {
        return fabs(sample);
    }

    float getBuddyBoxTmpLocalMinSample(float bufferSample, float tmpLocalMinSample)
    {
        return (bufferSample < tmpLocalMinSample) ? bufferSample : tmpLocalMinSample;
    }

    float getBuddyBoxTmpLocalMaxSample(float bufferSample, float tmpLocalMaxSample)
    {
        return (bufferSample > tmpLocalMaxSample) ? bufferSample : tmpLocalMaxSample;
    }

    float getBuddyBoxTmpLocalMaxElapsedCount(BuddyBox *bb, unsigned int tmpLocalMaxElapsedCount, unsigned int bufferSize)
    {
        return (bb->elapsedSampleCounts > tmpLocalMaxElapsedCount && bb->elapsedSampleCounts < bufferSize) ? bb->elapsedSampleCounts : tmpLocalMaxElapsedCount;
    }

    unsigned int isBuddyBoxSignalEdge(BuddyBox *bb, float bufferSample)
    {
        unsigned int signalEdge;
        
        if (isBuddyBoxRawSignalHigh(bb, bufferSample))
        {
            signalEdge = (bb->wireSignal == SIGNAL_LOW) ? 1 : 0;
            bb->wireSignal = SIGNAL_HIGH;
        }
        else
        {
            signalEdge = (bb->wireSignal == SIGNAL_HIGH) ? 1 : 0;
            bb->wireSignal = SIGNAL_LOW;
        }
        return signalEdge;
    }

        unsigned int isBuddyBoxRawSignalHigh(BuddyBox *bb, float bufferSample)
        {
            return (isBuddyBoxSignalAboveNoiseThreshold(bufferSample) && bufferSample > (bb->localMaxSample + bb->localMinSample) / 2);
        }

    void processBuddyBoxSignalEdge(BuddyBox *bb)
    {
        updateBuddyBoxElapsedCounts(bb);
        if (isBuddyBoxWireSignalHigh(bb))
            processHighBuddyBoxWireSignal(bb);
        bb->lastSignalEdgeSampleCount = bb->sampleReadCount;
    }

        void updateBuddyBoxElapsedCounts(BuddyBox *bb)
        {
            if (bb->lastSignalEdgeSampleCount > bb->sampleReadCount)
                bb->elapsedSampleCounts = bb->sampleReadCount + (MAXUINT - bb->lastSignalEdgeSampleCount);
            else
                bb->elapsedSampleCounts = bb->sampleReadCount - bb->lastSignalEdgeSampleCount;
        }

            unsigned int isBuddyBoxWireSignalHigh(BuddyBox *bb)
            {
                return (bb->wireSignal == SIGNAL_HIGH);
            }

            void processHighBuddyBoxWireSignal(BuddyBox *bb)
            {
                if (isBuddyBoxSynchroFrameEncountered(bb))
                    processBuddyBoxSynchroFrame(bb);
                else if (isBuddyBoxChannelValid(bb))
                    targetNextBuddyBoxPacketChannel(bb);
                else if (!isBuddyBoxCalibrating(bb))
                    handleInvalidBuddyBoxChannel(bb);
            }

                unsigned int isBuddyBoxSynchroFrameEncountered(BuddyBox *bb)
                {
                    return (bb->currentSignalChannel >= MIN_CHANNELS && (bb->elapsedSampleCounts > bb->localMaxElapsedCount / 2));
                }

                void processBuddyBoxSynchroFrame(BuddyBox *bb)
                {
                    bb->synchroFrameCount++;
                    if (!isBuddyBoxCalibrating(bb))
                    {
                        if (isBuddyBoxChannelCountValid(bb))
                        {
                            processBuddyBoxPacket(bb);
                            storeBuddyBoxChannelCount(bb);
                        }
                        else
                            handleInvalidBuddyBoxChannelCount(bb);
                    }
                    else
                        storeBuddyBoxChannelCount(bb);
                    targetNextBuddyBoxPacket(bb);
                }

                    unsigned int isBuddyBoxChannelCountValid(BuddyBox *bb)
                    {
                        return (getCurrentBuddyBoxChannel(bb) == bb->channelCount);
                    }

                        unsigned int getCurrentBuddyBoxChannel(BuddyBox *bb)
                        {
                            return bb->currentSignalChannel - 1;
                        }

                    void storeBuddyBoxChannelCount(BuddyBox *bb)
                    {
                        bb->channelCount = getCurrentBuddyBoxChannel(bb);
                    }

                    void handleInvalidBuddyBoxChannelCount(BuddyBox *bb)
                    {
                        bb->badPacketCount++;
                        if (!isBuddyBoxSignalViable(bb))
                        {
                            printf("Channel Count has Changed...%d\n", bb->synchroFrameCount);
                            disconnectBuddyBox(bb);
                        }   
                    }

                    void targetNextBuddyBoxPacket(BuddyBox *bb)
                    {
                        bb->currentSignalChannel = 0;
                    }

                unsigned int isBuddyBoxChannelValid(BuddyBox *bb)
                {
                    return (bb->currentSignalChannel < MAX_CHANNELS);
                }

                void processBuddyBoxPacket(BuddyBox *bb)
                {
                    unsigned int i;

                    for (i = 0; i < MAX_CHANNELS; i++)
                        if (i < bb->currentSignalChannel)
                            printf("%u\t,", bb->signal[i]);
                    printf("%u\n", bb->synchroFrameCount);
                    
                    bb->badPacketCount = 0;
                }

                void targetNextBuddyBoxPacketChannel(BuddyBox *bb)
                {
                    bb->signal[bb->currentSignalChannel] = bb->elapsedSampleCounts;
                    bb->currentSignalChannel++;
                }

                void handleInvalidBuddyBoxChannel(BuddyBox *bb)
                {
                    bb->badPacketCount++;
                    if (!isBuddyBoxSignalViable(bb))
                    {
                        printf("Invalid Channel Received...\n");
                        disconnectBuddyBox(bb);
                    }
                }

                unsigned int isBuddyBoxSignalViable(BuddyBox *bb)
                {
                    return (bb->badPacketCount < BAD_PACKET_THRESHOLD);
                }

    unsigned int isBuddyBoxCalibrating(BuddyBox *bb)
    {
        return (bb->synchroFrameCount < CALIBRATION_PACKETS);
    }

    void calibrateBuddyBox(BuddyBox *bb, float tmpLocalMinSample, float tmpLocalMaxSample, unsigned int tmpLocalMaxElapsedCount)
    {
        bb->localMinSample = tmpLocalMinSample;
        bb->localMaxSample = tmpLocalMaxSample;
        bb->localMaxElapsedCount = tmpLocalMaxElapsedCount;
    }

void writeBuddyBoxChannelsIntoBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int sampleRate)
{
    unsigned int i, j, endJ, channel;
    bb->signal[0] = 87;
    bb->signal[1] = 138;
    bb->signal[2] = 137;
    bb->signal[3] = 138;
    bb->signal[4] = 187;
    bb->signal[5] = 117;
    bb->signal[6] = 162;
    bb->signal[7] = 182;
    bb->signal[8] = 187;

    for (i = 0; i < bb->overflowSampleCount; i++)
    {
        buffer[i] = bb->overflowPacket[i];
    }
    
    bb->overflowSampleCount = 0;
    
    while (i < bufferSize)
    {
        channel = 0;
        j = 0;
//printf("LOOPING FOR %u, i=%d\n", sampleRate * PACKET_DURATION / MICROSECONDS_PER_SECOND, i);
        while (j < sampleRate * PACKET_DURATION / MICROSECONDS_PER_SECOND)
        {
            if (channel < 9)//bb->channelCount)
            {
                endJ = j + SEPARATOR_DURATION * sampleRate / MICROSECONDS_PER_SECOND;
                while (j < endJ)
                {
                    if (i + j < bufferSize)
                    {
//printf("SEPARATOR %u: %u / %u - %f\n", channel, j, endJ, SIGNAL_HIGH_FLOAT);
                        buffer[i + j] = SIGNAL_HIGH_FLOAT;
                        bb->sampleWriteCount++;
                    }
                    else
                    {
                        bb->overflowPacket[i + j - bufferSize] = SIGNAL_HIGH_FLOAT;
                        bb->overflowSampleCount++;
                    }
                    j++;
                }
                endJ = j + bb->signal[channel];
                while (j < endJ)
                {
                    if (i + j < bufferSize)
                    {
//printf("SIGNAL %u: %u / %u - %f\n", channel, j, endJ, SIGNAL_LOW_FLOAT);
                        buffer[i + j] = SIGNAL_LOW_FLOAT;
                        bb->sampleWriteCount++;
                    }
                    else
                    {
                        bb->overflowPacket[i + j - bufferSize] = SIGNAL_LOW_FLOAT;
                        bb->overflowSampleCount++;
                    }
                    j++;
                }
            }
            else
            {
                if (i + j < bufferSize)
                {
//printf("SYNCHRO: %u / %u - %f\n", j, bufferSize, SIGNAL_LOW_FLOAT);
                    buffer[i + j] = SIGNAL_LOW_FLOAT;
                    bb->sampleWriteCount++;
                }
                else
                {
                    bb->overflowPacket[i + j - bufferSize] = SIGNAL_LOW_FLOAT;
                    bb->overflowSampleCount++;
                }
                j++;
            }
            channel++;
        }
        i += sampleRate * PACKET_DURATION / MICROSECONDS_PER_SECOND;
    }
    
    for (i = 0; i < bufferSize; i++)
        printf("%f\n",buffer[i]);
}

void disconnectBuddyBox(BuddyBox *bb)
{
    bb->active = 0;
    printf("Buddy Box Disconnected...\n");
}
