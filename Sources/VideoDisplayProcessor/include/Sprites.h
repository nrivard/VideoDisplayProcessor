//
//  Sprites.h
//  
//
//  Created by Nate Rivard on 24/07/2023.
//

#ifndef Sprites_h
#define Sprites_h

#include "VideoDisplayProcessor.h"

extern void SpritesOverwriteScanline(VideoDisplayProcessorRef ref, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX]);

#endif /* Sprites_h */
