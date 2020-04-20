/***************************************************
  This is a library for SAMD hardware CRC32 support

  https://github.com/knicholson32/SAMD-CRC32

  This library uses the DSU CRC32 hardware contained in the
  ATSAMD series microcontrollers. If used on a device
  that is not SAMD (such as AVR), a software CRC32 algorithm
  will be used.

  The software CRC32 algorithm was adapted from this source:
  http://home.thep.lu.se/~bjorn/crc/
  Björn Samuelsson

  CRC32 uses the standard 32-bit CRC parameters:
    Poly:    0x04C11DB7
    Init:    0xFFFFFFFF
    XOR:     0xFFFFFFFF
    Reflect: Yes

  Written by Keenan Nicholson
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <SAMD_CRC32.h>

// Create the CRC object
SAMD_CRC32 crc = SAMD_CRC32();

// Filler data to test the CRC32 on. Data must be 'word aligned'; that is, divisible by 32 bits. If
// the data is not 32-bit word aligned, the software CRC32 algorithm will be used.
// Note that SAMD devices are little endian; raw data such as below will not 'paste' to other
// CRC32 calculators and produce the same CRC unless care is taken to ensure proper endian.
const uint32_t data[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

void setup()
{

    // Init serial communication
    Serial.begin(9600);
    while (!Serial) {}

    // Create a variable for the CRC32 computation to be stored to
    uint32_t crc_result = 0;

    /* Four ways to get hardware status information from the CRC object.
     * Hardware status infomation is generated after each CRC is performed. If the
     * status is not 'OK' (0x00), software CRC was used. */

    // -------- Style 1:
    uint8_t status_code = crc.crc32(&data, sizeof(data), &crc_result);
    Serial.print("Style 1: Status Code: ");
    Serial.print(status_code);
    Serial.print(":");
    Serial.println(crc.decode_hardware_status_code(status_code));

    // -------- Style 2:
    Serial.print("Style 2: Status Code: ");
    Serial.print(crc.crc32(&data, sizeof(data), &crc_result));
    Serial.print(":");
    Serial.println(crc.get_hardware_status_msg());

    // -------- Style 3:
    crc.crc32(&data, sizeof(data), &crc_result);
    Serial.print("Style 3: Status Code: ");
    Serial.print(crc.get_hardware_status_code());
    Serial.print(":");
    Serial.println(crc.get_hardware_status_msg());

    /* Example of some of the possible hardware status codes and how
     * they can be triggered (assuming the hardware supports hardware
     * CRC in general): */

    // -------- NOT_WORD_ALIGNED:
    crc.crc32(&data, 7, &crc_result); // 7 bytes is not word aligned (multiple of 4 bytes)
    Serial.print("NOT_WORD_ALIGNED:     Status Code: ");
    Serial.print(crc.get_hardware_status_code());
    Serial.print(":");
    Serial.println(crc.get_hardware_status_msg());

    // -------- USER_FORCED_SOFTWARE:
    crc.force_use_software_crc32(true); // Forcing software CRC causes the hardware CRC to be bypassed
    crc.crc32(&data, 7, &crc_result);
    Serial.print("USER_FORCED_SOFTWARE: Status Code: ");
    Serial.print(crc.get_hardware_status_code());
    Serial.print(":");
    Serial.println(crc.get_hardware_status_msg());

    // -------- BUS_ERROR:                Internal SAMD bus error (unknown error)
    // -------- HARDWARE_NOT_SUPPORTED:   Hardware does not support this hardware CRC implementation (not SAMD)
}

void loop()
{

}
