//
//  GraphicsMode1.c
//  
//
//  Created by Nate Rivard on 02/06/2023.
//

#include "GraphicsMode1.h"
#include "VideoDisplayProcessor_Private.h"

#include <memory.h>
#include <stdlib.h>

void GraphicsMode1GetScanline(VideoDisplayProcessorRef vdp, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX])  {
    if (!vdp) {
        return;
    }

    uint8_t *pixels = pixelBuffer;

    const uint8_t nameTableRow = rowIdx >> 3; // divide by size of each tile (8 bytes) to get first tile in that row
    const uint8_t innerPatternRow = rowIdx & 0x07; // this is the row within the pattern itself

    const uint16_t nameTable = VDPGetVramNameTableAddress(vdp) + (nameTableRow * kVDPGraphicsTileX);
    const uint16_t patternTable = VDPGetVramPatternTableAddress(vdp);
    const uint16_t colorTable = VDPGetVramColorTableAddress(vdp);

    for (uint8_t colIdx = 0; colIdx < kVDPGraphicsTileX; ++colIdx) {
        const uint8_t patternIndex = VDPGetVram(vdp, nameTable + colIdx);
        const uint8_t pattern = VDPGetVram(vdp, patternTable + (patternIndex * kVDPPatternSize) + innerPatternRow);

        const uint8_t color = VDPGetVram(vdp, colorTable + (patternIndex >> 3)); // colors apply to groups of 8 tiles
        const uint8_t foreground = VDPColorForeground(color);
        const uint8_t background = VDPColorBackground(color);

        // iterate over each bit
        for (uint8_t bit = 0; bit < kVDPGraphicsPatternWidth; ++bit) {
            *pixels = (pattern << bit) & 0x80 ? foreground : background; // test top bit
            ++pixels;
        }
    }
}
