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
// CRC32 of this data evaluates to 0x88BAD147

#define TEST_TRIALS 10000

void setup()
{

    // Init serial communication
    Serial.begin(9600);
    while (!Serial) {}

    // Create some variables for timing
    unsigned long t1;
    unsigned long t2;
    
    // Create a variable for the CRC32 computation to be stored to
    uint32_t crc_result = 0;

    // ----- Run the Hardware CRC test
    t1 = micros();
    for(int i = 0; i < TEST_TRIALS; i++){
      crc_result = 0;
      crc.crc32(&data, sizeof(data), &crc_result);
    }
    t2 = micros();

    // Print results
    Serial.print(crc.check_used_hardware_crc32()?"Hardware: 0x":"Software: 0x");
    Serial.print(crc_result, HEX);
    Serial.print(" ");
    Serial.print((t2-t1)/((float)TEST_TRIALS));
    Serial.println("us/crc");


    // Force the use of software CRC for the next test
    crc.force_use_software_crc32(true);


    // ----- Run the Software CRC test
    t1 = micros();
    for(int i = 0; i < TEST_TRIALS; i++){
      crc_result = 0;
      crc.crc32(&data, sizeof(data), &crc_result);
    }
    t2 = micros();

    // Print results
    Serial.print(crc.check_used_hardware_crc32()?"Hardware: 0x":"Software: 0x");
    Serial.print(crc_result, HEX);
    Serial.print(" ");
    Serial.print((t2-t1)/((float)TEST_TRIALS));
    Serial.println("us/crc");
}

void loop()
{

}