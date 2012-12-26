# JR / Spektrum / Futaba BuddyBox PPM Audio Driver

by Nicholas Robinson

## Overview

This driver allows a JR (Japan Remove Control Co., Ltd.), Spektrum (Horizon Hobby, Inc.) or Futaba (Hobbico, Inc.) radio transmitter to communicate with a PC (Windows / OS X / Linux). It is written in ANSI C, leveraging the excellent PortAudio Portable Cross-Platform Audio I/O library, and facilitates one-way communication from the radio transmitter to the PC via any available audio input device (microphone-in, line-in etc), using output from the transmitter's trainer port.

## Requirements

* Make
* cc/gcc
* PortAudio
* JR / Spektrum / Futaba Radio Transmitter with Trainer Port
* Trainer Cable
* Sound card with microphone-in / line-in

## Installation

    $ git clone git://github.com/nicholasrobinson/BuddyBox-PPM.git
    $ cd BuddyBox-PPM
    $ make clean && make
    
## Usage

### Execution

    $ ./BuddyBox
    
### Sample Output

    Initializing Buddy Box...
    
    (Plug in your transmitter cable: Tx first, line-in / microphone-in second)
    
    137     ,214	,214	,214	,291	,183	,253	,290	,291	,51
    137     ,214	,214	,213	,290	,183	,253	,290	,291	,52
    138     ,214	,214	,213	,291	,184	,252	,291	,291	,53
    138     ,342	,214	,214	,291	,184	,252	,291	,290	,54
    138     ,214	,214	,214	,291	,184	,252	,290	,291	,55
    137     ,214	,214	,214	,291	,183	,252	,290	,291	,56
    137     ,213	,214	,214	,291	,183	,253	,290	,291	,57
    
    (ctrl-c pressed)
    
    Program Halted...
    
### Sample Output Explanation

The above output indicates the raw output of 9 channels from my JR PCM9xII Tx, represented by the first 9 columns. The final column is an incremental sequential packet number.

## References
    
* http://www.portaudio.com/
* http://spektrumrc.com/
* http://www.jrpropo.co.jp/
* http://www.futaba-rc.com/

# Notes

* You may need to change "PA_INPUT_DEVICE" in "PortAudioStream.h" to suit your audio input  device.
* You may need to tune "SAMPLE_RATE" in "PortAudioStream.h" to suit your audio input device. A higher sample rate will yield a higher resolution on each channel's output. For example 44.1kHz sample rate -> resolution of ~36 per channel, whereas 192kHz sample rate -> resolution of ~154 per channel
* Depending on your sound card's characteristics, you may need to tweak "BAD_PACKET_THRESHOLD" and "SAMPLE_NOISE_THRESHOLD" in "BuddyBox.h"
* Whilst this driver and the supplied "example" implementation could be used directly it is perhaps most usefully placed in a separate thread, sharing a pointer to "pas.signal", allowing real-time access to each channel's output. Examples coming soon...

Please let me know if you find this useful or come up with any novel implementations.

Enjoy!

Nicholas Robinson

me@nicholassavilerobinson.com
