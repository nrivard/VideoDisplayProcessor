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
#define kVDPPatternSize            8

#define VDPColorBackground(x)   (x & 0x0F)
#define VDPColorForeground(x)   ((x >> 4) & 0x0F)

#pragma mark Utilities

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

#endif /* Header_h */
