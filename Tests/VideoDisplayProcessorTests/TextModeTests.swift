//
//  TextModeTests.swift
//  
//
//  Created by Nate Rivard on 04/07/2023.
//

import XCTest
@testable import VideoDisplayProcessor

final class TextModeTests: XCTestCase {
    var vdp: VideoDisplayProcessorRef!

    static let patternData: [UInt8] = [
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // all background
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // all foreground
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // all background except final 2 pixels
        0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, // all foreground except final 2 pixels
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // alternating vertical
        0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, // alternating horizontal v1
        0x38, 0x44, 0x5C, 0x54, 0x5C, 0x40, 0x38, 0x00, // `@`
        0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00  // `T`
    ]

    // 40 tiles per row
    static let nameData: [UInt8] = Array(0..<40)

    static let lineData: [UInt8] = [
        .bg, .bg, .bg, .bg, .bg, .bg, // all black
        .fg, .fg, .fg, .fg, .fg, .fg, // all white
        .bg, .bg, .bg, .bg, .bg, .bg, // all black (again)
        .fg, .fg, .fg, .fg, .fg, .fg, // all white (again)
        .fg, .bg, .fg, .bg, .fg, .bg, // alternating white then black
        .bg, .bg, .bg, .bg, .bg, .bg, // all black (top row of vertically alternating pattern)
        .bg, .bg, .fg, .fg, .fg, .bg, // top line of '0'
        .bg, .fg, .fg, .fg, .fg, .fg, // top line of 'T'
    ]

    override func setUp() {
        vdp = VDPCreate()

        // put VDP in text mode
        VDPSetRegister(vdp, 0, 0x00)
        VDPSetRegister(vdp, 1, 0xD0)

        // setup table addresses
        VDPSetRegister(vdp, 2, 0x05); // 0x1400
        VDPSetRegister(vdp, 3, 0x80); // 0x2000
        VDPSetRegister(vdp, 4, 0x01); // 0x0800
        VDPSetRegister(vdp, 5, 0x20); // 0x1000
        VDPSetRegister(vdp, 6, 0x00); // 0x0000 :)
        VDPSetRegister(vdp, 7, 0xF1); // white on black

        // setup vram: patterns, colors, nametable
        VDPSetVramAddress(vdp, 0x1400)  // already primed for writing
        for name in TextModeTests.nameData {
            VDPWriteToDataPort(vdp, name)
        }

        // repeat the patterns 5 times to fill the row (5 * 8 patterns)
        VDPSetVramAddress(vdp, 0x0800)
        for _ in 0..<5 {
            for pattern in TextModeTests.patternData {
                VDPWriteToDataPort(vdp, pattern)
            }
        }
    }

    override func tearDown() {
        VDPDestroy(vdp)
    }

    func testTiles() {
        // given
        let testLine = Array(repeating: UInt8.bg, count: 8)
            + Array(repeating: TextModeTests.lineData, count: 5).flatMap { return $0 }
            + Array(repeating: UInt8.bg, count: 8)

        // when
        let scanline = UnsafeMutableBufferPointer<UInt8>.allocate(capacity: Int(kVDPSizeX))

        // then: line 0
        VDPGetScanline(vdp, 0, scanline.baseAddress)
        XCTAssert(Array(scanline) == testLine, "\(Array(scanline)) did not match test data: \(testLine)")
    }
}

fileprivate extension UInt8 {
    static var bg = UInt8(kVDPColorBlack.rawValue)
    static var fg = UInt8(kVDPColorWhite.rawValue)
}
