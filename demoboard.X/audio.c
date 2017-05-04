#include "audio.h"
#include "soundfx.h"
#include <xc.h>

#define DEMOSONG
#include "songs.h"

void audio_setup() {
    PR1 = 256;
    _T1IP = 5;	// set interrupt priority
    _TON  = 1;	// turn on the timer
    _T1IF = 0;	// reset interrupt flag
    _T1IE = 1;	// turn on the timer1 interrupt
}


static unsigned int notendx=-1;
static unsigned short channdx1=0;
static unsigned short channdx2=0;
static unsigned short channdx3=0;
static unsigned short channdx4=0;

static unsigned short cf1=0;
static unsigned short ca1=0;

static unsigned short cf2=0;
static unsigned short ca2=0;

static unsigned short cf3=0;
static unsigned short ca3=0;

static unsigned short cf4=0;
static unsigned short ca4=0;

// This allows to notes to be turned on or off based on a bitmask:
//              Note Off     Note On
// Channel 1:      1            2
// Channel 2:      4            8
// Channel 3:      16           32
// Channel 4:      64           128
static unsigned short filt;

static unsigned short samplendx=0;

void __attribute__((__interrupt__)) _T1Interrupt(void);
void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{
    unsigned short sample=0;
    unsigned short sample2;

    // Handle channel 1
    if(cf1>channdx1<<1) {
        sample=ca1;
    }
    if(filt&1 && samplendx>14000) {
        sample=0;
    }
    if(filt&2 && samplendx&1024) {
        sample>>=1;
    }

    // Handle channel 2
    if(cf2>channdx2<<1) {
        sample2=ca2;
    } else {
        sample2=0;
    }
    if(filt&4 && samplendx>14000) {
        sample2=0;
    }
    if(filt&8 && samplendx&1024) {
        sample2>>=1;
    }

    // First Mix: Channel 1 and Channel 2:
    sample+=sample2;

    // Handle channel 3
    if(cf3>channdx3<<1) {
        sample2=ca3;
    } else {
        sample2=0;
    }
    if(filt&16 && samplendx>14000) {
        sample2=0;
    }
    if(filt&32 && samplendx&1024) {
        sample2>>=1;
    }

    // Second Mix: First Mix with Channel 3:
    sample+=sample2;

    // Handle channel 4
    if(cf4>channdx4<<1) {
        sample2=ca4;
    } else {
        sample2=0;
    }
    if(filt&64 && samplendx>14000) {
        sample2=0;
    }
    if(filt&128 && samplendx&1024) {
        sample2>>=1;
    }
    // Third Mix: Second Mix with Channel 4:
    sample+=sample2;

    // Spit the mixed value to the audio port:
    PORTB=(sample<<8);

    // Step all of the channels through to their next note and bound check/loop
    // them back to the beginning
    channdx1++;
    if(channdx1>cf1) {
        channdx1=0;
    }
    channdx2++;
    if(channdx2>cf2) {
        channdx2=0;
    }
    channdx3++;
    if(channdx3>cf3) {
        channdx3=0;
    }
    channdx4++;
    if(channdx4>cf4) {
        channdx4=0;
    }


    samplendx++;
    if(samplendx==16384) {
        samplendx=0;
        notendx++;
        if(notendx==(sizeof(c1f)/sizeof(c1f[0]))) {
            notendx=0;
        }

        // Pull in all channel values for the next time the interrupt fires:
        cf1=c1f[notendx];
        ca1=c1a[notendx];
        cf2=c2f[notendx];
        ca2=c2a[notendx];
        cf3=c3f[notendx];
        ca3=c3a[notendx];
        cf4=c4f[notendx];
        ca4=c4a[notendx];
        filt=fltr[notendx];
    }
    _T1IF = 0;
}
