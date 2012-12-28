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
    
    bb->wireSignal                  = SIGNAL_LOW;
    bb->channelCount                = 0;
    bb->active                      = 1;
    
    bb->localMinSample              = 0.0f;
    bb->localMaxSample              = 0.0f;
    bb->localMaxElapsedCount        = 0;
    bb->sampleCount                 = 0;
    bb->synchroFrameCount           = 0;
    bb->lastSignalEdgeSampleCount   = 0;
    bb->elapsedSampleCounts         = 0;
    bb->currentSignalChannel        = 0;
    bb->badPacketCount              = 0;
}

void readBufferIntoBuddyBox(BuddyBox *bb, float* buffer, unsigned int bufferSize)
{
    float tmpLocalMinSample, tmpLocalMaxSample;
    unsigned int i, tmpLocalMaxElapsedCount;
    
    detectBuddyBoxTimeout(bb, buffer, bufferSize);
    
    tmpLocalMinSample = 0.0f;
    tmpLocalMaxSample = 0.0f;
    tmpLocalMaxElapsedCount = 0;
    for (i = 0; bb->active && i < bufferSize; i++, bb->sampleCount++)
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
        bb->lastSignalEdgeSampleCount = bb->sampleCount;
    }

        void updateBuddyBoxElapsedCounts(BuddyBox *bb)
        {
            if (bb->lastSignalEdgeSampleCount > bb->sampleCount)
                bb->elapsedSampleCounts = bb->sampleCount + (MAXUINT - bb->lastSignalEdgeSampleCount);
            else
                bb->elapsedSampleCounts = bb->sampleCount - bb->lastSignalEdgeSampleCount;
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
                            printf("%d\t,", bb->signal[i]);
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

void disconnectBuddyBox(BuddyBox *bb)
{
    bb->active = 0;
    printf("Buddy Box Disconnected...\n");
}
