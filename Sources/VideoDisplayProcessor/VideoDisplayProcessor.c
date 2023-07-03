//
//  VideoDisplayProcessor.c
//  
//
//  Created by Nate Rivard on 30/05/2023.
//

#include "VideoDisplayProcessor.h"
#include "VideoDisplayProcessor_Private.h"
#include "GraphicsMode1.h"

#include <memory.h>
#include <stdlib.h>

#define kVDPRegisterCount   8
#define kVDPVramMask        (kVDPVramSize - 1)
#define kVDPUnsetAddr       0xFFFF  // 0xFFFF is outside of valid vram space so we're using this to express the register is in stage 1
#define kVDPDisplayEnMask   0b01000000
#define kVDPIRQEnMask       0b00100000
#define kVDPStatusIRQMask    0b10000000
#define kVDPStatus5thSprMask 0b01000000
#define kVDPStatusSprCoMask  0b0010000

const uint32_t VDPColorPaletteArray[] = {
    0x00000000, // kVDPColorTransparent
    0x000000ff, // kVDPColorBlack
    0x21c942ff, // kVDPColorMediumGreen
    0x5edc78ff, // kVDPColorLightGreen
    0x5455edff, // kVDPColorDarkBlue
    0x7d75fcff, // kVDPColorLightBlue
    0xd3524dff, // kVDPColorDarkRed
    0x43ebf6ff, // kVDPColorCyan
    0xfd5554ff, // kVDPColorMediumRed
    0xff7978ff, // kVDPColorLightRed
    0xd3c153ff, // kVDPColorDarkYellow
    0xe5ce80ff, // kVDPColorLightYellow
    0x21b03cff, // kVDPColorDarkGreen
    0xc95bbaff, // kVDPColorMagenta
    0xccccccff, // kVDPColorGray
    0xffffffff  // kVDPColorWhite
};
const uint32_t * const VDPColorPalette = VDPColorPaletteArray;

typedef struct {
    void *observer;
    void (*handler)(void *observer);
} InterruptHandler;

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

    InterruptHandler interruptHandler;
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
    memset(&vdp->interruptHandler, 0, sizeof(vdp->interruptHandler));
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
    if (!vdp || !(vdp->registers[1] & kVDPDisplayEnMask)) {
        return;
    }

    switch (VDPGetGraphicsMode(vdp)) {
        case kVDPGraphicsMode1:
            GraphicsMode1GetScanline(vdp, rowIdx, pixelBuffer);
            break;

        case kVDPGraphicsMode2:
            break;

        case kVDPGraphicsModeMultiColor:
            break;

        case kVDPGraphicsModeText:
            break;

        default:
            // unsupported mode, just copy zeroes into the buffer
            memset(pixelBuffer, 0, kVDPSizeX);
    }

    if (rowIdx == kVDPSizeY - 1) {
        vdp->status |= kVDPStatusIRQMask;

        if (vdp->interruptHandler.handler && (vdp->registers[1] & kVDPIRQEnMask)) {
            (*vdp->interruptHandler.handler)(vdp->interruptHandler.observer);
        }
    }
}

void VDPSetInterruptHandler(VideoDisplayProcessorRef vdp, void *observer, void (*handler)(void *)) {
    vdp->interruptHandler.observer = observer;
    vdp->interruptHandler.handler = handler;
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

    return vdp->vram[addr & kVDPVramMask];
}

void VDPSetVram(VideoDisplayProcessorRef vdp, uint16_t addr, uint8_t value) {
    if (!vdp) {
        return;
    }

    vdp->vram[addr & kVDPVramMask] = value;
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

uint16_t VDPGetVramNameTableAddress(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return kVDPUnsetAddr;
    }

    return (vdp->registers[2] << 10) & kVDPVramMask;
}

uint16_t VDPGetVramColorTableAddress(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return kVDPUnsetAddr;
    }

    // TODO: does this differ in gfx mode II?
    return (vdp->registers[3] << 6) & kVDPVramMask;
}

uint16_t VDPGetVramPatternTableAddress(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return kVDPUnsetAddr;
    }

    return (vdp->registers[4] << 11) & kVDPVramMask;
}

uint16_t VDPGetVramSpriteAttributesAddress(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return kVDPUnsetAddr;
    }

    return (vdp->registers[5] << 7) & kVDPVramMask;
}

uint16_t VDPGetVramSpriteNamesAddress(VideoDisplayProcessorRef vdp) {
    if (!vdp) {
        return kVDPUnsetAddr;
    }

    return (vdp->registers[6] << 11) & kVDPVramMask;
}

VDPColor VDPGetBackgroundColor(VideoDisplayProcessorRef vdp) {
    return VDPColorBackground(vdp->registers[0x7]);
}

VDPColor VDPGetForegroundColor(VideoDisplayProcessorRef vdp) {
    return VDPColorForeground(vdp->registers[0x7]);
}
