//
//  VideoDisplayProcessor.h
//  
//
//  Created by Nate Rivard on 30/05/2023.
//

#ifndef VideoDisplayProcessor_h
#define VideoDisplayProcessor_h

#include <stdio.h>

#define kVDPSizeX               256
#define kVDPSizeY               192
#define kVDPVramSize            0x4000

#define kVDPRegisterWriteMask   0x80
#define kVDPVramWriteMask       0x40

typedef struct __VideoDisplayProcessor *VideoDisplayProcessorRef;

/// Color palette indices
typedef enum {
    kVDPColorTransparent = 0,
    kVDPColorBlack,
    kVDPColorMediumGreen,
    kVDPColorLightGreen,
    kVDPColorDarkBlue,
    kVDPColorLightBlue,
    kVDPColorDarkRed,
    kVDPColorCyan,
    kVDPColorMediumRed,
    kVDPColorLightRed,
    kVDPColorDarkYellow,
    kVDPColorLightYellow,
    kVDPColorDarkGreen,
    kVDPColorMagenta,
    kVDPColorGray,
    kVDPColorWhite
} VDPColor;

/// Current graphics mode
typedef enum {
    kVDPGraphicsMode1 = 0,
    kVDPGraphicsMode2 = 1,
    kVDPGraphicsModeMultiColor = 2,
    kVDPGraphicsModeText = 4
} VDPGraphicsMode;

#pragma mark Lifecycle

/// creates a new VDP instance
extern VideoDisplayProcessorRef VDPCreate();

/// destroys an existing VDP instance
extern void VDPDestroy(VideoDisplayProcessorRef ref);

/// Describes hardware access to the VDP
#pragma mark Hardware API

/// resets internal state (does not affect VRAM!)
extern void VDPReset(VideoDisplayProcessorRef ref);

/// writes a value to register port
/// this is equivalent of `mode = 1` when writing
extern void VDPWriteToRegisterPort(VideoDisplayProcessorRef ref, uint8_t value);

/// reads the status register
/// this is equivalent to `mode = 1` when reading
extern uint8_t VDPReadFromRegisterPort(VideoDisplayProcessorRef ref);

/// writes a value to the data port
/// this is equivalent to `mode = 0` when writing
extern void VDPWriteToDataPort(VideoDisplayProcessorRef ref, uint8_t value);

/// reads from data port
/// this is equivalent to `mode = 0` when reading
extern uint8_t VDPReadFromDataPort(VideoDisplayProcessorRef ref);

/// Describes video hardware access
#pragma mark Video Display

/// calculates pixel values for an entire scanline and copies to the provided buffer
/// `pixels` is filled out with `VDPColor` values
extern void VDPGetScanline(VideoDisplayProcessorRef ref, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX]);

/// Allows debuggers to access internal state
#pragma mark Debugger Access

/// returns the contents of the internal register at `registerIdx`
extern uint8_t VDPGetRegister(VideoDisplayProcessorRef ref, uint8_t registerIdx);

/// sets the contents of the internal register at `registerIdx` to `value`
extern void VDPSetRegister(VideoDisplayProcessorRef ref, uint8_t registerIdx, uint8_t value);

/// copies raw vram content into the provided buffer
extern void VDPGetVramContents(VideoDisplayProcessorRef ref, uint8_t vramBuffer[kVDPVramSize]);

/// gets vram contents at `addr`
extern uint8_t VDPGetVram(VideoDisplayProcessorRef ref, uint16_t addr);

/// sets the contents of vram at `addr` to `value`
extern void VDPSetVram(VideoDisplayProcessorRef ref, uint16_t addr, uint8_t value);

/// gets the current vram address
extern uint16_t VDPGetVramAddress(VideoDisplayProcessorRef ref);

/// sets the current vram address outside of the normal write process
extern void VDPSetVramAddress(VideoDisplayProcessorRef ref, uint16_t addr);

/// gets the current graphics mode
extern VDPGraphicsMode VDPGetGraphicsMode(VideoDisplayProcessorRef ref);

#endif /* VideoDisplayProcessor_h */
