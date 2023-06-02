//
//  VDPTests.swift
//  
//
//  Created by Nate Rivard on 30/05/2023.
//

import XCTest
@testable import VideoDisplayProcessor

final class VDPTests: XCTestCase {
    var vdp: VideoDisplayProcessorRef!

    override func setUp() {
        vdp = VDPCreate()
    }

    override func tearDown() {
        VDPDestroy(vdp)
    }

    func testGetSetRegister() {
        VDPSetRegister(vdp, 2, 0xAA)
        XCTAssert(VDPGetRegister(vdp, 2) == 0xAA)
    }

    func testGetSetVram() {
        // given
        VDPSetVram(vdp, 0x1001, 0x55)
        VDPSetVram(vdp, 0x0000, 0xBE)
        VDPSetVram(vdp, 0x3FFF, 0xEF)

        // when
        let vram = UnsafeMutableBufferPointer<UInt8>.allocate(capacity: 0x4000)
        VDPGetVramContents(vdp, vram.baseAddress);

        // then
        XCTAssert(vram[0x1001] == 0x55)
        XCTAssert(vram[0x0000] == 0xBE)
        XCTAssert(vram[0x3FFF] == 0xEF)
    }

    func testReset() {
        // given
        VDPSetRegister(vdp, 3, 0xA0)
        VDPSetVram(vdp, 0x1001, 0x1F)

        XCTAssert(VDPGetRegister(vdp, 3) == 0xA0)
        XCTAssert(VDPGetVram(vdp, 0x1001) == 0x1F)

        // when
        VDPReset(vdp)

        // then
        XCTAssert(VDPGetRegister(vdp, 3) == 0x00)
        // vram is untouched
        XCTAssert(VDPGetVram(vdp, 0x1001) == 0x1F)
    }

    func testWriteToRegister() {
        // register write
        VDPWriteToRegisterPort(vdp, 0x12)
        VDPWriteToRegisterPort(vdp, UInt8(kVDPRegisterWriteMask) | 0x07)

        XCTAssert(VDPGetRegister(vdp, 0x07) == 0x12)

        // set vram addr (read)
        VDPWriteToRegisterPort(vdp, 0x01)
        VDPWriteToRegisterPort(vdp, 0x10)

        XCTAssert(VDPGetVramAddress(vdp) == 0x1002)

        // set vram addr (write)
        VDPWriteToRegisterPort(vdp, 0x22)
        VDPWriteToRegisterPort(vdp, UInt8(kVDPVramWriteMask) | 0x22)

        XCTAssert(VDPGetVramAddress(vdp) == 0x2222)
    }

    func testAutoincrementingRead() {
        // given
        VDPSetVram(vdp, 0x3FFE, 0x02)
        VDPSetVram(vdp, 0x3FFF, 0x06)

        // when
        VDPWriteToRegisterPort(vdp, 0xFE)
        VDPWriteToRegisterPort(vdp, 0x3F)

        // then
        XCTAssert(VDPGetVramAddress(vdp) == 0x3FFF)
        XCTAssert(VDPReadFromDataPort(vdp) == 0x02)

        // then again
        XCTAssert(VDPGetVramAddress(vdp) == 0x0000)
        XCTAssert(VDPReadFromDataPort(vdp) == 0x06)
        XCTAssert(VDPGetVramAddress(vdp) == 0x0001)
    }

    func testAutoincrementingWrite() {
        // when (no given!)
        VDPWriteToRegisterPort(vdp, 0xFF)
        VDPWriteToRegisterPort(vdp, UInt8(kVDPVramWriteMask) | 0x3F)
        XCTAssert(VDPGetVramAddress(vdp) == 0x3FFF)

        // then
        VDPWriteToDataPort(vdp, 0xAA)
        XCTAssert(VDPGetVram(vdp, 0x3FFF) == 0xAA)
        XCTAssert(VDPGetVramAddress(vdp) == 0x0000)

        // then again
        VDPWriteToDataPort(vdp, 0x44)
        XCTAssert(VDPGetVram(vdp, 0x0000) == 0x44)
        XCTAssert(VDPGetVramAddress(vdp) == 0x0001)
    }

    func testWriteThenRead() {
        // given
        VDPSetVram(vdp, 0x0401, 0xF2)

        // when
        VDPWriteToRegisterPort(vdp, 0x00)
        VDPWriteToRegisterPort(vdp, UInt8(kVDPVramWriteMask) | 0x04)
        XCTAssert(VDPGetVramAddress(vdp) == 0x0400)

        // then
        VDPWriteToDataPort(vdp, 0xF1)
        XCTAssert(VDPGetVramAddress(vdp) == 0x0401)
        XCTAssert(VDPGetVram(vdp, 0x0400) == 0xF1)

        // verify contents are from read-ahead buffer and NOT from the actual vram addr!
        XCTAssert(VDPReadFromDataPort(vdp) == 0xF1)
        XCTAssert(VDPGetVramAddress(vdp) == 0x0402)

        // verify read-ahead has been filled by 0x401
        XCTAssert(VDPReadFromDataPort(vdp) == 0xF2)
    }

    func testReadThenWrite() {
        // given
        VDPSetVram(vdp, 0x1000, 0xBB)

        // when
        VDPWriteToRegisterPort(vdp, 0x00)
        VDPWriteToRegisterPort(vdp, 0x10)
        XCTAssert(VDPGetVramAddress(vdp) == 0x1001)

        VDPWriteToDataPort(vdp, 0xAA)

        // then
        XCTAssert(VDPGetVram(vdp, 0x1000) == 0xBB)
        XCTAssert(VDPGetVram(vdp, 0x1001) == 0xAA)
    }
}
