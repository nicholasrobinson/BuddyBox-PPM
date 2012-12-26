//
//  BuddyBox.h
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#ifndef BuddyBox_BuddyBox_h
#define BuddyBox_BuddyBox_h

#define MAXUINT (0xffffffff)

static const unsigned int MIN_CHANNELS                  = 5;
static const unsigned int MAX_CHANNELS                  = 20;
static const unsigned int SIGNAL_HIGH                   = 1;
static const unsigned int SIGNAL_LOW                    = 0;
static const unsigned int CALIBRATION_PACKETS           = 50;    // @50Hz -> ~1 second calibrating
static const unsigned int BAD_PACKET_THRESHOLD          = 10;
static const float SAMPLE_NOISE_THRESHOLD               = 0.1f;

typedef struct
{
    unsigned int signal[MAX_CHANNELS];
    unsigned int channelCount;
    unsigned int active;
    
    unsigned int wireSignal;
    float localMaxSample;
    unsigned int localMaxElapsedCount;
    unsigned int sampleCount;
    unsigned int synchroFrameCount;
    unsigned int sampleCountOverflow;
    unsigned int lastSignalEdgeSampleCount;
    unsigned int elapsedSampleCounts;
    unsigned int currentSignalChannel;
    unsigned int badPacketCount;
} BuddyBox;

void initializeBuddyBox(BuddyBox* bb);

void readBufferIntoBuddyBox(BuddyBox* bb, float* buffer, unsigned int bufferSize);
    void detectBuddyBoxTimeout(BuddyBox *bb, float* buffer, unsigned int bufferSize);
        unsigned int isBuddyBoxSignalAboveNoiseThreshold(float bufferSampleMagnitude);
        void handleBuddyBoxTimeout(BuddyBox *bb);
    float getBuddyBoxSampleMagnitude(float sample);
    float getBuddyBoxTmpLocalMaxSample(float bufferSampleMagnitude, float tmpLocalMaxSample);
    float getBuddyBoxTmpLocalMaxElapsedCount(BuddyBox *bb, unsigned int tmpLocalMaxElapsedCount, unsigned int bufferSize);
    unsigned int isBuddyBoxSignalEdge(BuddyBox *bb, float bufferSampleMagnitude);
        unsigned int isBuddyBoxRawSignalHigh(BuddyBox *bb, float bufferSampleMagnitude);
    void processBuddyBoxSignalEdge(BuddyBox *bb);
        void updateBuddyBoxElapsedCounts(BuddyBox *bb);
            unsigned int isBuddyBoxWireSignalHigh(BuddyBox *bb);
            void processHighBuddyBoxWireSignal(BuddyBox *bb);
                unsigned int isBuddyBoxSynchroFrameEncountered(BuddyBox *bb);
                void processBuddyBoxSynchroFrame(BuddyBox *bb);
                    unsigned int isBuddyBoxChannelCountValid(BuddyBox *bb);
                        unsigned int getCurrentBuddyBoxChannel(BuddyBox *bb);
                    void storeBuddyBoxChannelCount(BuddyBox *bb);
                    void handleInvalidBuddyBoxChannelCount(BuddyBox *bb);
                        unsigned int isBuddyBoxSignalViable(BuddyBox *bb);
                    void targetNextBuddyBoxPacket(BuddyBox *bb);
                unsigned int isBuddyBoxChannelValid(BuddyBox *bb);
                void processBuddyBoxPacket(BuddyBox *bb);
                void targetNextBuddyBoxPacketChannel(BuddyBox *bb);
                void handleInvalidBuddyBoxChannel(BuddyBox *bb);
    unsigned int isBuddyBoxCalibrating(BuddyBox *bb);
    void calibrateBuddyBox(BuddyBox *bb, float tmpLocalMaxSample, unsigned int tmpLocalMaxElapsedCount);

void disconnectBuddyBox(BuddyBox *bb);

#endif
