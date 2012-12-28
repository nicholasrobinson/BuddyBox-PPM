//
//  BuddyBox.h
//  BuddyBox
//
//  Created by Nicholas Robinson on 12/23/12.
//  Copyright (c) 2012 Nicholas Robinson. All rights reserved.
//

#ifndef BuddyBox_BuddyBox_h
#define BuddyBox_BuddyBox_h

#define MAXUINT                 (0xffffffff)
#define MICROSECONDS_PER_SECOND (1000000)

static const unsigned int MIN_CHANNELS                  = 5;
static const unsigned int MAX_CHANNELS                  = 20;
static const unsigned int SIGNAL_HIGH                   = 1;
static const unsigned int SIGNAL_LOW                    = 0;
static const unsigned int CALIBRATION_PACKETS           = 50;       // @50 Packets per second -> ~1 second calibrating
static const unsigned int BAD_PACKET_THRESHOLD          = 10;
static const unsigned int OVERFLOW_SAMPLES              = 4096;
static const float SAMPLE_NOISE_THRESHOLD               = 0.1f;

static const unsigned int PACKET_DURATION               = 20000;    // 20000 us
static const unsigned int SEPARATOR_DURATION            = 400;      // 400 us
static const unsigned int CHANNEL_MIN_DURATION          = 500;      // 500 us
static const unsigned int CHANNEL_MAX_DURATION          = 1700;     // 1700 us
static const float SIGNAL_HIGH_FLOAT                    = 0.75f;
static const float SIGNAL_LOW_FLOAT                     = -0.23f;

typedef struct
{
    unsigned int signal[MAX_CHANNELS];
    unsigned int channelCount;
    unsigned int active;
    
    unsigned int wireSignal;
    unsigned int localMaxElapsedCount;
    unsigned int sampleReadCount;
    unsigned int sampleWriteCount;
    unsigned int synchroFrameCount;
    unsigned int sampleCountOverflow;
    unsigned int lastSignalEdgeSampleCount;
    unsigned int elapsedSampleCounts;
    unsigned int currentSignalChannel;
    unsigned int badPacketCount;
    unsigned int overflowSampleCount;
    float localMinSample;
    float localMaxSample;
    float overflowPacket[OVERFLOW_SAMPLES];
} BuddyBox;

void initializeBuddyBox(BuddyBox* bb);

void readBufferIntoBuddyBox(BuddyBox* bb, float* buffer, unsigned int bufferSize);
    void detectBuddyBoxTimeout(BuddyBox *bb, float* buffer, unsigned int bufferSize);
        unsigned int isBuddyBoxSignalAboveNoiseThreshold(float bufferSample);
        void handleBuddyBoxTimeout(BuddyBox *bb);
    float getBuddyBoxSampleMagnitude(float sample);
    float getBuddyBoxTmpLocalMinSample(float bufferSample, float tmpLocalMinSample);
    float getBuddyBoxTmpLocalMaxSample(float bufferSample, float tmpLocalMaxSample);
    float getBuddyBoxTmpLocalMaxElapsedCount(BuddyBox *bb, unsigned int tmpLocalMaxElapsedCount, unsigned int bufferSize);
    unsigned int isBuddyBoxSignalEdge(BuddyBox *bb, float bufferSample);
        unsigned int isBuddyBoxRawSignalHigh(BuddyBox *bb, float bufferSample);
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
    void calibrateBuddyBox(BuddyBox *bb, float tmpLocalMinSample, float tmpLocalMaxSample, unsigned int tmpLocalMaxElapsedCount);

void writeBuddyBoxChannelsIntoBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int sampleRate);

void disconnectBuddyBox(BuddyBox *bb);

#endif
