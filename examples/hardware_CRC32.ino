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

// Filler data to test the CRC32 on. Data must be 'word aligned'; that is, divisible by 32 bits.
// Note that SAMD devices are little endian; raw data such as below will not 'paste' to other
// CRC32 calculators and produce the same CRC unless care is taken to ensure proper endian.
const uint32_t data[96] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                           0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
// CRC32 of this data evaluates to 0xD70A721F

void setup()
{

    // Init serial communication
    Serial.begin(9600);
    while (!Serial)

        // Check whether or not the current device supports hardware CRC32
        Serial.println(crc.can_use_hardware_crc32() ? "Hardware CRC32 Supported" : "Hardware CRC32 Not Supported");

    // Create a variable for the CRC32 computation to be stored to
    uint32_t crc_result = 0;

    // Run a CRC32 with the filler data
    crc.crc32(&data, sizeof(data), &crc_result);

    // Print the resulting CRC32 value
    Serial.print(crc.check_using_hardware_crc32() ? "Hardware: 0x" : "Software: 0x");
    Serial.println(crc_result, HEX);

    // Force the use of software CRC32
    crc.force_use_software_crc32(true);

    // Reset the result variable to 0
    crc_result = 0;

    // Run a CRC32 with the filler data
    crc.crc32(&data, sizeof(data), &crc_result);

    // Print the resulting CRC32 value
    Serial.print(crc.check_using_hardware_crc32() ? "Hardware: 0x" : "Software: 0x");
    Serial.println(crc_result, HEX);
}

void loop()
{

}
