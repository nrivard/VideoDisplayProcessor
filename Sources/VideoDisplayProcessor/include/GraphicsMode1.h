//
//  Header.h
//  
//
//  Created by Nate Rivard on 02/06/2023.
//

#ifndef GraphicsMode1_h
#define GraphicsMode1_h

#include "VideoDisplayProcessor.h"

void GraphicsMode1GetScanline(VideoDisplayProcessorRef ref, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX]);

#endif /* Header_h */
