//
//  TextMode.h
//  
//
//  Created by Nate Rivard on 04/07/2023.
//

#ifndef TextMode_h
#define TextMode_h

#include "VideoDisplayProcessor.h"

extern void TextModeGetScanline(VideoDisplayProcessorRef ref, uint8_t rowIdx, uint8_t pixelBuffer[kVDPSizeX]);

#endif /* TextMode_h */
