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

static const unsigned int MIN_CHANNELS                  = 4;
static const unsigned int MAX_CHANNELS                  = 10;
static const unsigned int SIGNAL_HIGH                   = 1;
static const unsigned int SIGNAL_LOW                    = 0;
static const unsigned int CALIBRATION_SAMPLES           = 10000;

typedef struct
{
    // Public
    unsigned int signal[MAX_CHANNELS];
    unsigned int channelCount;
    unsigned int active;
    
    // Private
    unsigned int wireSignal;
    float localMaxSample;
    unsigned int localMaxElapsedCount;
    unsigned int sampleCount;
    unsigned int sampleCountOverflow;
    unsigned int lastSignalEdgeSampleCount;
    unsigned int elapsedSampleCounts;
    unsigned int currentSignalChannel;
} BuddyBox;

// Public
void initializeBuddyBox(BuddyBox* bb);
void readBufferIntoBuddyBox(BuddyBox* bb, float* buffer, unsigned int bufferSize);
void disconnectBuddyBox(BuddyBox *bb);

// Private
void detectBuddyBoxTimeout(BuddyBox *bb, float* buffer, unsigned int bufferSize);
float getBuddyBoxSampleMagnitude(float sample);
float getBuddyBoxTmpLocalMaxSample(float bufferSampleMagnitude, float tmpLocalMaxSample);
float getBuddyBoxTmpLocalMaxElapsedCount(BuddyBox *bb, unsigned int tmpLocalMaxElapsedCount, unsigned int bufferSize);
void incrementBuddyBoxSampleCount(BuddyBox *bb);
unsigned int isBuddyBoxSignalEdge(BuddyBox *bb, float bufferSampleMagnitude);
unsigned int isBuddyBoxRawSignalHigh(BuddyBox *bb, float bufferSampleMagnitude);
void processBuddyBoxSignalEdge(BuddyBox *bb);
void updateBuddyBoxElapsedCounts(BuddyBox *bb);
unsigned int isBuddyBoxWireSignalHigh(BuddyBox *bb);
void processHighBuddyBoxWireSignal(BuddyBox *bb);
unsigned int isBuddyBoxSynchroFrameEncountered(BuddyBox *bb);
void processBuddyBoxSynchroFrame(BuddyBox *bb);
void processBuddyBoxPacket(BuddyBox *bb);
void storeBuddyBoxChannelCount(BuddyBox *bb);
void targetNextBuddyBoxPacket(BuddyBox *bb);
unsigned int isBuddyBoxChannelValid(BuddyBox *bb);
unsigned int isBuddyBoxCalibrating(BuddyBox *bb);
unsigned int isBuddyBoxChannelCountValid(BuddyBox *bb);
void targetNextBuddyBoxPacketChannel(BuddyBox *bb);

#endif
