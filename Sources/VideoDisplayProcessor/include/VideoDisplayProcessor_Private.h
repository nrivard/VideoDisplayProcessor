//
//  Header.h
//  
//
//  Created by Nate Rivard on 05/06/2023.
//

#ifndef VideoDisplayProcessor_Private_h
#define VideoDisplayProcessor_Private_h

#include "VideoDisplayProcessor.h"

#define kVDPGraphicsTileX         32
#define kVDPGraphicsTileY         24
#define kVDPGraphicsPatternWidth   8
#define kVDPTextTileX             40
#define kVDPTextTileY             24
#define kVDPTextPatternWidth       6
#define kVDPPatternSize            8

#define VDPColorBackground(x)   (x & 0x0F)
#define VDPColorForeground(x)   ((x >> 4) & 0x0F)

#define kVDPSpriteSizeLargeMask 0x02
#define kVDPSpriteMagMask       0x01

#define kVDPSpriteMax           32
#define kVDPSpriteEarlyClkMask  0b10000000
// after encountering this as `yPos`, stop processing sprites
#define kVDPSpriteHideRemaining 0xD0

#pragma mark Utilities

/// returns the current status register without modifying it
extern uint8_t VDPGetStatus(VideoDisplayProcessorRef vdp);

/// sets the current status
extern void VDPSetStatus(VideoDisplayProcessorRef vdp, uint8_t status);

/// gets the address for the start of the nametable in VRAM
extern uint16_t VDPGetVramNameTableAddress(VideoDisplayProcessorRef vdp);

/// gets the address for the start of the color table in VRAM
extern uint16_t VDPGetVramColorTableAddress(VideoDisplayProcessorRef vdp);

/// gets the address for the start of the pattern table in VRAM
extern uint16_t VDPGetVramPatternTableAddress(VideoDisplayProcessorRef vdp);

/// gets the address for the start of the sprite attributes table in VRAM
extern uint16_t VDPGetVramSpriteAttributesAddress(VideoDisplayProcessorRef vdp);

/// gets the address for the start of the sprite names table in VRAM
extern uint16_t VDPGetVramSpriteNamesAddress(VideoDisplayProcessorRef vdp);

/// copies contents of VRAM as a block starting at given address up to given size
/// this does no safety checks!
extern void VDPGetVramBlock(VideoDisplayProcessorRef vdp, uint16_t address, uint8_t *const buffer, uint16_t size);

#endif /* Header_h */
