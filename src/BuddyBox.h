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
static const unsigned int CALIBRATION_FRAMES            = 50;       // @50 Frames per second -> ~1 second calibrating
static const unsigned int BAD_FRAME_THRESHOLD           = 10;
static const unsigned int OVERFLOW_SAMPLES              = 4096;
static const float SIGNAL_NOISE_THRESHOLD               = 0.1f;

static const unsigned int FRAME_DURATION                = 20000;    // 20000 us
static const unsigned int SEPARATOR_DURATION            = 400;      // 400 us
static const unsigned int CHANNEL_MIN_DURATION          = 500;      // 500 us
static const unsigned int CHANNEL_MAX_DURATION          = 1700;     // 1700 us
static const float SIGNAL_HIGH_FLOAT                    = 1.0f;    // Experimentally: 0.75f;
static const float SIGNAL_LOW_FLOAT                     = -1.0f;     // Experimentally: -0.23f;

typedef struct
{
    unsigned int active;
    unsigned int negativeShift;
    unsigned int sampleRate;
    unsigned int input;
    unsigned int lastInput;
    unsigned int inputChannel;
    unsigned int inputChannelCount;
    unsigned int outputChannelCount;
    unsigned int inputSampleCount;
    unsigned int outputSampleCount;
    unsigned int elapsedInputSampleCounts;
    unsigned int maxElapsedInputSampleCount;
    unsigned int lastInputEdgeSampleCount;
    unsigned int inputSynchroFrameCount;
    unsigned int badInputFrameCount;
    unsigned int outputOverflowSampleCount;
    unsigned int outputOverflowBufferSize;
    unsigned int inputChannelBuffer[MAX_CHANNELS];
    unsigned int outputChannelBuffer[MAX_CHANNELS];
    float minInputSample;
    float maxInputSample;
    float inputChannelValues[MAX_CHANNELS];
    float* outputOverflowBuffer;
} BuddyBox;

void initializeBuddyBox(BuddyBox *bb, unsigned int sampleRate);
    void allocateOutputOverflowBuffer(BuddyBox *bb);

void readBufferIntoBuddyBoxInputChannelBuffer(BuddyBox* bb, float* buffer, unsigned int bufferSize);
    void detectBuddyBoxInputTimeout(BuddyBox *bb, float* buffer, unsigned int bufferSize);
        unsigned int isBuddyBoxRawInputAboveNoiseThreshold(float bufferSample);
        void handleBuddyBoxInputTimeout(BuddyBox *bb);
    float getBuddyBoxSampleMagnitude(float sample);
    float getBuddyBoxLocalMinSample(float bufferSample, float localMinSample);
    float getBuddyBoxLocalMaxSample(float bufferSample, float localMaxSample);
    float getBuddyBoxLocalMaxElapsedInputSampleCount(BuddyBox *bb, unsigned int localMaxElapsedInputSampleCount, unsigned int bufferSize);
    unsigned int getBuddyBoxLocalMaxElapsedInput(BuddyBox *bb, unsigned int localMaxElapsedInput, unsigned int localMaxElapsedInputSampleCount);
    void processBuddyBoxRawInput(BuddyBox *bb, float bufferSample);
    unsigned int isBuddyBoxInputEdge(BuddyBox *bb, float bufferSample);
        unsigned int isBuddyBoxRawInputHigh(BuddyBox *bb, float bufferSample);
    void processBuddyBoxInputEdge(BuddyBox *bb);
        void updateBuddyBoxElapsedInputSampleCounts(BuddyBox *bb);
            unsigned int isBuddyBoxInputHigh(BuddyBox *bb);
            void processHighBuddyBoxInput(BuddyBox *bb);
                unsigned int isBuddyBoxInputSynchroFrame(BuddyBox *bb);
                void processBuddyBoxInputSynchroFrame(BuddyBox *bb);
                    unsigned int isBuddyBoxInputChannelCountValid(BuddyBox *bb);
                    void storeBuddyBoxInputChannelCount(BuddyBox *bb);
                    void handleInvalidBuddyBoxInputChannelCount(BuddyBox *bb);
                        unsigned int isBuddyBoxInputViable(BuddyBox *bb);
                    void targetNextBuddyBoxInputFrame(BuddyBox *bb);
                unsigned int isBuddyBoxInputChannelValid(BuddyBox *bb);
                void processBuddyBoxInputFrame(BuddyBox *bb);
                void storeBuddyBoxInputChannel(BuddyBox *bb);
                void handleInvalidBuddyBoxInputChannel(BuddyBox *bb);
                void targetNextBuddyBoxInputChannel(BuddyBox *bb);
    unsigned int isBuddyBoxInputCalibrating(BuddyBox *bb);
    void calibrateBuddyBoxInput(BuddyBox *bb, float localMinSample, float localMaxSample, unsigned int localMaxElapsedCount, unsigned int localMaxElapsedInput);

void setBuddyBoxOutputChannelValue(BuddyBox *bb, unsigned int channel, float channelValue);
    void setBuddyBoxOutputChannelDuration(BuddyBox *bb, unsigned int channel, unsigned int channelDuration);

void writeBuddyBoxOutputChannelBufferIntoBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSize);
    unsigned int writeBuddyBoxOverflowBufferIntoBuffer(BuddyBox *bb, float* buffer);
    unsigned int writeBuddyBoxOutputChannelBufferIntoBufferFrame(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount);
        unsigned int writeBuddyBoxOutputChannelBufferIntoBufferChannels(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount);
            unsigned int writeBuddyBoxOutputChannelBufferIntoBufferChannel(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount, unsigned int channelsSampleCount, unsigned int channel);
                unsigned int writeBuddyBoxChannelSeperatorIntoBufferChannel(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int startBufferSample);
                        unsigned int writeBuddyBoxSignalsToBufferOrOverflowBuffer(BuddyBox *bb, float *buffer, unsigned int bufferSize, unsigned int startBufferSample, unsigned int comparatorOffset, unsigned int endBufferSampleOffset, float signal);
                            void writeBuddyBoxSignalToBufferOrOverflowBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSample, float signal);
                                void writeBuddyBoxSignalToBuffer(BuddyBox *bb, float buffer[], unsigned int bufferSample, float signal);
                                void writeBuddyBoxSignalToOverflowBuffer(BuddyBox *bb, unsigned int bufferSample, float signal);
            unsigned int writeBuddyBoxChannelDurationIntoBufferChannel(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int startBufferSample, unsigned int endBufferSampleOffset);
        unsigned int writeBuddyBoxOutputChannelBufferIntoBufferSynchro(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int bufferSampleCount, unsigned int frameSampleCount);
            unsigned int writeBuddyBoxSynchroIntoBufferSynchro(BuddyBox *bb, float buffer[], unsigned int bufferSize, unsigned int startBufferSample, unsigned int comparatorOffset);

void disconnectBuddyBox(BuddyBox *bb);

#endif
