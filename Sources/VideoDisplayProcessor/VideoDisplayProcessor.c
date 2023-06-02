//
//  VideoDisplayProcessor.c
//  
//
//  Created by Nate Rivard on 30/05/2023.
//

#include "VideoDisplayProcessor.h"

#include <memory.h>
#include <stdlib.h>

#define kVDPRegisterCount   8
#define kVDPUnsetAddr       0xFFFF  // 0xFFFF is outside of valid vram space so we're using this to express the register is in stage 1

struct __VideoDisplayProcessor {
    /// write-only registers
    uint8_t registers[kVDPRegisterCount];

    /// dedicated vram
    uint8_t vram[kVDPVramSize];

    /// read-only status register
    uint8_t status;

    /// current vram address for reading/writing (auto-increments on access)
    uint16_t vramAddr;

    /// first "byte" of a register write pair
    /// if kVDPUnsetAddr, register is in "reset" mode (waiting for 1st byte)
    /// if any other value, it is waiting for a value pair to complete the register write process (waiting for 2nd byte)
    uint16_t registerValue;

    /// the VDP has a read-ahead buffer which is important to simulate to get proper real-world behavior
    uint8_t readAheadBuffer;
};

#pragma mark Lifecycle

VideoDisplayProcessorRef VDPCreate() {
    struct __VideoDisplayProcessor *vdp = malloc(sizeof(struct __VideoDisplayProcessor));

    if (vdp) {
        VDPReset(vdp);
    }
    
    return vdp;
}

void VDPDestroy(VideoDisplayProcessorRef vdp) {
    if (vdp) {
        free(vdp);
        vdp = NULL;
    }
}

#pragma mark Hardware API

void VDPReset(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return;
    }

    vdp->status = 0;
    vdp->vramAddr = 0;
    vdp->readAheadBuffer = 0;
    vdp->registerValue = kVDPUnsetAddr;
    memset(vdp->registers, 0, sizeof(vdp->registers));
}

void VDPWriteToRegisterPort(VideoDisplayProcessorRef vdp, uint8_t value) {
    if (!vdp) {
        return;
    }

    if (vdp->registerValue == kVDPUnsetAddr) {
        vdp->registerValue = value;
    } else {
        if (value & kVDPRegisterWriteMask) {
            // register write
            vdp->registers[value & (kVDPRegisterCount - 1)] = vdp->registerValue & 0xFF;
        } else {
            // vram address
            vdp->vramAddr = ((value & (kVDPVramWriteMask - 1)) << 8) | vdp->registerValue;
            if (!(value & kVDPVramWriteMask)) {
                // this primes `readAheadBuffer` and properly sets r/w pointer for reading
                VDPReadFromDataPort(vdp);
            }
        }

        vdp->registerValue = kVDPUnsetAddr;
    }
}

uint8_t VDPReadFromRegisterPort(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return;
    }

    uint8_t status = vdp->status;
    vdp->status = 0;                    // a read resets status
    vdp->registerValue = kVDPUnsetAddr; // a read resets register write mode
    return status;
}

void VDPWriteToDataPort(VideoDisplayProcessorRef vdp, uint8_t value) {
    if (!vdp) {
        return;
    }

    // write value at current vram addr
    vdp->vram[vdp->vramAddr] = value;

    // fill read-ahead buffer with the passed in value
    vdp->readAheadBuffer = value;

    // advance the pointer
    vdp->vramAddr = (vdp->vramAddr + 1) % kVDPVramSize;
}

// the logic here is taken from a deep-dive into the actual hardware from
// this forum post: http://bifi.msxnet.org/msxnet/tech/tms9918a.txt
// see section 2.1 "Memory Access"
uint8_t VDPReadFromDataPort(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return 0;
    }

    uint8_t data = vdp->readAheadBuffer;

    // fill read-ahead buffer with value pointed at by current address
    vdp->readAheadBuffer = vdp->vram[vdp->vramAddr];

    // advance the pointer
    vdp->vramAddr = (vdp->vramAddr + 1) % kVDPVramSize;

    return data;
}

#pragma mark Video Display

void VDPGetScanline(VideoDisplayProcessorRef vdp, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX]) {
    if (!vdp) {
        return;
    }

    
}

#pragma mark Debugger Access

uint8_t VDPGetRegister(VideoDisplayProcessorRef vdp, uint8_t registerIdx) {
    if (!vdp) {
        return 0;
    }

    return vdp->registers[registerIdx & (kVDPRegisterCount - 1)];
}

void VDPSetRegister(VideoDisplayProcessorRef vdp, uint8_t registerIdx, uint8_t value) {
    if (!vdp) {
        return;
    }

    vdp->registers[registerIdx & (kVDPRegisterCount - 1)] = value;
}

void VDPGetVramContents(VideoDisplayProcessorRef vdp, uint8_t vramBuffer[kVDPVramSize]) {
    if (!vdp) {
        return;
    }

    memcpy(vramBuffer, vdp->vram, kVDPVramSize);
}

uint8_t VDPGetVram(VideoDisplayProcessorRef vdp, uint16_t addr) {
    if (!vdp) {
        return 0;
    }

    return vdp->vram[addr & (kVDPVramSize - 1)];
}

void VDPSetVram(VideoDisplayProcessorRef vdp, uint16_t addr, uint8_t value) {
    if (!vdp) {
        return;
    }

    vdp->vram[addr & (kVDPVramSize - 1)] = value;
}

uint16_t VDPGetVramAddress(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return 0;
    }

    return vdp->vramAddr;
}

void VDPSetVramAddress(VideoDisplayProcessorRef vdp, uint16_t addr) {
    if (!vdp) {
        return;
    }

    vdp->vramAddr = addr;
}

VDPGraphicsMode VDPGetGraphicsMode(VideoDisplayProcessorRef vdp) {
    return ((vdp->registers[1] & 0x03) << 1) | (vdp->registers[0] & 0x01);
}

#pragma mark Utilities

inline VDPColor VDPGetBackgroundColor(VideoDisplayProcessorRef vdp) {
    return vdp->registers[0x7] & 0x0F;
}

inline VDPColor VDPGetForegroundColor(VideoDisplayProcessorRef vdp) {
    return vdp->registers[0x7] >> 4;
}
