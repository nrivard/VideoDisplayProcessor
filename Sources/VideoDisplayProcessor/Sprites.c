//
//  Sprites.c
//  
//
//  Created by Nate Rivard on 24/07/2023.
//

#include "Sprites.h"
#include "VideoDisplayProcessor_Private.h"

typedef struct {
    uint8_t yPos;
    uint8_t xPos;
    uint8_t patternIdx;
    uint8_t color;      // low nibble contains color, high bit is `kVDPSpriteEarlyClkMask`
} SpriteAttrs;

void SpritesOverwriteScanline(VideoDisplayProcessorRef vdp, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX]) {
    if (!vdp) {
        return;
    }

    uint8_t *pixels = pixelBuffer;

    const uint8_t status = VDPGetStatus(vdp);
    const uint8_t control1 = VDPGetRegister(vdp, 1);
    const uint8_t spriteSize = (control1 & kVDPSpriteSizeLargeMask ? 16 : 8) * (control1 & kVDPSpriteMagMask ? 2 : 1);

    const uint16_t spriteAttrTable = VDPGetVramSpriteAttributesAddress(vdp);
    const uint16_t spritePatternTable = VDPGetVramSpriteNamesAddress(vdp);

    // keeps track of 5th sprite where we need to stop processing sprites and set status register fields
    uint8_t spritesOnLine = 0;

    // let's copy over sprite attrs
    SpriteAttrs spriteAttrs[kVDPSpriteMax];
    VDPGetVramBlock(vdp, spriteAttrTable, (uint8_t *)spriteAttrs, sizeof(SpriteAttrs) * kVDPSpriteMax);

    for (uint8_t spriteIdx = 0; spriteIdx < kVDPSpriteMax; ++spriteIdx) {
        SpriteAttrs current = spriteAttrs[spriteIdx];

        if (current.yPos == kVDPSpriteHideRemaining) {
            if (status == 0) {
                VDPSetStatus(vdp, status | spriteIdx);
            }
            return;
        }

        // use 16 bit signed value. have to convert to signed 8 bit first otherwise sign won't be extended
        int16_t signedYPos = (int16_t)(int8_t)current.yPos;
    }
}
