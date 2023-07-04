//
//  TextMode.c
//  
//
//  Created by Nate Rivard on 04/07/2023.
//

#include "TextMode.h"
#include "VideoDisplayProcessor_Private.h"

#include <memory.h>
#include <stdlib.h>

void TextModeGetScanline(VideoDisplayProcessorRef vdp, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX]) {
    if (!vdp) {
        return;
    }

    uint8_t *pixels = pixelBuffer;

    const uint8_t nameTableRow = rowIdx >> 3; // divide by size of each tile (8 bytes) to get first tile in that row
    const uint8_t innerPatternRow = rowIdx & 0x07; // this is the row within the pattern itself

    const uint16_t nameTable = VDPGetVramNameTableAddress(vdp) + (nameTableRow * kVDPTextTileX);
    const uint16_t patternTable = VDPGetVramPatternTableAddress(vdp);

    // there is no color table for this mode so just get the colors from registers
    const VDPColor background = VDPGetBackgroundColor(vdp);
    const VDPColor foreground = VDPGetForegroundColor(vdp);

    // text mode is 40 * 6 (240) so to center content, write BG color to first 8 pixels
    memset(pixels, background, 8);
    pixels += 8;

    for (uint8_t colIdx = 0; colIdx < kVDPTextTileX; ++colIdx) {
        const uint8_t patternIndex = VDPGetVram(vdp, nameTable + colIdx);
        const uint8_t pattern = VDPGetVram(vdp, patternTable + (patternIndex * kVDPPatternSize) + innerPatternRow);

        // iterate over each bit
        for (uint8_t bit = 0; bit < kVDPTextPatternWidth; ++bit) {
            *pixels = (pattern << bit) & 0x80 ? foreground : background; // test top bit
            ++pixels;
        }
    }

    // and write BG color to the last 8 pixels
    memset(pixels, background, 8);
}
